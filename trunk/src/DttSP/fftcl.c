#define _CRT_SECURE_NO_WARNINGS
#define PROGRAM_FILE "fft.cl"
#define INIT_FUNC "fft_init"
#define STAGE_FUNC "fft_stage"
#define SCALE_FUNC "fft_scale"

/* Each point contains 2 floats - 1 real, 1 imaginary */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>

#ifdef MAC
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "common.h"

/* Host/device data structures */
cl_device_id device;
cl_context context;
cl_command_queue queue;
cl_program program;
cl_kernel init_kernel, stage_kernel, scale_kernel;
cl_int err, i;
size_t global_size, local_size;
cl_ulong local_mem_size;

/* Find a GPU or CPU associated with the first available platform */
cl_device_id create_device() {

   cl_platform_id platform[2];
   cl_device_id dev;
   int err;

   /* Identify a platform */
   err = clGetPlatformIDs(2, platform, NULL);
   if(err < 0) {
      perror("Couldn't identify a platform");
      exit(1);
   } 

   /* Access a device */
   err = clGetDeviceIDs(platform[0], CL_DEVICE_TYPE_ALL, 1, &dev, NULL);
   if(err == CL_DEVICE_NOT_FOUND) {
      err = clGetDeviceIDs(platform[1], CL_DEVICE_TYPE_ALL, 1, &dev, NULL);
   }
   if(err < 0) {
      perror("Couldn't access any devices");
      exit(1);   
   }

   return dev;
}

/* Create program from a file and compile it */
cl_program build_program(cl_context ctx, cl_device_id dev, const char* filename) {

   cl_program program;
   FILE *program_handle;
   char *program_buffer, *program_log;
   size_t program_size, log_size;
   int err;

   /* Read program file and place content into buffer */
   program_handle = fopen(filename, "r");
   if(program_handle == NULL) {
      perror("Couldn't find the program file");
      exit(1);
   }
   fseek(program_handle, 0, SEEK_END);
   program_size = ftell(program_handle);
   rewind(program_handle);
   program_buffer = (char*)malloc(program_size + 1);
   program_buffer[program_size] = '\0';
   err = fread(program_buffer, sizeof(char), program_size, program_handle);
   if (err < 0) {
	perror("Error in reading OpenCL program");
	exit(1);
   }
   fclose(program_handle);

   /* Create program from file */
   program = clCreateProgramWithSource(ctx, 1, 
      (const char**)&program_buffer, &program_size, &err);
   if(err < 0) {
      perror("Couldn't create the program");
      exit(1);
   }
   free(program_buffer);

   /* Build program */
   err = clBuildProgram(program, 0, NULL, NULL, NULL, NULL);
   if(err < 0) {

      /* Find size of log and print to std output */
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
            0, NULL, &log_size);
      program_log = (char*) malloc(log_size + 1);
      program_log[log_size] = '\0';
      clGetProgramBuildInfo(program, dev, CL_PROGRAM_BUILD_LOG, 
            log_size + 1, program_log, NULL);
      printf("%s\n", program_log);
      free(program_log);
      exit(1);
   }

   return program;
}

fftcl_plan *fftcl_plan_create(int N, COMPLEX *in, COMPLEX *out, int direction){
	fftcl_plan *plan;
	plan = (fftcl_plan*) malloc(sizeof(fftcl_plan));
	plan->N = N;
	plan->in = in;
	plan->out = out;
	plan->direction = direction;
	return plan;
}

void fftcl_plan_destroy(fftcl_plan* plan){
	if (plan != NULL){
		free(plan);
	}
}

void fftcl_plan_execute(fftcl_plan* plan){
   cl_mem data_buffer;
   int direction;
   unsigned int num_points, points_per_group, stage;
   float *data = malloc(plan->N * 2 * sizeof(float));

   for (int i=0; i<plan->N; i++){
   	data[i*2] = c_re(plan->in[i]);
	data[i*2+1] = c_im(plan->in[i]);
   }
   /* Create buffer */
   data_buffer = clCreateBuffer(context, 
         CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, 
         2*plan->N*sizeof(float), data, &err);
   if(err < 0) {
      perror("Couldn't create a buffer");
      exit(1);
   };

   /* Initialize kernel arguments */
   if (plan->direction == FFTW_FORWARD) direction = 1;
   else direction = -1;
   // kernel direction 1 is forward FFT, so it the reverse of FFTW_FORWARD, which is -1
   num_points = plan->N;
   points_per_group = local_mem_size/(2*sizeof(float));
   if(points_per_group > num_points)
      points_per_group = num_points;

   /* Set kernel arguments */
   err = clSetKernelArg(init_kernel, 0, sizeof(cl_mem), &data_buffer);
   err |= clSetKernelArg(init_kernel, 1, local_mem_size, NULL);
   err |= clSetKernelArg(init_kernel, 2, sizeof(points_per_group), &points_per_group);
   err |= clSetKernelArg(init_kernel, 3, sizeof(num_points), &num_points);
   err |= clSetKernelArg(init_kernel, 4, sizeof(direction), &direction);
   if(err < 0) {
      printf("Couldn't set a kernel argument");
      exit(1);   
   };

   /* Enqueue initial kernel */
   global_size = (num_points/points_per_group)*local_size;
   err = clEnqueueNDRangeKernel(queue, init_kernel, 1, NULL, &global_size, 
                                &local_size, 0, NULL, NULL); 
   if(err < 0) {
      perror("Couldn't enqueue the initial kernel");
      exit(1);
   }

   /* Enqueue further stages of the FFT */
   if(num_points > points_per_group) {

      err = clSetKernelArg(stage_kernel, 0, sizeof(cl_mem), &data_buffer);
      err |= clSetKernelArg(stage_kernel, 2, sizeof(points_per_group), &points_per_group);
      err |= clSetKernelArg(stage_kernel, 3, sizeof(direction), &direction);
      if(err < 0) {
         printf("Couldn't set a kernel argument");
         exit(1);   
      };
      for(stage = 2; stage <= num_points/points_per_group; stage <<= 1) {

         clSetKernelArg(stage_kernel, 1, sizeof(stage), &stage);
         err = clEnqueueNDRangeKernel(queue, stage_kernel, 1, NULL, &global_size, 
                                      &local_size, 0, NULL, NULL); 
         if(err < 0) {
            perror("Couldn't enqueue the stage kernel");
            exit(1);
         }
      }
   }

   /* Scale values if performing the inverse FFT */
   if(direction < 0) {
      err = clSetKernelArg(scale_kernel, 0, sizeof(cl_mem), &data_buffer);
      err |= clSetKernelArg(scale_kernel, 1, sizeof(points_per_group), &points_per_group);
      err |= clSetKernelArg(scale_kernel, 2, sizeof(num_points), &num_points);
      if(err < 0) {
         printf("Couldn't set a kernel argument");
         exit(1);   
      };
      err = clEnqueueNDRangeKernel(queue, scale_kernel, 1, NULL, &global_size, 
                                   &local_size, 0, NULL, NULL); 
      if(err < 0) {
         perror("Couldn't enqueue the scale kernel");
         exit(1);
      }
   }

   /* Read the results */
   err = clEnqueueReadBuffer(queue, data_buffer, CL_TRUE, 0, 
         2*plan->N*sizeof(float), data, 0, NULL, NULL);
   if(err < 0) {
      perror("Couldn't read the buffer");
      exit(1);
   }

   for (int i=0; i < plan->N; i++){
	plan->out[i] = Cmplx(data[i*2], data[i*2+1]);
   }
   free(data);
   clReleaseMemObject(data_buffer);
}

void fftcl_initialize(void){
   /* Create a device and context */
   device = create_device();
   context = clCreateContext(NULL, 1, &device, NULL, NULL, &err);
   if(err < 0) {
      perror("Couldn't create a context");
      exit(1);   
   }

   /* Build the program */
   program = build_program(context, device, PROGRAM_FILE);

   /* Create kernels for the FFT */
   init_kernel = clCreateKernel(program, INIT_FUNC, &err);
   if(err < 0) {
      printf("Couldn't create the initial kernel: %d", err);
      exit(1);
   };
   stage_kernel = clCreateKernel(program, STAGE_FUNC, &err);
   if(err < 0) {
      printf("Couldn't create the stage kernel: %d", err);
      exit(1);
   };
   scale_kernel = clCreateKernel(program, SCALE_FUNC, &err);
   if(err < 0) {
      printf("Couldn't create the scale kernel: %d", err);
      exit(1);
   };

   /* Determine maximum work-group size */
   err = clGetKernelWorkGroupInfo(init_kernel, device, 
      CL_KERNEL_WORK_GROUP_SIZE, sizeof(local_size), &local_size, NULL);
   if(err < 0) {
      perror("Couldn't find the maximum work-group size");

      exit(1);   
   };
   //local_size = (int)pow(2, trunc(log2(local_size)));
   fprintf(stderr,"GPU: max workgroup size = %d\n", (int)local_size);

   local_size = 16;

   /* Determine local memory size */
   err = clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, 
      sizeof(local_mem_size), &local_mem_size, NULL);
   if(err < 0) {
      perror("Couldn't determine the local memory size");
      exit(1);   
   };
   fprintf(stderr,"GPU: local memory size = %d\n", (int)local_mem_size);

   /* Create a command queue */
   queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
   if(err < 0) {
      perror("Couldn't create a command queue");
      exit(1);   
   };
}

void fftcl_destroy(void){

   /* Deallocate resources */
   clReleaseKernel(init_kernel);
   clReleaseKernel(stage_kernel);
   clReleaseKernel(scale_kernel);
   clReleaseCommandQueue(queue);
   clReleaseProgram(program);
   clReleaseContext(context);
}


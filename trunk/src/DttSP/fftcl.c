#define _CRT_SECURE_NO_WARNINGS
#define PROGRAM_FILE "FFT_Kernels.cl"


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
cl_kernel kernel;
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
   cl_mem buffer_r, buffer_i;
   int direction;
   float *data_r = malloc(plan->N * sizeof(float));
   float *data_i = malloc(plan->N * sizeof(float));

   for (int i=0; i<plan->N; i++){
   	data_r[i] = c_re(plan->in[i]);
	data_i[i] = c_im(plan->in[i]);
   }
   /* Create buffers */
   buffer_r = clCreateBuffer(context, 
         CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, 
         plan->N*sizeof(float), data_r, &err);
   if(err < 0) {
      perror("Couldn't create buffer_r");
      exit(1);
   };
   buffer_i = clCreateBuffer(context, 
         CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, 
         plan->N*sizeof(float), data_i, &err);
   if(err < 0) {
      perror("Couldn't create buffer_r");
      exit(1);
   };

   /* Initialize kernel arguments */
   if (plan->direction == FFTW_FORWARD) direction = 1;
   else direction = -1;

   /* Set kernel arguments */
   err = clSetKernelArg(kernel, 0, sizeof(cl_mem), &buffer_r);
   err |= clSetKernelArg(kernel, 1, sizeof(cl_mem), &buffer_i);

   if(err < 0) {
      printf("Couldn't set a kernel argument");
      exit(1);   
   };

   /* Enqueue initial kernel */
   global_size = 1024;
   err = clEnqueueNDRangeKernel(queue, kernel, 1, NULL, &global_size, 
                                &local_size, 0, NULL, NULL); 
   if(err < 0) {
      perror("Couldn't enqueue the initial kernel");
      exit(1);
   }

   /* Read the results */
   err = clEnqueueReadBuffer(queue, buffer_r, CL_TRUE, 0, 
         2*plan->N*sizeof(float), data_r, 0, NULL, NULL);
   if(err < 0) {
      perror("Couldn't read the buffer");
      exit(1);
   }

   for (int i=0; i < plan->N; i++){
	plan->out[i] = Cmplx(data_r[i], data_i[i]);
   }
   free(data_r);
   free(data_i);
   clReleaseMemObject(buffer_r);
   clReleaseMemObject(buffer_i);
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
   kernel = clCreateKernel(program, "kfft", &err);
   if(err < 0) {
      printf("Couldn't create the kernel: %d", err);
      exit(1);
   };

   /* Determine maximum work-group size */
   err = clGetKernelWorkGroupInfo(kernel, device, 
      CL_KERNEL_WORK_GROUP_SIZE, sizeof(local_size), &local_size, NULL);
   if(err < 0) {
      perror("Couldn't find the maximum work-group size");

      exit(1);   
   };
   local_size = (int)pow(2, trunc(log2(local_size)));
   fprintf(stderr,"MAX WORK_GROUP_SIZE = %d\n", (int)local_size);

   local_size = 64;

   /* Determine local memory size */
   err = clGetDeviceInfo(device, CL_DEVICE_LOCAL_MEM_SIZE, 
      sizeof(local_mem_size), &local_mem_size, NULL);
   if(err < 0) {
      perror("Couldn't determine the local memory size");
      exit(1);   
   };

   /* Create a command queue */
   queue = clCreateCommandQueue(context, device, CL_QUEUE_PROFILING_ENABLE, &err);
   if(err < 0) {
      perror("Couldn't create a command queue");
      exit(1);   
   };
}

void fftcl_destroy(void){

   /* Deallocate resources */
   clReleaseKernel(kernel);
   clReleaseCommandQueue(queue);
   clReleaseProgram(program);
   clReleaseContext(context);
}


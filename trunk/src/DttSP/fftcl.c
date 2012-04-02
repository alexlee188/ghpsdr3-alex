#define _CRT_SECURE_NO_WARNINGS
#define PROGRAM_FILE "FFT_Kernels.cl"


/* Each point contains 2 floats - 1 real, 1 imaginary */

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <semaphore.h>

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
cl_kernel kernel_f512, kernel_b512, kernel_f1024, kernel_b1024, kernel_f2048, kernel_b2048;
cl_kernel kernel_f4096, kernel_b4096;
cl_int err, i;
cl_ulong local_mem_size;

static sem_t fft_semaphore;

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
   cl_mem buffer_c;		// complex buffer_c

   sem_wait(&fft_semaphore);

   // copy input to output, for doing in place fft at output buffer
   memcpy(plan->out, plan->in, plan->N*2*sizeof(float));

   buffer_c = clCreateBuffer(context, 
	 CL_MEM_READ_WRITE | CL_MEM_USE_HOST_PTR, 
	 2*plan->N*sizeof(float), plan->out, &err);
   if(err < 0) {
      perror("Couldn't create buffer_c");
      exit(1);
   };

   if (plan->N == 512){
	size_t global_size = 64;
	size_t local_size = 64;
	if (plan->direction == FFTW_FORWARD){
	   /* Set kernel arguments */
	   err = clSetKernelArg(kernel_f512, 0, sizeof(cl_mem), &buffer_c);
	   if(err < 0) {
	      printf("Couldn't set a kernel argument");
	      exit(1);   
	   };
	   /* Enqueue kernel */
	   err = clEnqueueNDRangeKernel(queue, kernel_f512, 1, NULL, &global_size, 
		                        &local_size, 0, NULL, NULL); 
	   if(err < 0) {
	      perror("Couldn't enqueue kernel_f512");
	      exit(1);
	   }
        } else	{// inverse
	   /* Set kernel arguments */
	   err = clSetKernelArg(kernel_b512, 0, sizeof(cl_mem), &buffer_c);
	   if(err < 0) {
	      printf("Couldn't set a kernel argument");
	      exit(1);   
	   };
	   /* Enqueue kernel */
	   err = clEnqueueNDRangeKernel(queue, kernel_b512, 1, NULL, &global_size, 
		                        &local_size, 0, NULL, NULL); 
	   if(err < 0) {
	      perror("Couldn't enqueue kernel_b512");
	      exit(1);
	   }
	}

   }

   else if (plan->N == 1024){
	size_t global_size = 128;
	size_t local_size = 128;
	if (plan->direction == FFTW_FORWARD){
	   /* Set kernel arguments */
	   err = clSetKernelArg(kernel_f1024, 0, sizeof(cl_mem), &buffer_c);
	   if(err < 0) {
	      printf("Couldn't set a kernel argument");
	      exit(1);   
	   };
	   /* Enqueue kernel */
	   err = clEnqueueNDRangeKernel(queue, kernel_f1024, 1, NULL, &global_size, 
		                        &local_size, 0, NULL, NULL); 
	   if(err < 0) {
	      perror("Couldn't enqueue kernel_f1024");
	      exit(1);
	   }
        } else	{// inverse
	   /* Set kernel arguments */
	   err = clSetKernelArg(kernel_b1024, 0, sizeof(cl_mem), &buffer_c);
	   if(err < 0) {
	      printf("Couldn't set a kernel argument");
	      exit(1);   
	   };
	   /* Enqueue kernel */
	   err = clEnqueueNDRangeKernel(queue, kernel_b1024, 1, NULL, &global_size, 
		                        &local_size, 0, NULL, NULL); 
	   if(err < 0) {
	      perror("Couldn't enqueue kernel_b1024");
	      exit(1);
	   }
	}

   } // end N == 1024

   else if (plan->N == 2048){
	size_t global_size = 256;
	size_t local_size = 256;
	if (plan->direction == FFTW_FORWARD){
	   /* Set kernel arguments */
	   err = clSetKernelArg(kernel_f2048, 0, sizeof(cl_mem), &buffer_c);
	   if(err < 0) {
	      printf("Couldn't set a kernel argument");
	      exit(1);   
	   };
	   /* Enqueue kernel */
	   err = clEnqueueNDRangeKernel(queue, kernel_f2048, 1, NULL, &global_size, 
		                        &local_size, 0, NULL, NULL); 
	   if(err < 0) {
	      perror("Couldn't enqueue kernel_f2048");
	      exit(1);
	   }
        } else	{// inverse
	   /* Set kernel arguments */
	   err = clSetKernelArg(kernel_b2048, 0, sizeof(cl_mem), &buffer_c);
	   if(err < 0) {
	      printf("Couldn't set a kernel argument");
	      exit(1);   
	   };
	   /* Enqueue kernel */
	   err = clEnqueueNDRangeKernel(queue, kernel_b2048, 1, NULL, &global_size, 
		                        &local_size, 0, NULL, NULL); 
	   if(err < 0) {
	      perror("Couldn't enqueue kernel_b2048");
	      exit(1);
	   }
	}

   } // end N == 2048

   else if (plan->N == 4096){
	size_t global_size = 256;
	size_t local_size = 256;
	if (plan->direction == FFTW_FORWARD){
	   /* Set kernel arguments */
	   err = clSetKernelArg(kernel_f4096, 0, sizeof(cl_mem), &buffer_c);
	   if(err < 0) {
	      printf("Couldn't set a kernel argument");
	      exit(1);   
	   };
	   /* Enqueue kernel */
	   err = clEnqueueNDRangeKernel(queue, kernel_f4096, 1, NULL, &global_size, 
		                        &local_size, 0, NULL, NULL); 
	   if(err < 0) {
	      perror("Couldn't enqueue kernel_f4096");
	      exit(1);
	   }
        } else	{// inverse
	   /* Set kernel arguments */
	   err = clSetKernelArg(kernel_b4096, 0, sizeof(cl_mem), &buffer_c);
	   if(err < 0) {
	      printf("Couldn't set a kernel argument");
	      exit(1);   
	   };
	   /* Enqueue kernel */
	   err = clEnqueueNDRangeKernel(queue, kernel_b4096, 1, NULL, &global_size, 
		                        &local_size, 0, NULL, NULL); 
	   if(err < 0) {
	      perror("Couldn't enqueue kernel_b4096");
	      exit(1);
	   }
	}

   } // end N == 4096


   /* Read the results */
   err = clEnqueueReadBuffer(queue, buffer_c, CL_TRUE, 0, 
	 2*plan->N*sizeof(float), plan->out, 0, NULL, NULL);
   if(err < 0) {
      perror("Couldn't read the buffer_c output");
      exit(1);
   }
   clReleaseMemObject(buffer_c);

   sem_post(&fft_semaphore);
}

void fftcl_initialize(void){
   size_t local_size;

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
   kernel_f512 = clCreateKernel(program, "fft_fwd_512", &err);
   if(err < 0) {
      printf("Couldn't create the kernel_f512: %d", err);
      exit(1);
   };

   /* Create kernels for the FFT */
   kernel_b512 = clCreateKernel(program, "fft_back_512", &err);
   if(err < 0) {
      printf("Couldn't create the kernel_b512: %d", err);
      exit(1);
   };
   /* Create kernels for the FFT */
   kernel_f1024 = clCreateKernel(program, "fft_fwd_1024", &err);
   if(err < 0) {
      printf("Couldn't create the kernel_f1024: %d", err);
      exit(1);
   };

   /* Create kernels for the FFT */
   kernel_b1024 = clCreateKernel(program, "fft_back_1024", &err);
   if(err < 0) {
      printf("Couldn't create the kernel_b1024: %d", err);

      exit(1);
   };
   /* Create kernels for the FFT */
   kernel_f2048 = clCreateKernel(program, "fft_fwd_2048", &err);
   if(err < 0) {
      printf("Couldn't create the kernel_f2048: %d", err);
      exit(1);
   };

   /* Create kernels for the FFT */
   kernel_b2048 = clCreateKernel(program, "fft_back_2048", &err);
   if(err < 0) {
      printf("Couldn't create the kernel_b2048: %d", err);

      exit(1);
   };
   /* Create kernels for the FFT */
   kernel_f4096 = clCreateKernel(program, "fft_fwd_4096", &err);
   if(err < 0) {
      printf("Couldn't create the kernel_f4096: %d", err);
      exit(1);
   };

   /* Create kernels for the FFT */
   kernel_b4096 = clCreateKernel(program, "fft_back_4096", &err);
   if(err < 0) {
      printf("Couldn't create the kernel_b4096: %d", err);

      exit(1);
   };

   /* Determine maximum work-group size */
   err = clGetKernelWorkGroupInfo(kernel_f512, device, 
      CL_KERNEL_WORK_GROUP_SIZE, sizeof(local_size), &local_size, NULL);
   if(err < 0) {
      perror("Couldn't find the maximum work-group size");

      exit(1);   
   };
   //local_size = (int)pow(2, trunc(log2(local_size)));
   fprintf(stderr,"GPU: max workgroup size = %d\n", (int)local_size);

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

   sem_init(&fft_semaphore, 0, 1);
}

void fftcl_destroy(void){

   /* Deallocate resources */
   clReleaseKernel(kernel_f512);
   clReleaseKernel(kernel_b512);
   clReleaseKernel(kernel_f1024);
   clReleaseKernel(kernel_b1024);
   clReleaseKernel(kernel_f2048);
   clReleaseKernel(kernel_b2048);
   clReleaseKernel(kernel_f4096);
   clReleaseKernel(kernel_b4096);
   clReleaseCommandQueue(queue);
   clReleaseProgram(program);
   clReleaseContext(context);
}


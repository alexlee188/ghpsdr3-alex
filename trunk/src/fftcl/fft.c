#define _CRT_SECURE_NO_WARNINGS
#define PROGRAM_FILE "fft.cl"
#define INIT_FUNC "fft_init"
#define STAGE_FUNC "fft_stage"
#define SCALE_FUNC "fft_scale"

/* Each point contains 2 floats - 1 real, 1 imaginary */
#define NUM_POINTS 4096

/* -1 - forward FFT, 1 - inverse FFT */
#define DIRECTION 1

#include "fft_check.c"

#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <complex.h>

#ifdef MAC
#include <OpenCL/cl.h>
#else
#include <CL/cl.h>
#endif

#include "fftcl.h"


int main() {

   /* Data and buffer */

   float dataf[NUM_POINTS*2];
   float data[NUM_POINTS*2];
   double complex cdat[NUM_POINTS];
   double complex cdat_out[NUM_POINTS];
   double error, check_input[NUM_POINTS][2], check_output[NUM_POINTS][2];
   fftcl_plan *planA;
   int i;

   /* Initialize data */
   srand(time(NULL));
   for(i=0; i<NUM_POINTS; i++) {
      dataf[2*i] = rand();
      dataf[2*i+1] = rand();
      cdat[i] = dataf[2*i] + I * dataf[2*i+1];
      check_input[i][0] = dataf[2*i];
      check_input[i][1] = dataf[2*i+1];
   }

   fftcl_initialize();
   planA = fftcl_plan_create(NUM_POINTS, cdat, cdat_out, DIRECTION);
   fftcl_plan_execute(planA);

   for(i=0; i < NUM_POINTS; i++){
	data[2*i] = creal(cdat_out[i]);
	data[2*i+1] = cimag(cdat_out[i]);
   }

   fftcl_plan_destroy(planA);

   /* Compute accurate values */
   if(DIRECTION < 0)
      fft(NUM_POINTS, check_input, check_output);
   else
      ifft(NUM_POINTS, check_output, check_input);

   /* Determine error */
   error = 0.0;
   for(i=0; i<NUM_POINTS; i++) {
      error += fabs(check_output[i][0] - data[2*i])/fmax(fabs(check_output[i][0]), 0.0001);
      error += fabs(check_output[i][1] - data[2*i+1])/fmax(fabs(check_output[i][1]), 0.0001);
   }
   error = error/(NUM_POINTS*2);

   /* Display check results */
   printf("%u-point ", NUM_POINTS);
   if(DIRECTION < 0) 
      printf("FFT ");
   else
      printf("IFFT ");
   printf("completed with %lf average relative error.\n", error);

   fftcl_destroy();
   return 0;
}

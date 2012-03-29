#ifndef _FFTCL_H
#define _FFTCL_H

#include <complex.h>

typedef struct _fftcl_plan {
	int N;
	_Complex float *in;
	_Complex float *out;
	int direction;
} fftcl_plan;


fftcl_plan *fftcl_plan_create(int N, float complex *in, float complex *out, int direction);
void fftcl_plan_destroy(fftcl_plan* plan);
void fftcl_plan_execute(fftcl_plan* plan);
void fftcl_initialize(void);
void fftcl_destroy(void);

#endif

#ifndef _FFTCL_H
#define _FFTCL_H

typedef struct _fftcl_plan {
	int N;
	double complex *in;
	double complex *out;
	int direction;
} fftcl_plan;


fftcl_plan *fftcl_plan_create(int N, double complex *in, double complex *out, int direction);
void fftcl_plan_destroy(fftcl_plan* plan);
void fftcl_plan_execute(fftcl_plan* plan);
void fftcl_initialize(void);
void fftcl_destroy(void);

#endif

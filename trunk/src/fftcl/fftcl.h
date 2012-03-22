#ifndef _FFTCL_H
#define _FFTCL_H

typedef struct _fftcl_plan {
	int N;
	float *data;
	complex *out;
	int direction;
} fftcl_plan;

#endif

#ifndef _FFTCL_H
#define _FFTCL_H

#include <fromsys.h>
#include <defs.h>
#include <banal.h>
#include <splitfields.h>
#include <datatypes.h>
#include <bufvec.h>
#include <cxops.h>

typedef struct _fftcl_plan {
	int N;
	COMPLEX *in;
	COMPLEX *out;
	int direction;
} fftcl_plan;


fftcl_plan *fftcl_plan_create(int N, COMPLEX *in, COMPLEX *out, int direction);
void fftcl_plan_destroy(fftcl_plan* plan);
void fftcl_plan_execute(fftcl_plan* plan);
void fftcl_initialize(void);
void fftcl_destroy(void);

#endif

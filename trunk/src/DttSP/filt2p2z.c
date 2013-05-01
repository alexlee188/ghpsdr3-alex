#include <common.h>
#include <math.h>

#define asinh(value) (REAL)log(value + sqrt(value * value + 1))

IIR_2P2Z 
new_IIR_2P2Z (CXB buf, REAL Gain, REAL Parameter, Filter_Shape_Param ParameterType, Filter_Type FilterType, REAL Fs, REAL F0)
{
	REAL w0, sw,cw, alpha;
	IIR_2P2Z gen = (IIR_2P2Z)malloc(sizeof(iir_2p2z));
	gen->sigbuf = buf;
	gen->Gain = (REAL)sqrt(pow(10,(Gain/20)));

	gen->doComplex = FALSE;
	memset(gen->B,0,3*sizeof(REAL));
	memset(gen->A,0,3*sizeof(REAL));

	gen->out1=cxzero;
	gen->out2=cxzero;
	gen->sig1=cxzero;
	gen->sig2=cxzero;
	gen->Fs = Fs;
	gen->F0 = F0;

	w0 = (REAL)(2*M_PI*F0/Fs);

	sw = (REAL)sin(w0);
	cw = (REAL)cos(w0);

	if (ParameterType == Q)
	{
		alpha = sw/(2*Parameter);
		gen->Q = Parameter;
		gen->BW = (REAL)(2*sw*asinh(1/(2*gen->Q))/log(2)/w0);
	}
	else
	{
		// ParameterType is assumed to be Bandwidth
		alpha = (REAL)(sw*sinh(log(2)/2 * Parameter * w0/sw));
		gen->BW = Parameter;
		gen->Q = (REAL)(1/(2*sinh(log(2)/2 * gen->BW*w0/sw)));
	}
	
	switch (FilterType)
	{
		case LPF:
			// H(s) = 1/(s^2 + S/Q +1)
			gen->B[0] = (1-cw)/2;
			gen->B[1] =  (1-cw);
			gen->B[2] = (1-cw)/2;
			gen->A[0] = 1+alpha;
			gen->A[1] = -2*cw;
			gen->A[2] = 1 - alpha;
			break;
		case HPF:
			// H(s) = s^2/(s^2 + S/Q +1)
			gen->B[0] = (1+cw)/2;
			gen->B[1] =  -(1+cw);
			gen->B[2] = (1+cw)/2;
			gen->A[0] = 1+alpha;
			gen->A[1] = -2*cw;
			gen->A[2] = 1 - alpha;
			break;
		case BPF:
			// H(s) = (s/Q)/(s^2 + S/Q +1) 0 dB peak gain
			gen->B[0] = alpha;
			gen->B[1] =  0;
			gen->B[2] = -alpha;
			gen->A[0] = 1+alpha;
			gen->A[1] = -2*cw;
			gen->A[2] = 1 - alpha;
			break;
		case NOTCH:
			// H(s) = (s^2 + 1) / (s^2 + s/Q + 1)
			gen->B[0] = 1/(1+alpha);
			gen->B[1] =  -2*cw/(1+alpha);
			gen->B[2] = 1/(1+alpha);
			gen->A[0] = 1;
			gen->A[1] = -2*cw/(1+alpha);
			gen->A[2] = (1 - alpha)/(1+alpha);
			break;
		case APF:
			// H(s) = (s^2 - s/Q + 1) / (s^2 + s/Q + 1)
			gen->B[0] = 1 - alpha;
			gen->B[1] =  -2*cw;
			gen->B[2] = 1+alpha;
			gen->A[0] = 1+alpha;
			gen->A[1] = -2*cw;
			gen->A[2] = 1 - alpha;
			break; 
	}
	return gen;
}

void
del_IIR_2P2Z(IIR_2P2Z gen)
{
	if (gen) safefree((char *)gen);
}
void
do_IIR_2P2Z(IIR_2P2Z gen)
{
	int i;
	for (i=0;i < CXBsize(gen->sigbuf); i++)
	{
		REAL tmp = CXBreal(gen->sigbuf,i);
		CXBreal(gen->sigbuf,i) = tmp*gen->B[0] + gen->sig1.re*gen->B[1] + gen->sig2.re*gen->B[2]
								 - gen->out1.re*gen->A[1] - gen->out2.re*gen->A[2];
		gen->sig2.re = gen->sig1.re;
		gen->sig1.re = tmp;
		gen->out2.re = gen->out1.re;
		gen->out1.re = CXBreal(gen->sigbuf,i);
	}

	if (gen->doComplex) for (i=0;i < CXBsize(gen->sigbuf); i++)
	{
		REAL tmp = CXBimag(gen->sigbuf,i);
		CXBimag(gen->sigbuf,i) = tmp*gen->B[0] + gen->sig1.im*gen->B[1] + gen->sig2.im*gen->B[2]
								 - gen->out1.im*gen->A[1] - gen->out2.im*gen->A[2];
		gen->sig2.im = gen->sig1.im;
		gen->sig1.im = tmp;
		gen->out2.im = gen->out1.im;
		gen->out1.im = CXBimag(gen->sigbuf,i);
	}
	
}
IIR_LPF_2P 
new_IIR_LPF_2P (CXB buf, REAL samplerate, REAL cuttoff_freq_Hz, REAL Q)				  
{
	REAL w = (REAL)TWOPI*cuttoff_freq_Hz / samplerate;
	//REAL T = 1/samplerate;
	IIR_LPF_2P lpf = (IIR_LPF_2P)malloc(sizeof(iir_lpf_2p));
	lpf->sigbuf = buf;
	lpf->kf = (w*w)/(1 + Q*w + w*w);  
	lpf->kq = (Q*w)/(1 + Q*w + w*w); 
	lpf->out1 = 0.0f;
	lpf->out2 = 0.0f;
	return lpf;
}

void
del_IIR_LPF_2P(IIR_LPF_2P lpf)
{
	if (lpf) safefree((char *)lpf);
}
void do_IIR_LPF_2P (IIR_LPF_2P lpf)
{
	//int CXB_size = sizeof(lpf->sigbuf)/sizeof(CXB);
	int i = 0;
	for(i = 0; i < CXBsize(lpf->sigbuf); i++)
	{
		CXBreal(lpf->sigbuf, i) = lpf->kf * CXBreal(lpf->sigbuf, i) + 2*lpf->out1 - lpf->kq*lpf->out1 - 2*lpf->kf*lpf->out1 - lpf->out2 + lpf->kq*lpf->out2 + lpf->kf*lpf->out2;
		lpf->out2 = lpf->out1;
		lpf->out1 = CXBreal(lpf->sigbuf, i);		
	}
}


IIR_BPF_2P 
new_IIR_BPF_2P (CXB buf, REAL samplerate, REAL cuttoff_freq_Hz, REAL Q)				  
{
	REAL w = (REAL)TWOPI*cuttoff_freq_Hz / samplerate;
	REAL T = 1/samplerate;
	IIR_BPF_2P bpf = (IIR_BPF_2P)malloc(sizeof(iir_bpf_2p));
	bpf->sigbuf = buf;
	bpf->kf = (w*w)/(1 + Q*w + w*w);  
	bpf->kq = (Q*w)/(1 + Q*w + w*w); 
	bpf->out1 = 0.0f;
	bpf->out2 = 0.0f;
	bpf->sig1 = 0.0f;
	return bpf;
}
void
del_IIR_BPF_2P(IIR_BPF_2P bpf)
{
	if (bpf) safefree((char *)bpf);
}
void do_IIR_BPF_2P (IIR_BPF_2P bpf)
{
	int i = 0;
	REAL sig1temp;
	for(i = 0; i < CXBsize(bpf->sigbuf); i++)
	{
		sig1temp = CXBreal(bpf->sigbuf, i);	//save for previous input sample next time (bpf->sig1)
		CXBreal(bpf->sigbuf, i) = bpf->kq*CXBreal(bpf->sigbuf, i) - bpf->kq*bpf->sig1 + 2*bpf->out1 - bpf->kq*bpf->out1 - 2*bpf->kf*bpf->out1 - bpf->out2 + bpf->kq*bpf->out2 + bpf->kf*bpf->out2;
		bpf->out2 = bpf->out1;
		bpf->out1 = CXBreal(bpf->sigbuf, i);		
		bpf->sig1 = sig1temp;
	}
}

IIR_HPF_2P 
new_IIR_HPF_2P (CXB buf, REAL samplerate, REAL cuttoff_freq_Hz, REAL Q)				  
{
	REAL w = (REAL)TWOPI*cuttoff_freq_Hz / samplerate;
	REAL T = 1/samplerate;
	IIR_HPF_2P hpf = (IIR_HPF_2P)malloc(sizeof(iir_hpf_2p));
	hpf->sigbuf = buf;
	hpf->kf = (w*w)/(4 + 2*Q*w + w*w);  
	hpf->kq = (2*Q*w)/(4 + 2*Q*w + w*w); 
	hpf->out1 = 0.0f;
	hpf->out2 = 0.0f;
	hpf->sig1 = 0.0f;
	hpf->sig2 = 0.0f;
	return hpf;
}

void
del_IIR_HPF_2P(IIR_HPF_2P hpf)
{
	if (hpf) safefree((char *)hpf);
}
void do_IIR_HPF_2P (IIR_HPF_2P hpf)
{
	int i = 0;
	REAL sig1temp;
	for(i = 0; i < CXBsize(hpf->sigbuf); i++)
	{
		sig1temp = CXBreal(hpf->sigbuf, i);	//save for previous input sample next time (bpf->sig1)
		CXBreal(hpf->sigbuf, i) = 2*hpf->out1 - 2*hpf->kq*hpf->out1 - 4*hpf->kf*hpf->out1
			- hpf->out2 + 2*hpf->kq*hpf->out2
			+ (1 - hpf->kq - hpf->kf)*(CXBreal(hpf->sigbuf, i) - 2*hpf->sig1 + hpf->sig2);
		hpf->out2 = hpf->out1;
		hpf->out1 = CXBreal(hpf->sigbuf, i);		
		hpf->sig2 = hpf->sig1;
		hpf->sig1 = sig1temp;
		
	}
}

IIR_1P1Z 
new_IIR_1P1Z (CXB buf, REAL samplerate, REAL pole_freq_hz, REAL zero_freq_hz)				  
{
	REAL wp = (REAL) TWOPI * pole_freq_hz / samplerate;
	REAL wz = (REAL) TWOPI * zero_freq_hz / samplerate;
	REAL kp = wp/(2+wp); 
	REAL kz = wz/(2+wp);  
	IIR_1P1Z filter = (IIR_1P1Z)malloc(sizeof(iir_1p1z));
	filter->sigbuf = buf;
	filter->b0 = 1-kp+kz;		// numerator Vi(n) term
	filter->b1 = -1+kp+kz;  	// numerator Vi(n-1) term
	filter->a1 = -1+2*kp;	 	// denominator Vo(n-1) term
	filter->gain = wp/wz;	 	// gain factor to achieve unity gain at low freq
	filter->out1 = 0.0f;
	filter->sig1 = 0.0f;
	return filter;
}

void
del_IIR_1P1Z(IIR_1P1Z filter)
{
	if (filter) safefree((char *)filter);
}
void do_IIR_1P1Z (IIR_1P1Z filter)
{
	int i = 0;
	REAL sig1temp;

	for(i = 0; i < CXBsize(filter->sigbuf); i++)
	{
		sig1temp = filter->gain * CXBreal(filter->sigbuf, i);
		// Vo(n) = (1-kp+kz)wp/wz*Vi(n) + (-1+kp+kz)wp/wz*Vi(n-1) - (-1+2kp)Vo(n-1)
		CXBreal(filter->sigbuf, i) = filter->b0 * sig1temp
				+ filter->b1 * filter->sig1
				- filter->a1 * filter->out1;
		filter->out1 = CXBreal(filter->sigbuf, i);		
		filter->sig1 = sig1temp;		
	}
}

/** 

*/
#ifndef filt2p2z
#define filt2p2z
#include <bufvec.h>

typedef enum filter_shape_param {
	Q,
	BW,
	ShelfSlope
} Filter_Shape_Param;

typedef enum filter_type {
	LPF,
	HPF,
	BPF,
	APF,
	NOTCH
} Filter_Type;

typedef struct _iir_lpf_2p {
  CXB sigbuf;
  REAL kq;
  REAL kf;
  REAL out1;
  REAL out2;
} iir_lpf_2p,*IIR_LPF_2P;


typedef struct _iir_bpf_2p {
  CXB sigbuf;
  REAL kq;
  REAL kf;
  REAL sig1;
  REAL out1;
  REAL out2;
} iir_bpf_2p,*IIR_BPF_2P;

typedef struct _iir_hpf_2p {
  CXB sigbuf;
  REAL kq;
  REAL kf;
  REAL sig1;
  REAL sig2;
  REAL out1;
  REAL out2;
} iir_hpf_2p,*IIR_HPF_2P;


typedef struct _iir_2p2z {
	CXB sigbuf;
	REAL Q;
	REAL Gain;
	REAL BW;
	REAL F0;
	REAL Fs;
	REAL B[3];
	REAL A[3];
	COMPLEX sig1;
	COMPLEX sig2;
	COMPLEX out1;
	COMPLEX out2;
	BOOLEAN doComplex;
} iir_2p2z, *IIR_2P2Z;

typedef struct _iir_1p1z {
  CXB sigbuf;
  REAL b0;
  REAL b1;
  REAL a1;
  REAL gain;
  REAL sig1;
  REAL out1;
} iir_1p1z,*IIR_1P1Z;

extern IIR_LPF_2P new_IIR_LPF_2P (CXB buf, REAL samplerate, REAL cuttoff_freq_Hz, REAL Q);
extern void del_IIR_LPF_2P(IIR_LPF_2P lpf);
extern void do_IIR_LPF_2P (IIR_LPF_2P lpf);

extern IIR_BPF_2P new_IIR_BPF_2P (CXB buf, REAL samplerate, REAL cuttoff_freq_Hz, REAL Q);
extern void del_IIR_BPF_2P(IIR_BPF_2P bpf);
extern void do_IIR_BPF_2P (IIR_BPF_2P bpf);

extern IIR_HPF_2P new_IIR_HPF_2P (CXB buf, REAL samplerate, REAL cuttoff_freq_Hz, REAL Q);
extern void del_IIR_HPF_2P(IIR_HPF_2P HPF);
extern void do_IIR_HPF_2P (IIR_HPF_2P hpf);

extern IIR_2P2Z new_IIR_2P2Z (CXB buf, REAL Gain, REAL Parameter,Filter_Shape_Param ParameterType,Filter_Type FilterType,REAL Fs,REAL F0);
extern void del_IIR_2P2Z(IIR_2P2Z iirgen);
extern void do_IIR_2P2Z(IIR_2P2Z iirgen);

extern IIR_1P1Z new_IIR_1P1Z (CXB buf, REAL samplerate, REAL pole_freq_hz, REAL zero_freq_hz);
extern void del_IIR_1P1Z (IIR_1P1Z filter);
extern void do_IIR_1P1Z (IIR_1P1Z filter);

#endif
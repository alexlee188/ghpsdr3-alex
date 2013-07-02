/** 
* @file dttsp.h
* @brief DttSP interface definitions
* @author John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* @version 0.1
* @date 2009-04-11
*/
// dttsp.h

/* Copyright (C) 
* 2009 - John Melton, G0ORX/N6LYT, Doxygen Comments Dave Larsen, KV0S
* This program is free software; you can redistribute it and/or
* modify it under the terms of the GNU General Public License
* as published by the Free Software Foundation; either version 2
* of the License, or (at your option) any later version.
* 
* This program is distributed in the hope that it will be useful,
* but WITHOUT ANY WARRANTY; without even the implied warranty of
* MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
* GNU General Public License for more details.
* 
* You should have received a copy of the GNU General Public License
* along with this program; if not, write to the Free Software
* Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
* 
*/

#if !defined __DTTSP_H__
#define __DTTSP_H__

#include "complex.h"
#include "datatypes.h"
#include "defs.h"
#include "bufvec.h"
#include "dttspagc.h"
#include <sdrexport.h>

//
// what we know about DttSP
//

/* --------------------------------------------------------------------------*/
/** 
* @brief Setup_SDR  
* 
* @return
*/
extern void Setup_SDR();
extern void Destroy_SDR();

/* --------------------------------------------------------------------------*/
/** 
* @brief Release_Update  
* 
* @return
*/
extern void Release_Update();

/* --------------------------------------------------------------------------*/
/** 
* @brief SetThreadCom  
*
* @param thread
* 
* @return
*/
extern void SetThreadCom(int thread);

/* --------------------------------------------------------------------------*/
/** 
* @brief DttSP audio callback 
* 
* @param input_l
* @param input_r
* @param output_l
* @param output_r
* @param nframes
* 
* @return 
*/
extern void Audio_Callback (float *input_l, float *input_r, float *output_l,
                            float *output_r, unsigned int nframes, int thread);

/* --------------------------------------------------------------------------*/
/** 
* @brief Process the spectrum 
* 
* @param thread
* @param results
* 
* @return 
*/
extern void Process_Spectrum (int thread, float *results);
/* --------------------------------------------------------------------------*/
/** 
* @brief Process Panadapter
* 
* @param thread
* @param results
* 
* @return 
*/
extern void Process_Panadapter (int thread, float *results);
/* --------------------------------------------------------------------------*/
/** 
* @brief Process Phase 
* 
* @param thread
* @param results
* @param numpoints
* 
* @return 
*/
extern void Process_Phase (int thread, float *results, int numpoints);
/* --------------------------------------------------------------------------*/
/** 
* @brief Process scope 
* 
* @param thread
* @param results
* @param numpoints
* 
* @return 
*/
extern void Process_Scope (int thread, float *results, int numpoints);
/* --------------------------------------------------------------------------*/
/** 
* @brief Calculate the RX meter 
* 
* @param subrx
* @param mt
* 
* @return 
*/
extern float CalculateRXMeter(int thread,unsigned int subrx, int mt);

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the sample rate 
* 
* @param sampleRate
* 
* @return 
*/
extern int SetSampleRate(double sampleRate);

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the Oscillator frequency 
* 
* @param frequency
* 
* @return 
*/
extern int SetRXOsc(unsigned int thread, unsigned subrx, double freq);

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the receiver output gain
* 
* @param gain
* 
* @return 
*/
extern int SetRXOutputGain(unsigned int thread, unsigned subrx, double gain);

/* --------------------------------------------------------------------------*/
/** 
* @brief Set the receiver pan position
* 
* @param pos
* 
* @return 
*/
extern int SetRXPan(unsigned int thread, unsigned subrx, float pos);
extern int SetRingBufferOffset(unsigned int thread, int offset);
extern void SetSpectrumPolyphase (unsigned int thread, BOOLEAN setit);
extern void SetSquelchVal (unsigned int thread, unsigned int subrx, float setit);
extern void SetSquelchState (unsigned int thread, unsigned int subrx, BOOLEAN setit);
extern void SetRXDCBlock(unsigned int thread, unsigned int subrx, BOOLEAN setit);
extern void SetNBvals (unsigned int thread, unsigned subrx, double threshold);
extern void SetANRvals (unsigned int thread, unsigned subrx, int taps, int delay, double gain, double leakage);
extern void SetMode (unsigned int thread, unsigned int subrx, SDRMODE m);
extern void SetRXFilter (unsigned int thread, unsigned int subrx, double low_frequency, double high_frequency);
extern void SetRXAGC (unsigned int thread, unsigned subrx, AGCMODE setit);
extern void SetANR (unsigned int thread, unsigned subrx, BOOLEAN setit);
extern void SetNB (unsigned int thread, unsigned subrx, BOOLEAN setit);
extern void SetSDROM (unsigned int thread, unsigned subrx, BOOLEAN setit);
extern void SetANF (unsigned int thread, unsigned subrx, BOOLEAN setit);
extern void SetSubRXSt(unsigned int thread, unsigned int subrx, BOOLEAN setit);
extern void SetSDROMvals (unsigned int thread, unsigned subrx, double threshold);
extern void SetANFvals (unsigned int thread, unsigned subrx, int taps, int delay, double gain, double leakage);
extern void SetTRX (unsigned int thread, TRXMODE setit);
extern void SetThreadProcessingMode (unsigned int thread, RUNMODE runmode);
extern int reset_for_buflen (unsigned int, int);
extern void SetTXDCBlock (unsigned int thread, BOOLEAN setit);
extern void SetTXFMDeviation(unsigned int thread, double deviation);
extern int SetTXFilter (unsigned int thread, double low_frequency, double high_frequency);
extern int SetTXOsc (unsigned int thread, double newfreq);
//extern void SetTXCompandSt (unsigned int thread, BOOLEAN setit);
//extern void SetTXCompand (unsigned int thread, double setit);
extern void SetTXAMCarrierLevel (unsigned int thread, double setit);
extern void SetTXLevelerSt (unsigned int thread, BOOLEAN state);
extern void SetTXLevelerAttack (unsigned int thread, int attack);
extern void SetTXLevelerHang (unsigned int thread, int decay);
extern void SetTXLevelerTop (unsigned int thread, double maxgain);
//extern float CalculateTXMeter(int thread,unsigned int subrx);
extern void SetCorrectIQEnable(int setit);
extern void SetCorrectRXIQMu (unsigned int thread, unsigned int subrx, double mu);

// Following lines added by KD0OSS
extern void SetPWSmode (unsigned thread, unsigned subrx, int setit);
extern void SetWindow (unsigned int thread, Windowtype window);
extern void SetGrphTXEQ (unsigned int thread, int *txeq);
extern void SetGrphTXEQ10(unsigned int thread, int *txeq); 
extern void SetGrphTXEQcmd (unsigned int thread, BOOLEAN state);
extern void SetGrphRXEQ (unsigned int thread, unsigned int subrx, int *rxeq);
extern void SetGrphRXEQ10(unsigned int thread, unsigned int subrx, int *rxeq) ;
extern void SetGrphRXEQcmd (unsigned int thread, unsigned int subrx,BOOLEAN state);
extern void SetRXManualNotchEnable(unsigned int thread, unsigned int subrx, unsigned int index, BOOLEAN setit);
extern void SetRXManualNotchBW(unsigned int thread, unsigned int subrx, unsigned int index, double BW);
extern void SetRXManualNotchFreq(unsigned int thread, unsigned int subrx, unsigned int index, double F0);
extern void SetFixedAGC (unsigned int thread, unsigned int subrx, double fixed_agc);
extern float CalculateTXMeter (unsigned int thread, METERTYPE mt);
extern void SetCorrectTXIQ (unsigned int thread, double phase, double gain);
extern void SetCorrectTXIQGain (unsigned int thread, double gain);
extern void SetCorrectTXIQPhase (unsigned int thread, double phase);
extern void SetCorrectIQGain (unsigned int thread, unsigned int subrx, double gain);
extern void SetCorrectIQPhase (unsigned int thread, unsigned int subrx, double phase);
// Added 2013-06-09 by KD0OSS
extern void Process_ComplexSpectrum (unsigned int thread, float *results);
extern void SetSBMode(unsigned int thread, unsigned int subrx, int sbmode);
extern void SetRXDCBlockGain(unsigned int thread, unsigned int subrx, REAL gain);
extern void SetOscPhase(double phase);
extern void SetBIN (unsigned int thread, unsigned subrx, BOOLEAN setit);
extern void SetTXCompressorSt (unsigned int thread, BOOLEAN setit);
extern void SetTXCompressor (unsigned int thread, double setit);
extern void SetRXAGCAttack (unsigned int thread, unsigned subrx, int attack);
extern void SetRXAGCDecay (unsigned int thread, unsigned subrx, int decay);
extern void SetRXAGCHang (unsigned int thread, unsigned subrx, int hang);
extern void SetRXAGCHangLevel(unsigned int thread, unsigned int subrx, double hangLevel);
extern void SetRXAGCHangThreshold (unsigned int thread, unsigned int subrx, int hangthreshold);
extern void SetRXAGCThresh(unsigned int thread, unsigned int subrx, double thresh);
extern void SetRXAGCTop (unsigned int thread, unsigned int subrx, double max_agc);
extern void SetRXAGCSlope (unsigned int thread, unsigned subrx, int slope);
extern void SetCorrectRXIQwReal (unsigned int thread, unsigned int subrx, REAL wr, unsigned int index);
extern void SetCorrectRXIQwImag (unsigned int thread, unsigned int subrx, REAL wi, unsigned int index);
extern void SetTXALCAttack (unsigned int thread, int attack);
extern void SetTXALCDecay (unsigned int thread, int decay);
extern void SetTXALCBot (unsigned int thread, double max_agc);
extern void SetTXALCHang (unsigned int thread, int decay);
extern void SetTXALCSt (unsigned int thread, BOOLEAN state);
extern void SetTXLevelerDecay (unsigned int thread, int decay);
extern void SetFadeLevel(unsigned int thread, unsigned int subrx, int levelfade);
extern void SetTXAGCFF (unsigned int thread, BOOLEAN setit);
extern void SetTXAGCFFCompression (unsigned int thread, double txcompression);
extern void SetCorrectTXIQMu (unsigned int thread, double mu);
extern void SetCorrectTXIQW (unsigned int thread, double wr, double wi);
// need to be added
extern void SetANFposition (unsigned int thread, unsigned subrx, int position);
extern void SetANRposition (unsigned int thread, unsigned subrx, int position);
extern void SetCTCSSFreq(unsigned int thread, double freq_hz);
extern void SetCTCSSFlag(unsigned int thread, BOOLEAN flag);
extern void SetFMSquelchThreshold(unsigned int thread, unsigned int k, REAL threshold);
extern void SetCorrectIQ (unsigned int thread, unsigned int subrx, double phase, double gain);
extern void SetCorrectRXIQw (unsigned int thread, unsigned int subrx, REAL wr, REAL wi, unsigned int index);
extern void SetTXSquelchSt (unsigned int thread, BOOLEAN setit);
extern void SetTXSquelchVal (unsigned int thread, float setit);
extern void SetTXSquelchAtt (unsigned int thread, float setit);
extern void DelPolyPhaseFIR (ResSt resst);
extern void DelPolyPhaseFIRF (ResSt resst);
extern void SetDiversity (int setit);
extern void SetDiversityScalar(REAL re, REAL im);
extern void SetDiversityGain(REAL gain);
// These functions require a way to return a value.
extern void GetRXAGCHangLevel(unsigned int thread, unsigned int subrx, double *hangLevel);
extern void GetRXAGCHangThreshold(unsigned int thread, unsigned int subrx, int *hangthreshold);
extern void GetRXAGCThresh(unsigned int thread, unsigned int subrx, double *thresh);
extern void GetRXAGCTop(unsigned int thread, unsigned int subrx, double *max_agc);
extern REAL GetCorrectRXIQMu(unsigned int thread, unsigned int subrx);
extern void GetCorrectRXIQw(int thread, int subrx, REAL *realw, REAL *imagw, unsigned int index);
// The following functions may not need to be added
extern void FlushAllBufs (unsigned int thread, BOOLEAN trx);
extern void SetDSPBuflen (unsigned int thread, int newBuffSize);
extern void SetAudioSize (unsigned int size);
extern void NewResampler (int samplerate_in, int samplerate_out);
extern void DoResampler (COMPLEX * input, COMPLEX * output, int numsamps, int *outsamps,ResSt ptr);
extern void NewResamplerF (int samplerate_in, int samplerate_out);
extern void DoResamplerF (float *input, float *output, int numsamps, int *outsamps, ResStF ptr);

#endif

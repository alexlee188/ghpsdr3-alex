/**
* @file softrockio.h
* @brief Audio I/O
* @author John Melton, G0ORX/N6LYT
* @version 0.1
* @date 2009-10-13
*/

/* Copyright (C)
* 2009 - John Melton, G0ORX/N6LYT
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

typedef struct tag_readblock{
	unsigned char LeftChannelIQSamplePack[32*4];
	unsigned char RightChannelIQSamplePack[32*4];
	unsigned short LeftChannelAudioSamplePack[32];
	unsigned char unused320;
	unsigned char unused321;
	unsigned char unused322;
	unsigned char unused323;
	unsigned char ControlFlagValid;
	unsigned char unused325;
	unsigned char unused326;
	unsigned char unused327;
	unsigned char unused328;
	unsigned char Dot;
	unsigned char unused330;
	unsigned char Dash;
	unsigned char unused332;
	unsigned char unused333;
	unsigned char unused334;
	unsigned char unused335;
	unsigned char unused336;
	unsigned char unused337;
	unsigned short RunningCounter;
	unsigned short ReadBadBlocksReportedFromDevice;
	unsigned short WriteBadBlocksReportedFromDevice;
	unsigned char ControlA_unused[40];
	unsigned short RightChannelAudioSamplePack[32];
	unsigned char ControlB_unused[64];
}RD_BLOCK;

typedef struct tag_writeblock{
	unsigned long LeftChannelIQSamplePack[32];
	unsigned long RightChannelIQSamplePack[32];
	unsigned short LeftChannelAudioSamplePack[32];
	unsigned char unused320;
	unsigned char unused321;
	unsigned char unused322;
	unsigned char unused323;
	unsigned char unused324;
	unsigned char unused325;
	unsigned char unused326;
	unsigned char unused327;
	unsigned char unused328;
	unsigned char unused329;
	unsigned char unused330;
	unsigned char unused331;
	unsigned char unused332;
	unsigned char Xmit;
	unsigned char unused334;
	unsigned char Dir;
	unsigned char unused336;
	unsigned char BPF0;
	unsigned char unused338;
	unsigned char BPF1;
	unsigned char unused340;
	unsigned char BPF2;
	unsigned char unused342;
	unsigned char Isolate;
	unsigned char ControlA_unused[40];
	unsigned short RightChannelAudioSamplePack[32];
	unsigned char i2c_cmds;
	unsigned char ControlB_unused[63];
}WR_BLOCK;

#define IQ_RINGBUF_SIZE		1024

typedef struct tag_ringbuf{
	float buf[IQ_RINGBUF_SIZE];
	float *buf_end;
	float *rp;
	float *wp;
	int count;	//values in buf
	char name[255];
}RINGBUF;

int softrock_open(void);
int softrock_close();
#ifdef PULSEAUDIO
int softrock_write(float* left_samples,float* right_samples);
int softrock_read(float* left_samples,float* right_samples);
#endif
#ifdef PORTAUDIO
int softrock_write(float* left_samples,float* right_samples);
int softrock_read(float* left_samples,float* right_samples);
#endif
#ifdef DIRECTAUDIO
int softrock_write(unsigned char* buffer,int buffer_size);
int softrock_read(unsigned char* buffer,int buffer_size);
#endif


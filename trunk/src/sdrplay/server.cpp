/**
* @file server.c
* @brief HiQSDR server application
* @author Andrea Montefusco, IW0HDV
* @version 0.1
* @date 4/1/2012
*/


/* Copyright (C)
*  2012 - Andrea Montefusco, IW0HDV
*  This program is free software; you can redistribute it and/or
*  modify it under the terms of the GNU General Public License
*  as published by the Free Software Foundation; either version 2
*  of the License, or (at your option) any later version.
*
*  This program is distributed in the hope that it will be useful,
*  but WITHOUT ANY WARRANTY; without even the implied warranty of
*  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
*  GNU General Public License for more details.
*
*  You should have received a copy of the GNU General Public License
*  along with this program; if not, write to the Free Software
*  Foundation, Inc., 59 Temple Place - Suite 330, Boston, MA  02111-1307, USA.
*
*/

//#include <boost/program_options.hpp>
//#using namespace boost::program_options;
#include <iostream>
#include <string>
using namespace std;

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "client.h"
#include "listener.h"
#include "mirsdrapi-rsp.h"
#include "receiver.h"

SdrPlayConfig cfg;

int debug_level = 0;

// Megahertz of band edges, init sets rf front end,
// frequency can only move around inside the band edges
// (but that doesn't make sense, could tune outside the edges
//  and you'd just start dealing more with the filter skirts)
// In any case, the band edges get set by init and you need to
// uninit and re-init to change to a different band.

double band_breaks[] = {
  0.100, 12.0, 30.0, 60.0, 120.0, 250.0, 420.0, 1000.0, 2000.0
};

void print_help()
{
  // do not offer non-zero if, it returns real samples instead of complex
  // gain reduction ranges from 0 to 102 dB below 420MHz, from 0 to 85 dB above 420 MHz
  // ah, 1536000 is 8 * 192000
  // but samplerate remains
    cerr << 
      "    Allowed options:"                                    << endl <<
      "      -f [ --frequency ]  initial frequency in Hertz"    << endl <<
      "      -s [ --samplerate ] samplerate in Samples/second"  << endl <<
      "                          from 500000 to 12000000"       << endl <<
      "                          def=768000"                    << endl <<
      "      -b [ --bandwidth]   bandwith in Hertz"             << endl <<
      "                          200000 | 300000 | 600000 |"    << endl <<
      "                          1536000 | 5000000 | 6000000 |" << endl <<
      "                          7000000 | 8000000"             << endl <<
      "                          def=1536000"                   << endl <<
      "      -i [ --if ]         if offset in kHz"              << endl <<
      "                          0 | 450 | 1620 | 2048, def=0"  << endl <<
      "      -g [ --gain ]       gain in dB or -1 for AGC"      << endl <<
      "      -h [ --help ]       print usage message"           << endl <<
      "      -d [ --debug ]      debug level (default=0)"       << endl <<
      endl;
}

int parseOptions (int argc, char **argv, SdrPlayConfig &cfg)
{
  int c;
  int rc = 0;

  while (1) {
    static struct option long_options[] =
      {
	/* These options set a flag. */
	{"help",       no_argument,       0,  1 },
	{"debug",      required_argument, 0, 'd'},
	{"frequency",  required_argument, 0, 'f'},
	{"samplerate", required_argument, 0, 's'},
	{"gain",       required_argument, 0, 'g'},
	{"bandwidth",  required_argument, 0, 'b'},
	{"if",         required_argument, 0, 'i'},
	{0, 0, 0, 0}
      };
    /* getopt_long stores the option index here. */
    int option_index = 0;

    c = getopt_long (argc, argv, "hd:f:s:g:", long_options, &option_index);

    /* Detect the end of the options. */
    if (c == -1)
      break;

    switch (c) {
    case 's': cfg.sr = atoi(optarg); break;
    case 'g': cfg.gain = atoi(optarg); break;
    case 'f': cfg.freq = atoi(optarg); break;
    case 'd': debug_level = atoi(optarg); break;
    case 'b': cfg.bw = atoi(optarg); break;
    case 'i': cfg.ift = atoi(optarg); break;

    case 1:
    case '?':
    case 'h':
    default:
      /* getopt_long already printed an error message. */
      print_help();
      return -1;
    }
  }
  return rc;
}

int translateGain(SdrPlayConfig &cfg) {
  if (cfg.gain < -1 || cfg.gain > 85) {
    // out of bounds
    return -1;
  }
  cfg.gRdB = 85-cfg.gain;
  return 0;
}
int translateFreq(SdrPlayConfig &cfg) {
  if (cfg.freq < 100000 || cfg.freq > 2000000000) return -1;
  cfg.rfMHz = cfg.freq / 1.0e6;
  return 0;
}  
int translateSR(SdrPlayConfig &cfg) {
  if (cfg.sr < 48000 || cfg.sr > 12000000) return -1;
  cfg.fsMHz = double(cfg.sr) / 1.0e6;
  return 0;
}
int translateBW(SdrPlayConfig &cfg) {
  switch (cfg.bw) {
  case 200000:  cfg.bwType = mir_sdr_BW_0_200; break;
  case 300000:  cfg.bwType = mir_sdr_BW_0_300; break;
  case 600000:  cfg.bwType = mir_sdr_BW_0_600; break;
  case 1536000: cfg.bwType = mir_sdr_BW_1_536; break;
  case 5000000: cfg.bwType = mir_sdr_BW_5_000; break;
  case 6000000: cfg.bwType = mir_sdr_BW_6_000; break;
  case 7000000: cfg.bwType = mir_sdr_BW_7_000; break;
  case 8000000: cfg.bwType = mir_sdr_BW_8_000; break;
  default:
    // out of bounds
    return -1;
  }
  return 0;
}
int translateIF(SdrPlayConfig &cfg) {
  switch (cfg.ift) {
  case 0: cfg.ifType = mir_sdr_IF_Zero; break;
  case 450: cfg.ifType = mir_sdr_IF_0_450; break;
  case 1620: cfg.ifType = mir_sdr_IF_1_620; break;
  case 2048: cfg.ifType = mir_sdr_IF_2_048; break;
  default: return -1;
  }
  return 0;
}

int translateOptions (SdrPlayConfig &cfg)
{
  if (translateGain(cfg) < 0) return -1;
  if (translateFreq(cfg) < 0) return -1;
  if (translateSR(cfg) < 0) return -1;
  if (translateBW(cfg) < 0) return -1;
  if (translateIF(cfg) < 0) return -1;
  return 0;
}

int main(int argc, char* argv[]) {

  cfg.bw = 600000;
  cfg.sr = 768000;
  cfg.gain = 40;
  cfg.freq = 14500000;
  cfg.ift = 0;
  debug_level = 3;

  if (parseOptions (argc, argv, cfg) != 0 || translateOptions(cfg) != 0) {
    return 255;
  }

  cout << "Debug level:          " << debug_level <<  std::endl;
  cout << "Sample rate:          " << cfg.sr      <<  std::endl;
  cout << "Gain:                 " << cfg.gain    <<  std::endl;
  cout << "Freq:                 " << cfg.freq    <<  std::endl;

  // Init RTL SDR
  float ver;
  if (mir_sdr_ApiVersion(&ver) != mir_sdr_Success || ver != MIR_SDR_API_VERSION) {
    cout << "Mirics API version mismatch" << endl;
    return 254;
  }
  mir_sdr_ErrT err = mir_sdr_Success;
  int samplesPerPacket = 0;
  if ( ( err = mir_sdr_Init(cfg.gRdB, cfg.fsMHz, cfg.rfMHz, cfg.bwType, cfg.ifType, &samplesPerPacket))  != mir_sdr_Success)  {
    cout << "No SDRPlay hardware detected" << endl;
    return 255;
  }
  cout << "SDRplay hardware wants " << samplesPerPacket << " samples/packet" << endl;
  mir_sdr_Uninit();
  init_receivers (&cfg);
  create_listener_thread();

#if 1
  while(1) sleep(10);
#else
  cout << "Press q <ENTER> to exit." << endl;
  char ch;
  while((ch = getc(stdin)) != EOF) {
    if (ch == 'q') break;
  }
#endif
  stop_receiver(&receiver[0]);
  return 0;
}


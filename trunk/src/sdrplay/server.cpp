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
#include "receiver.h"

SdrPlayConfig cfg;

int debug_level = 0;

void print_help()
{
  // do not offer non-zero if, it returns real samples instead of complex
  // gain reduction ranges from 0 to 102 dB below 420MHz, from 0 to 85 dB above 420 MHz
  // but hardware samplerate is undocumented, determined by experiment
    cerr << 
      "    Allowed options:"                                    << endl <<
      "      -f [ --frequency ]  initial frequency in Hertz"    << endl <<
      "      -s [ --samplerate ] samplerate in Samples/second"  << endl <<
      "                          24000 |48000 | 96000 | 192000 |" << endl <<
      "                          384000 | 768000 | 1536000,"    << endl <<
      "                          def=384000"                    << endl <<
      "      -i [ --if ]         if offset in kHz"              << endl <<
      "                          0 | 450 | 1620 | 2048, def=0"  << endl <<
      "      -g [ --gain ]       gain in dB or -1 for AGC"      << endl <<
      "                          (no agc at present)"           << endl <<
      "      -h [ --help ]       print usage message"           << endl <<
      "      -d [ --debug ]      debug level (default=0)"       << endl <<
      endl;
}

int parseOptions (int argc, char **argv, SdrPlayConfig &cfg) {
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

    c = getopt_long (argc, argv, "hd:f:s:g:i:", long_options, &option_index);

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

int validateGain(SdrPlayConfig &cfg) {
  if (cfg.gain < -1 || cfg.gain > 85) {
    // out of bounds
    return -1;
  }
  return 0;
}
int validateFreq(SdrPlayConfig &cfg) {
  if (cfg.freq < 100000 || cfg.freq > 2000000000) return -1;
  return 0;
}  
int validateSR(SdrPlayConfig &cfg) {
  switch (cfg.sr) {
  case 24000:			// m = 64
  case 48000:			// m = 32
  case 96000:			// m = 16
  case 192000:			// m = 8
  case 384000:			// m = 4
  case 768000:			// m = 2
  case 1536000:			// m = 1
  case 22050:			// m = 64
  case 44100:			// m = 32
  case 88200:			// m = 16
  case 176400:			// m = 8
  case 352800:			// m = 4
  case 705600:			// m = 2
  case 1411200:			// m = 1
    return 0;
  }
  return -1;
}
int validateBW(SdrPlayConfig &cfg) {
  switch (cfg.bw) {
  case 200000:
  case 300000:
  case 600000:
  case 1536000:
  case 5000000:
  case 6000000:
  case 7000000:
  case 8000000:
    return 0;
  default:
    return -1;
  }
}
int validateIF(SdrPlayConfig &cfg) {
  switch (cfg.ift) {
  case 0:
  case 450:
  case 1620:
  case 2048:
    return 0;
  default:
    return -1;
  }
}

int validateOptions (SdrPlayConfig &cfg)
{
  if (validateGain(cfg) < 0) return -1;
  if (validateFreq(cfg) < 0) return -1;
  if (validateSR(cfg) < 0) return -1;
  if (validateBW(cfg) < 0) return -1;
  if (validateIF(cfg) < 0) return -1;
  return 0;
}

int main(int argc, char* argv[]) {

  cfg.bw = 1536000;
  cfg.sr = 192000;
  cfg.gain = 40;
  cfg.freq = 14500000;
  cfg.ift = 0;
  debug_level = 3;

  if (parseOptions (argc, argv, cfg) != 0 || validateOptions(cfg) != 0) {
    return 255;
  }

  cout << "Debug level:          " << debug_level <<  endl;
  cout << "Sample rate:          " << cfg.sr      <<  endl;
  cout << "Gain:                 " << cfg.gain    <<  endl;
  cout << "Freq:                 " << cfg.freq    <<  endl;

  if (init_receivers (&cfg) == 0) {
    create_listener_thread();

    cout << "Press q <ENTER> to exit." << endl;
    char ch;
    while((ch = getc(stdin)) != EOF)
      if (ch == 'q')
	break;
    
    stop_receiver(&receiver[0]);
    return 0;
  } else {
    return 1;
  }
    
}


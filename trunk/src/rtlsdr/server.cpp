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

#include <stdio.h>
#include <stdlib.h>
#include <getopt.h>
#include "client.h"
#include "listener.h"
#include "rtl-sdr.h"
#include "receiver.h"

struct RtlSdrConfig cfg;

int debug_level = 0;

void print_help()
{
    cerr << 
    "    Allowed options:"                                    << endl <<
    "      -s [ --samplerate ] samplerate in Samples/second"  << endl <<
    "                          250000 | 500000 | 1000000, def=250000" << endl <<
    "      -g [ --gain ]       gain in dB or -1 for AGC"      << endl <<
    "      -h [ --help ]       print usage message"           << endl <<
    "      -i [ --device]      device to be used (default=0)" << endl <<
    "      -d [ --debug ]      debug level (default=0)"       << endl <<
    endl;
}

int parseOptions (int argc, char **argv, RtlSdrConfig &cfg)
{
  int c;
  int rc = 0;

  while (1) {
      static struct option long_options[] =
        {
          /* These options set a flag. */
          {"help",       no_argument,       0,  1 },
          {"debug",      required_argument, 0, 'd'},
          {"samplerate", required_argument, 0, 's'},
          {"gain",       required_argument, 0, 'g'},
          {"device",     optional_argument, 0, 'i'},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "hd:s:g:i:",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c) {
        case 's':
          cfg.sr = atoi(optarg);
          break;

        case 'g':
          cfg.gain = atoi(optarg);
          break;

        case 'd':
          debug_level = atoi(optarg);
          break;

        case 'i':
          cfg.device_index = atoi(optarg);
          break;

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



int main(int argc, char* argv[]) {

    cfg.sr = 250000;
    cfg.gain = 450;
    cfg.device_index = 0;
    debug_level = 3;

    if (parseOptions (argc, argv, cfg) != 0) {
        return 255;
    }

    cout << "Debug level:          " << debug_level <<  std::endl;
    cout << "Sample rate:          " << cfg.sr      <<  std::endl;
    cout << "Gain:                 " << cfg.gain    <<  std::endl;
    cout << "Device index:         " << cfg.device_index <<  std::endl;

    // Init RTL SDR
    unsigned ndev = 0;
    if ( ( ndev = rtlsdr_get_device_count()) == 0 ) {
        cout << "No  RtlSDR hardware detected" << endl;
        return 255;
    } else {

        cout << ndev << " device(s) found:" <<  endl;

        for (unsigned i = 0; i < ndev; i++) cout << "  " << i << ":  " << rtlsdr_get_device_name(i) << endl;
	cout << endl ;

        if (cfg.device_index >= ndev) {
          cerr << "Bad device index !" << endl;
           return 255;
        }
	cout << "Using device " << cfg.device_index << ": " << rtlsdr_get_device_name(cfg.device_index) << endl;
		
        if (rtlsdr_open(&(cfg.rtl), cfg.device_index) < 0) {
            cerr << "Error opening device ! "<<  std::endl;
            return 255;
        } else {
            cout << "Device [" << rtlsdr_get_device_name(0) << "] successfully opened. " <<  cfg.rtl << endl;
        }

        int r;

        r = rtlsdr_set_sample_rate (cfg.rtl, cfg.sr);
        if (r < 0)
            fprintf(stderr, "WARNING: Failed to set sample rate.\n");

        int frequency = 145000000;

        r = rtlsdr_set_center_freq (cfg.rtl, frequency);
        if (r < 0)
            fprintf(stderr, "WARNING: Failed to set center freq.\n");
        else
            fprintf(stderr, "Tuned to %i Hz.\n", frequency);

if(cfg.gain<0) {
        r = rtlsdr_set_tuner_gain_mode(cfg.rtl, 0);
        if (r < 0)
           fprintf(stderr, "WARNING: Failed to set tuner gain mode.\n");
        else
           fprintf(stderr, "Tuner gain set in auto mode\n");

} else {
        r = rtlsdr_set_tuner_gain_mode(cfg.rtl, 1 /* manual */ );
        if (r < 0)
           fprintf(stderr, "WARNING: Failed to set tuner gain mode.\n");
        else
           fprintf(stderr, "Tuner gain set in manual mode\n");

        r = rtlsdr_set_tuner_gain(cfg.rtl, cfg.gain);
        if (r < 0)
           fprintf(stderr, "WARNING: Failed to set tuner gain.\n");
        else
           fprintf(stderr, "Tuner gain set to %.2f dB.\n", cfg.gain/10.0);

}


        cout << "Press q <ENTER> to exit." << endl;
        init_receivers (&cfg);
        create_listener_thread();

        char ch;
        while((ch = getc(stdin)) != EOF) {
            if (ch == 'q') break;
        }
        rtlsdr_close(cfg.rtl);
        return 0;
    }
}


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

//#include <iostream>
//#include <string>
//using namespace std;

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <getopt.h>
#include "client.h"
#include "listener.h"
#include "receiver.h"
#include "sdriq.h"

SDR_IQ_CONFIG cfg;

int debug_level = 0;


void print_help()
{
    puts(
    "    Allowed options:                                                      121\n" 
    "      -s [ --samplerate ] arg (=53333) samplerate in Samples/second       \n" 
    "                                       (53333  | 111111 | 133333 | 185185)\n"
    "      -u [ --usb ] arg (=/dev/ttyUSB0) serial device                      \n" 
    "      -h [ --help ]                    print usage message                \n" 
    "      -d [ --debug ] arg (=0)          debug level                        \n" 
    );
}

int parseOptions (int argc, char **argv, SDR_IQ_CONFIG *cfg)
{
  int c;
  int rc = 0;

  strcpy (cfg->start, "SSSSSSSSSSSSSSS");
  strcpy (cfg->stop,  "PPPPPPPPPPPPPPP");

  while (1) {
      static struct option long_options[] =
        {
          /* These options set a flag. */
          {"help",       no_argument,       0,  1 },
          {"usb",        required_argument, 0, 'u'},
          {"debug",      required_argument, 0, 'd'},
          {"samplerate", required_argument, 0, 's'},
          {0, 0, 0, 0}
        };
      /* getopt_long stores the option index here. */
      int option_index = 0;

      c = getopt_long (argc, argv, "hu:d:s:",
                       long_options, &option_index);

      /* Detect the end of the options. */
      if (c == -1)
        break;

      switch (c) {
        case 's':
          printf ("option -s with value `%s'\n", optarg);
          cfg->sr = atoi(optarg);
          break;

        case 'd':
          printf ("option -d with value `%s'\n", optarg);
          debug_level = atoi(optarg);
          break;

        case 'u':
          printf ("option -u with value `%s'\n", optarg);
          strcpy(cfg->usb, optarg);
          break;

        case 1:
        case '?':
        case 'h':
        default:
          /* getopt_long already printed an error message. */
          print_help();
          return -1;
          break;
        }
  }
  return rc;
}



int main(int argc, char* argv[]) 
{
    char buf[128];

    strcpy(cfg.usb, "/dev/ttyUSB0");
    cfg.sr = 53333;
    debug_level = 3;

    if (parseOptions (argc, argv, &cfg) != 0) {
        return 255;
    }

    fprintf ( stderr, "Debug level:          %d\n" ,debug_level );
    fprintf ( stderr, "Sample rate:          %d\n" ,cfg.sr      );
    fprintf ( stderr, "Serial device:        %s\n" ,cfg.usb     );

    // Init SDR-IQ
    if (open_samples(cfg.usb, "66666667.0", buf)  ) {
        puts( "No  HiqSDR hardware detected" );
        return 255;
    } else {
        fprintf ( stderr, "%s\n", buf );

        init_receivers (&cfg);
        create_listener_thread();
        fprintf (stderr, "Serial: %s\n", get_serial());

        puts( "Press q <ENTER> to exit." );
        char ch;
        while((ch = getc(stdin)) != EOF) {
            if (ch == 'q') break;
        }
        return 0;
    }
}


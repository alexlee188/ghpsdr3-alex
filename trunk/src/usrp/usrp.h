/**
* @file server.c
* @brief USRP intefrace handler
* @author Andrea Montefusco IW0HDV, Alberto Trentadue IZ0CEZ
* @author derived from the softrock implementation by John Melton, G0ORX/N6LYT
* @version 0.1
* @date 2009-10-13
*/

/* Copyright (C)
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

//Enables the OpenMP compilation
#ifndef _OPENMP
#define _OPENMP
#endif

#if ! defined __USPR_H__
#define       __USPR_H__

#include <sys/queue.h>


/*!
 * Initialises the USRP by means of UHD library.
 * Returns 'true' on success.
 */
bool usrp_init (const char *rx_subdev_par, const char *tx_subdev_par);

/*!
 * Starts the main USRP handling threads
 */
bool usrp_start (CLIENT *client);

/*!
 *Starts the RX sanmples forwarder thread
 */
int usrp_start_rx_forwarder_thread (CLIENT *client);

/*!
 * Causes the RX forwarder thread to exit
 */ 
void usrp_stop_rx_forwarder(void);

void usrp_deinit (void);

/*!
 * Subdevice configuration setter.
 * Accepts standard strings for subdevice configuration, for both RX (first) and TX (second).
 * The specs must be separated by a space: "RX_SPEC [TX_SPEC]"
 * TX_SPEC is optional
 * A SPEC is a ursp subdevice definition string like A:0 A.AB etc.
 * 
 * Please refer to UHD documentation for other examples.
 */
void usrp_set_subdev_args(const char *subdev_rx, const char *subdev_tx);

/*! 
 * Sets the swap iq option.
 */
void usrp_set_swap_iq(int);

/*!
 * Receiver max count setter
 */
void usrp_set_receivers(int);

/*!
 * Receiver max count getter
 */
int  usrp_get_receivers(void);

/*!
 * Tests the enabled TX function flag
 */
bool  usrp_is_tx_enabled(void);

/*!
 * Checks that USRP threads are started
 */
bool  usrp_is_started(void);


/*!
 * Client side sample rate setter.
 * This is the sample rate expected by this server's clients for IQ data.
 * 
 * The only allowed values:
 *  48 KSPS (48000)
 *  96 KSPS (96000)
 * 192 KSPS (192000)
 */
void usrp_set_client_rx_rate(int);

/*!
 * USRP clòient side sample rate getter.
 */
int  usrp_get_client_rx_rate(void);

/*! 
 * Sets the rf center frequency - in Hz
 */
void usrp_set_frequency(double freq);

/*!
 * Process the TX modulation to USRP
 */ 
int usrp_process_tx_modulation(float *outbuf, int mox);

/*!
 * Let the server discard rx samples
 */
void usrp_disable_rx_path(void);

#endif

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

#if ! defined __USPR_H__
#define       __USPR_H__

/*!
 * Initialises the USRP by means of UHD library.
 * Returns 'true' on success.
 */
bool usrp_init (const char *subdev_par);

/*!
 * Starts the main receiving thread
 */
bool usrp_start (RECEIVER *);

void usrp_deinit (void);

/*!
 * Subdevice configuration setter.
 * Accepts a standard string for subdevice configuration, for both RX (first) and TX (second)
 * The markup is: SUBDEVICE_SPEC ::= "<RX_SPEC> [<TX_SPEC>]"
 * TX_SPEC is optional
 * A SPEC is a ursp subdevice definition string like A:0 A.AB etc.
 * 
 * Please refer to UHD documentation for other examples.
 */
void usrp_set_subdev_args(const char *args);

/*! 
 * Sets the swap iq option.
 */
void usrp_set_swap_iq(int);

/*!
 * Receiver count setter
 */
void usrp_set_receivers(int);

/*!
 * Receiver count getter
 */
int  usrp_get_receivers(void);

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

#endif

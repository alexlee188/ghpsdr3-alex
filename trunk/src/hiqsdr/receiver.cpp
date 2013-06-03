/**
* @file receiver.c
* @brief manage client attachment to receivers
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

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <string.h>
#include <string>

#include "messages.h"
#include "client.h"
#include "receiver.h"
#include "hiqsdr.h"

#define SMALL_PACKETS

RECEIVER receiver[MAX_RECEIVERS];
static int iq_socket;
static struct sockaddr_in iq_addr;
static int iq_length;

static char response[80];

static unsigned long sequence=0L;

static int CORE_BANDWIDTH;

void init_receivers (HiqSdrConfig *pCfg) 
{
    int i;
    for(i=0;i<MAX_RECEIVERS;i++) {
        receiver[i].client  = (CLIENT*)NULL;
        receiver[i].samples = 0; 
    }

    iq_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(iq_socket<0) {
        perror("create socket failed for iq_socket\n");
        exit(1);
    }

    iq_length=sizeof(iq_addr);
    memset(&iq_addr,0,iq_length);
    iq_addr.sin_family=AF_INET;
    iq_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    iq_addr.sin_port=htons(11002);

    if(bind(iq_socket,(struct sockaddr*)&iq_addr,iq_length)<0) {
        perror("bind socket failed for iq socket");
        exit(1);
    }

    CORE_BANDWIDTH = pCfg->sr;

}

const char* attach_receiver(int rx, CLIENT* client) 
{
    if(client->state==RECEIVER_ATTACHED) {
        return CLIENT_ATTACHED;
    }

    //if(rx>=ozy_get_receivers()) {
    //    return RECEIVER_INVALID;
    //}

    hiqsdr_connect ();

    hiqsdr_set_frequency (7050000LL);
    hiqsdr_set_bandwidth (CORE_BANDWIDTH);


    if(receiver[rx].client!=(CLIENT *)NULL) {
        return RECEIVER_IN_USE;
    }
    
    client->state=RECEIVER_ATTACHED;
    receiver[rx].client=client;
    client->receiver=rx;

    receiver[rx].frame_counter = -1 ;
    receiver[rx].dc_average_i  = 0 ;
    receiver[rx].dc_average_q  = 0 ; 
    receiver[rx].dc_sum_i      = 0 ; 
    receiver[rx].dc_sum_q      = 0 ; 
    receiver[rx].dc_count      = 0 ; 
    receiver[rx].dc_key_delay  = 0 ; 



    //sprintf(response,"%s %d",OK,ozy_get_sample_rate());
    sprintf(response,"%s %d",OK,CORE_BANDWIDTH);

    return response;
}

const char* detach_receiver (int rx, CLIENT* client) {
    if(client->state==RECEIVER_DETACHED) {
        return CLIENT_DETACHED;
    }

    //if(rx>=ozy_get_receivers()) {
    //    return RECEIVER_INVALID;
    //}

    if(receiver[rx].client!=client) {
        return RECEIVER_NOT_OWNER;
    }
    printf("detach_receiver: ...");

    hiqsdr_stop_asynch_input ();
    printf(" done.\n");

    client->state=RECEIVER_DETACHED;
    receiver[rx].client = (CLIENT*)NULL;

    return OK;
}

const char* set_frequency (CLIENT* client, long frequency) {
    if(client->state==RECEIVER_DETACHED) {
        return CLIENT_DETACHED;
    }

    if(client->receiver<0) {
        return RECEIVER_INVALID;
    }

    receiver[client->receiver].frequency=frequency;
    receiver[client->receiver].frequency_changed=1;

    //fprintf (stderr, "%s: %ld\n", __FUNCTION__, receiver[client->receiver].frequency);

    hiqsdr_set_frequency (frequency);

    return OK;
}

const char* set_dither (CLIENT* client, bool dither)
{
//  receiver[client->receiver].ppc->dither = dither;
//
    return NOT_IMPLEMENTED_COMMAND;
    return OK;
}

const char* set_random (CLIENT* client, bool)
{
    return NOT_IMPLEMENTED_COMMAND;
    return OK;
}

const char* set_attenuator (CLIENT* client, int new_level_in_db)
{
    switch (new_level_in_db) {
    case 0:
    case 10:
    case 20:
    case 30:
    case 40:
        fprintf (stderr, "%s: new attenuator level: %d\n", __FUNCTION__, -(new_level_in_db));
        hiqsdr_set_attenuator (new_level_in_db);
        break;
    default:
        return INVALID_COMMAND;
    }
    return OK;
}

const char* select_antenna (CLIENT* client, int antenna)
{
    if (antenna == 0 || antenna == 1) {
        fprintf (stderr, "%s: new antenna: %d\n", __FUNCTION__, antenna);
        hiqsdr_set_antenna_input (antenna);
        return OK;
    } else
        return INVALID_COMMAND;
}


const char* select_preselector (CLIENT* client, int preselector)
{
    if (preselector >= 0 && preselector <= 15) {
        fprintf (stderr, "%s: new preselector: %d\n", __FUNCTION__, preselector);
        hiqsdr_set_preselector (preselector);
        return OK;
    } else
        return INVALID_COMMAND;
}

const char* set_preamplifier(CLIENT* client, int preamp)
{
    if (preamp >= 0 && preamp <= 1) {
        fprintf (stderr, "%s: new prepreamp: %d\n", __FUNCTION__, preamp);
        hiqsdr_set_preamp (preamp);
        return OK;
    } else
        return INVALID_COMMAND;
}

void send_IQ_buffer (RECEIVER *pRec) {
    struct sockaddr_in client;
    int client_length;
    unsigned short offset;
    BUFFER buffer;
    int rc;

    if(pRec->client != (CLIENT*)NULL) {
        if(pRec->client->iq_port != -1) {
            // send the IQ buffer

            client_length = sizeof(client);
            memset((char*)&client,0,client_length);
            client.sin_family = AF_INET;
            client.sin_addr.s_addr = pRec->client->address.sin_addr.s_addr;
            client.sin_port = htons(pRec->client->iq_port);

#ifdef SMALL_PACKETS
            // keep UDP packets to 512 bytes or less
            //     8 bytes sequency number
            //     2 byte offset
            //     2 byte length
            offset=0;
            while(offset<sizeof(pRec->input_buffer)) {
                buffer.sequence=sequence;
                buffer.offset=offset;
                buffer.length=sizeof(pRec->input_buffer)-offset;
                if(buffer.length>500) buffer.length=500;
                memcpy ((char*)&buffer.data[0], (char*)&(pRec->input_buffer[offset/4]), buffer.length);
                rc = sendto (iq_socket, (char*)&buffer, sizeof(buffer), 0, (struct sockaddr*)&client,client_length);
                if(rc<=0) {
                    perror("sendto failed for iq data");
                    exit(1);
                } 
                //else {
                //    fprintf (stderr, "%s: sending packet to %s.\n", __FUNCTION__, inet_ntoa(client.sin_addr));
                //}
                offset+=buffer.length;
            }
            sequence++;

            
#else
            rc=sendto(iq_socket,pRec->input_buffer,sizeof(pRec->input_buffer),0,(struct sockaddr*)&client,client_length);
            if(rc<=0) {
                perror("sendto failed for iq data");
                exit(1);
            }
#endif
 
        }
    }
}


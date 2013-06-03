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

#include "messages.h"
#include "client.h"
#include "receiver.h"
#include "sdriq.h"

#define SMALL_PACKETS

RECEIVER receiver[MAX_RECEIVERS];
static int iq_socket;
static struct sockaddr_in iq_addr;
static int iq_length;

static char response[80];

static unsigned long sequence=0L;

static int CORE_BANDWIDTH;

void *send_IQ_buffer_from_queue (void *pArg) ;

void init_receivers (SDR_IQ_CONFIG *pCfg) 
{
    int i;
    for(i=0;i<MAX_RECEIVERS;i++) {
        receiver[i].client  = (CLIENT*)NULL;
        receiver[i].samples = 0; 
        receiver[i].m_NcoSpurCalActive = true;
        receiver[i].m_NcoSpurCalCount  = 0; 
        receiver[i].m_NCOSpurOffsetI   = 0.0;
        receiver[i].m_NCOSpurOffsetQ   = 0.0;
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
    receiver[0].cfg = *pCfg; 

}

const char* attach_receiver(int rx, CLIENT* client) 
{
    if(client->state==RECEIVER_ATTACHED) {
        return CLIENT_ATTACHED;
    }

    //if(rx>=ozy_get_receivers()) {
    //    return RECEIVER_INVALID;
    //}

    gain_sdriq (0, -20);
    //gain_sdriq (1, 1);

    freq_sdriq (7050000);
    set_bandwidth (CORE_BANDWIDTH);


    //hiqsdr_connect ();
    //hiqsdr_set_frequency (7050000LL);
    //hiqsdr_set_bandwidth (CORE_BANDWIDTH);


    if(receiver[rx].client!=(CLIENT *)NULL) {
        return RECEIVER_IN_USE;
    }
    
    client->state=RECEIVER_ATTACHED;
    receiver[rx].client=client;
    client->receiver=rx;

    receiver[rx].frame_counter = -1 ;

    ManageNCOSpurOffsets( &(receiver[rx]), NCOSPUR_CMD_STARTCAL, 0,0);

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

    sdriq_stop_asynch_input ();
    //close_samples ();

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

    //hiqsdr_set_frequency (frequency);
    freq_sdriq (frequency);


    return OK;
}


const char* set_preamp (CLIENT* client, bool preamp)
{
//  receiver[client->receiver].ppc->preamp = preamp;
//
    return NOT_IMPLEMENTED_COMMAND;
    return OK;
}

const char* set_dither (CLIENT* client, bool dither)
{
//  receiver[client->receiver].ppc->dither = dither;
//
    return NOT_IMPLEMENTED_COMMAND;
    return OK;
}

const char* set_random (CLIENT* client, bool fRand)
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
        gain_sdriq (0, -(new_level_in_db));
        break;
    default:
        return INVALID_COMMAND;
    }
    return OK;
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



/*
 * Called to read/set/start calibration of the NCO Spur Offset value.
 * Excerpted from CuteSDR Copyright (c) 2010 Moe Wheatley
 */
void ManageNCOSpurOffsets( RECEIVER *pRec, eNCOSPURCMD cmd, double* pNCONullValueI,  double* pNCONullValueQ)
{
	pRec->m_NcoSpurCalActive = false;
	switch(cmd)
	{
		case NCOSPUR_CMD_SET:
			if( (NULL!=pNCONullValueI) && (NULL!=pNCONullValueQ) )
			{
				pRec->m_NCOSpurOffsetI = *pNCONullValueI;
				pRec->m_NCOSpurOffsetQ = *pNCONullValueQ;
//qDebug()<<"Cal Set"<< m_NCOSpurOffsetI << m_NCOSpurOffsetQ;
			}
			break;
		case NCOSPUR_CMD_STARTCAL:
			if((pRec->m_NCOSpurOffsetI>10.0) || (pRec->m_NCOSpurOffsetI<-10.0))
				pRec->m_NCOSpurOffsetI = 0.0;
			if((pRec->m_NCOSpurOffsetQ>10.0) || (pRec->m_NCOSpurOffsetQ<-10.0))
				pRec->m_NCOSpurOffsetQ = 0.0;
			pRec->m_NcoSpurCalCount = 0;
			pRec->m_NcoSpurCalActive = true;
//qDebug()<<"Start NCO Cal";
			break;
		case NCOSPUR_CMD_READ:
			if( (NULL!=pNCONullValueI) && (NULL!=pNCONullValueQ) )
			{
				*pNCONullValueI = pRec->m_NCOSpurOffsetI;
				*pNCONullValueQ = pRec->m_NCOSpurOffsetQ;
//qDebug()<<"Cal Read"<< m_NCOSpurOffsetI << m_NCOSpurOffsetQ;
			}
			break;
		default:
			break;
	}
}

/*
 * Called to calculate the NCO Spur Offset value from the incoming m_DataBuf.
 * Excerpted from CuteSDR (C) Copyright (c) 2010 Moe Wheatley
 */
void NcoSpurCalibrate (RECEIVER *pRec /* double* pData, qint32 length */ )
{
        if( pRec->m_NcoSpurCalCount < SPUR_CAL_MAXSAMPLES) {
            int j;

            for( j=0 ; j < pRec->samples; j++) {	//calculate average of I and Q data to get individual DC offsets
               float q = pRec->input_buffer[j]             ;
               float i = pRec->input_buffer[j+BUFFER_SIZE] ;

               pRec->m_NCOSpurOffsetQ = (1.0-1.0/100000.0)*pRec->m_NCOSpurOffsetQ + (1.0/100000.0)*q;
               pRec->m_NCOSpurOffsetI = (1.0-1.0/100000.0)*pRec->m_NCOSpurOffsetI + (1.0/100000.0)*i;
            }
            pRec->m_NcoSpurCalCount += (pRec->samples/4);

	} else {
            pRec->m_NcoSpurCalActive = false;
            fprintf (stderr, "NCO Cal Done");
	}
}

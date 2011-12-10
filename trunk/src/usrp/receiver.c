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

#ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#endif 
#include <string.h>

#include "buffer.h"
#include "client.h"
#include "receiver.h"
#include "messages.h"
#include "usrp.h"
#include "usrp_audio.h"
#include "util.h"

#define SMALL_PACKETS

RECEIVER receiver[MAX_RECEIVERS];
static int iq_socket;
static struct sockaddr_in iq_addr;
static int iq_length;

static char response[210];

static unsigned long sequence=0L;

void init_receivers(void) {
    int i;
    for(i=0;i<MAX_RECEIVERS;i++) {
		receiver[i].id=i;
        receiver[i].client=(CLIENT*)NULL;
		receiver[i].frequency=1000000;
    }

    iq_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(iq_socket<0) {
        perror("Receiver: create socket failed for iq_socket\n");
        exit(1);
    }

    iq_length=sizeof(iq_addr);
    memset(&iq_addr,0,iq_length);
    iq_addr.sin_family=AF_INET;
    iq_addr.sin_addr.s_addr=htonl(INADDR_ANY);
    iq_addr.sin_port=htons(11002);  //Server side socket

    if(bind(iq_socket,(struct sockaddr*)&iq_addr,iq_length)<0) {
        perror("Receiver: bind socket failed for iq socket");
        exit(1);
    }

}

const char* attach_receiver(int rx,CLIENT* client) {

    if(client->receiver_state==RECEIVER_ATTACHED) {
        return CLIENT_ATTACHED;
    }

    //fprintf(stderr,"attach_receiver: rx=%d %s:%d\n",rx,inet_ntoa(client->address.sin_addr),ntohs(client->address.sin_port));

    if(rx>=usrp_get_receivers()) {
        return RECEIVER_INVALID;
    }

    if(receiver[rx].client!=(CLIENT *)NULL) {
        return RECEIVER_IN_USE;
    }
    
    //Client data update
    client->receiver_state=RECEIVER_ATTACHED;    
    client->receiver=rx;
    
    //receiver data update
    receiver[rx].client=client;
    
    // attempt to open an audio stream on a local audio card
	if (usrp_get_server_audio() == 1) {
		usrp_audio_open (usrp_get_client_rx_rate());
		fprintf(stderr, "Server side audio open, passed USRP sample rate %d\n", usrp_get_client_rx_rate());    		
	} else {
		fprintf(stderr, "Server side audio disabled\n", usrp_get_client_rx_rate());    		
	}

    sprintf(response,"%s %d",OK, usrp_get_client_rx_rate());
    return (response);
}

const char* detach_receiver(int rx,CLIENT* client) {
    if(client->receiver_state==RECEIVER_DETACHED) {
        return CLIENT_DETACHED;
    }

    if(rx>=usrp_get_receivers()) {
        return RECEIVER_INVALID;
    }

    if(receiver[rx].client!=client) {
        return RECEIVER_NOT_OWNER;
    }

    client->receiver_state=RECEIVER_DETACHED;
    receiver[rx].client=(CLIENT*)NULL;

    return OK;
}

const char* set_frequency(CLIENT* client,long frequency) {
    if(client->receiver_state==RECEIVER_DETACHED) {
        return CLIENT_DETACHED;
    }

    if(client->receiver<0) {
        return RECEIVER_INVALID;
    }

    receiver[client->receiver].frequency=frequency;
    //fprintf(stderr, "Frequency to be set in USRP=%ld", receiver[client->receiver].frequency);
    usrp_set_frequency(frequency);
    receiver[client->receiver].frequency_changed=1;

    return OK;
}

void send_IQ_buffer(RECEIVER *pRec) {
    struct sockaddr_in client;
    int client_length;
    unsigned short offset;
    //unsigned short length; // unused
    BUFFER buffer;
    int rc;

    if(pRec->client!=(CLIENT*)NULL) {
        if(pRec->client->iq_port!=-1) {
            // send the IQ buffer
            //fprintf(stderr,"Client iq port: %d\n", pRec->client->iq_port);
            client_length=sizeof(client);
            memset((char*)&client,0,client_length);
            client.sin_family=AF_INET;
            client.sin_addr.s_addr=pRec->client->address.sin_addr.s_addr;
            client.sin_port=htons(pRec->client->iq_port);            

#ifdef SMALL_PACKETS
            // keep UDP packets to 512 bytes or less:
            //   500 iq data
            //     8 bytes sequency number
            //     2 byte offset
            //     2 byte length
            offset=0;
            while(offset<sizeof(pRec->input_buffer)) {
                buffer.sequence=sequence;
#ifndef __linux__
                buffer.sequenceHi = 0L;
#endif
                buffer.offset=offset;
                buffer.length=sizeof(pRec->input_buffer)-offset;
                if(buffer.length>500) buffer.length=500;
                memcpy((char*)&buffer.data[0],(char*)&pRec->input_buffer[offset/4],buffer.length);
                //fprintf(stderr,"send_IQ: rx:%d: %lld:%d:%d\n",pRec->id,buffer.sequence,buffer.offset,buffer.length);
                //fprintf(stderr,"Data start: %d %d %d\n",buffer.data[0], buffer.data[1], buffer.data[2]);
                rc=sendto(iq_socket,(char*)&buffer,sizeof(buffer),0,(struct sockaddr*)&client,client_length);
                if(rc<=0) {
                    perror("Receiver: sendto failed for iq data");
                    exit(1);
                }
                offset+=buffer.length;
            }
            sequence++;

#else
            rc=sendto(iq_socket,receiver[rx].input_buffer,sizeof(receiver[rx].input_buffer),0,(struct sockaddr*)&client,client_length);
            if(rc<=0) {
                perror("Receiver: sendto failed for iq data");
                exit(1);
            }
#endif
        //fprintf(stderr, "RC=%d\n", rc);
        }
    }
}

//Debugging function
const char* inspect_receiver(RECEIVER* rx) {
	
    char client_data[60];
	
    sprintf(response,"Receiver Instance:\nid:%d\naudio_socket:%d\nfrequency:%ld\n",rx->id,rx->audio_socket,rx->frequency);
    if (rx->client!=NULL) {
        sprintf(client_data,"client address:%s\nclient port:%d\n", inet_ntoa(rx->client->address.sin_addr),ntohs(rx->client->address.sin_port));
    } else sprintf(client_data,"NO CLIENT\n");
	strcat(response, client_data);
        
    return response;
}
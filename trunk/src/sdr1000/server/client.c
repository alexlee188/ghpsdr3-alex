/**
* @file client.c
* @brief Handle client connection
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

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <string.h>

#include "client.h"
#include "receiver.h"
#include "messages.h"

short audio_port=AUDIO_PORT;

char* parse_command(CLIENT* client,char* command);
void* audio_thread(void* arg);

void* client_thread(void* arg) 
{
    CLIENT* client=(CLIENT*)arg;
    char command[80];
    int bytes_read;
    char* response;

fprintf(stderr,"client connected: %s:%d\n",inet_ntoa(client->address.sin_addr),ntohs(client->address.sin_port));

    client->state=RECEIVER_DETACHED;

    while(1) 
    {
        bytes_read=recv(client->socket,command,sizeof(command),0);
        if(bytes_read<=0) {
            break;
        }
        command[bytes_read]=0;
        response=parse_command(client,command);
        send(client->socket,response,strlen(response),0);

        fprintf(stderr,"response: '%s'\n",response);
    }

    if(client->state==RECEIVER_ATTACHED) 
    {
        receiver[client->receiver].client=(CLIENT*)NULL;
        client->state=RECEIVER_DETACHED;
    }

    close(client->socket);

    fprintf(stderr,"client disconnected: %s:%d\n",inet_ntoa(client->address.sin_addr),ntohs(client->address.sin_port));

    free(client);
    return 0;
} // end client_thread


char* parse_command(CLIENT* client,char* command) 
{
    char* token;

    fprintf(stderr,"parse_command: '%s'\n",command);

    token=strtok(command," \r\n");
    if(token!=NULL) 
    {
        if(strcmp(token,"mox")==0) 
        {
            // PTT on/off
            token=strtok(NULL," \r\n");
            if(token!=NULL) 
            {
                int ptt=atoi(token);
                return set_ptt(client, ptt);
            } 
            else 
            {
                return INVALID_COMMAND;
            }
        } 
        else if(strcmp(token,"attach")==0) 
        {
            // select receiver
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
                int rx=atoi(token);
                return attach_receiver(rx,client);
            } 
            else 
            {
                return INVALID_COMMAND;
            }
        } 
        else if(strcmp(token,"detach")==0) 
        {
            // select receiver
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
                int rx=atoi(token);
                return detach_receiver(rx,client);
            } else {
                return INVALID_COMMAND;
            }
        } 
        else if(strcmp(token,"frequency")==0) 
        {
            // set frequency
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
               long f=atol(token);
               return set_frequency(client,f);
            } else {
                return INVALID_COMMAND;
            }
        } 
        else if(strcmp(token,"freqcaloffset")==0) 
        {
            // set frequency cal offset
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
               long f=atol(token);
               return set_frequency_offset(client,f);
            } else {
                return INVALID_COMMAND;
            }
        } 
        else if(strcmp(token,"start")==0) 
        {
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
                if(strcmp(token,"iq")==0) {
                    token=strtok(NULL," \r\n");
                    if(token!=NULL) {
                        client->iq_port=atoi(token);
                    }
                    if(pthread_create(&receiver[client->receiver].audio_thread_id,NULL,audio_thread,&receiver[client->receiver])!=0) {
                        fprintf(stderr,"failed to create audio thread for rx %d\n",client->receiver);
                        exit(1);
                    }
                    return OK;
                } else if(strcmp(token,"bandscope")==0) {
                    token=strtok(NULL," \r\n");
                    if(token!=NULL) {
                        client->bs_port=atoi(token);
                    }
                    return OK;
                } else {
                    // invalid command string
                    return INVALID_COMMAND;
                }
            } else {
                // invalid command string
                return INVALID_COMMAND;
            }
        } 
        else if(strcmp(token,"stop")==0) 
        {
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
                if(strcmp(token,"iq")==0) {
                    token=strtok(NULL," \r\n");
                    if(token!=NULL) {
                        client->iq_port=-1;
                    }
                    return OK;
                } else if(strcmp(token,"bandscope")==0) {
                    client->bs_port=-1;
                } else {
                    // invalid command string
                    return INVALID_COMMAND;
                }
            } else {
                // invalid command string
                return INVALID_COMMAND;
            }
        } else if(strcmp(token,"hardware?")==0) {
            fprintf (stderr, "*****************************\n");
            return "OK SDR1000";
        } else if(strcmp(token,"setattenuator")==0) {
            // set attenuator
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
               long av=atol(token);
               return set_attenuator(client,av);
            } else {
                return INVALID_COMMAND;
            }
        } else if(strcmp(token,"setspurreduction")==0) {
            // set spur reduction
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
               unsigned char enabled=(unsigned char)atoi(token);
               return set_spurreduction(client,enabled);
            } else {
                return INVALID_COMMAND;
            }
        } else if(strcmp(token,"record")==0) {
            // set record
            unsigned char enabled;
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
               if (strcmp(token, "on") == 0)
                   enabled = 1;
               else
                   enabled = 0;
               return set_record(client,enabled);
            } else {
                return INVALID_COMMAND;
            }
        } else if(strcmp(token,"getpaadc?")==0) {
            // get PA ADC value
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
               unsigned char channel=(unsigned char)atoi(token);
               return get_pa_adc(client,channel);
            } else {
                return INVALID_COMMAND;
            }
        } else {
            // invalid command string
            return INVALID_COMMAND;
        }
    }
    // empty command string
    return INVALID_COMMAND;
} // end parse_command


void* audio_thread(void* arg) 
{
    RECEIVER *rx=(RECEIVER*)arg;
    struct sockaddr_in audio;
    int audio_length;
    int old_state, old_type;
    int bytes_read;
    int on=1;

    fprintf(stderr,"audio_thread port=%d\n",audio_port+(rx->id*2));

    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&old_state);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&old_type);

    rx->audio_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(rx->audio_socket<0) 
    {
        perror("create socket failed for server audio socket");
        exit(1);
    }

    setsockopt(rx->audio_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

    audio_length=sizeof(audio);
    memset(&audio,0,audio_length);
    audio.sin_family=AF_INET;
    audio.sin_addr.s_addr=htonl(INADDR_ANY);
    audio.sin_port=htons(audio_port+(rx->id*2));

    if(bind(rx->audio_socket,(struct sockaddr*)&audio,audio_length)<0) 
    {
        perror("bind socket failed for server audio socket");
        exit(1);
    }

    fprintf(stderr,"listening for rx %d audio on port %d\n",rx->id,audio_port+(rx->id*2));

    while(1) 
    {
        // get audio from a client
        bytes_read=recvfrom(rx->audio_socket,rx->output_buffer,sizeof(rx->output_buffer),0,(struct sockaddr*)&audio,(socklen_t*)&audio_length);
        if(bytes_read<0) 
        {
            perror("recvfrom socket failed for audio buffer");
            exit(1);
        }
//fprintf(stderr, "Read: %d\n", bytes_read);
        process_sdr1000_output_buffer(rx->output_buffer,&rx->output_buffer[BUFFER_SIZE]);
    }
} // end audio_thread


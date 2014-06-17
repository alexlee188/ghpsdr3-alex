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
#include <sys/types.h>
#include <string.h>

  #ifdef __linux__
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <pthread.h>
#include <unistd.h>
  #else
#include <winsock.h>
#include "pthread.h"
  #endif

#include "client.h"
#include "receiver.h"
#include "messages.h"
#include "bandscope.h"
#include "usrp.h"
#include "transmitter.h"

const char* parse_command(CLIENT* client,char* command);

void set_mox(CLIENT* client, int mox) {
    
    pthread_mutex_lock(&client->mox_lock);
    client->mox = mox;
    pthread_mutex_unlock(&client->mox_lock);    
}

int get_mox(CLIENT* client) {
    
    pthread_mutex_lock(&client->mox_lock);
    int rv = client->mox;
    pthread_mutex_unlock(&client->mox_lock);    
    return rv;
}


int toggle_mox(CLIENT* client) {
    
    pthread_mutex_lock(&client->mox_lock);
    int rv = (client->mox = 1 - client->mox);
    pthread_mutex_unlock(&client->mox_lock);
    return rv;
}

//Client thread for RX IQ stream
void* client_thread(void* arg) {
    CLIENT* client=(CLIENT*)arg;
    int old_state, old_type;
    char command[80];
    int bytes_read;
    const char* response;

    client->receiver_state=RECEIVER_DETACHED;
    client->transmitter_state=TRANSMITTER_DETACHED;
    client->receiver_num=-1;    
    pthread_mutex_init(&client->mox_lock, NULL );
    set_mox(client,0);
    
    pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&old_state);
    pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&old_type);    

    while(1) {	
        bytes_read=recv(client->socket,command,sizeof(command),0);
        if(bytes_read<=0) {
            break;
        }
        command[bytes_read]=0;
        response=parse_command(client,command);
        send(client->socket,response,strlen(response),0);

        fprintf(stderr,"Response to DSP client (Rx%d): '%s'\n",client->receiver_num,response);		
    }
	fprintf(stderr,"Exiting DSP client thread loop (Rx%d)...\n",client->receiver_num);

    //GRACEFUL EXIT ON DISCONNECTION
    set_mox(client,0);
    
    if(client->transmitter_state==TRANSMITTER_ATTACHED) {
        stop_tx_audio(client); //Exits the tx_audio_thread @ transmitter
        client->transmitter_state=TRANSMITTER_DETACHED;
    }            
    sleep(1);
        
    if(client->receiver_state==RECEIVER_ATTACHED) {
        detach_receiver(client->receiver_num, client); 
        receiver[client->receiver_num].client=(CLIENT*)NULL;
        client->receiver_state=RECEIVER_DETACHED;
    }

    client->bs_port=-1;
    detach_bandscope(client);

#ifdef __linux__
    close(client->socket);
#else
    closesocket(client->socket);
#endif

    fprintf(stderr,"Client Handler: Client disconnected: %s:%d\n",inet_ntoa(client->address.sin_addr),ntohs(client->address.sin_port));    

    free(client);
    return 0;
}

//Creates a client therad for a client
int create_client_thread(CLIENT *client) {
 
    int rc=pthread_create(&client->thread_id,NULL,client_thread,(void *)client);
    if(rc!=0) fprintf(stderr, "Listener: pthread_create client_thread failed");        
    return rc;
}

const char* parse_command(CLIENT* client,char* command) {
    
    char* token;

    fprintf(stderr,"parse_command(Rx%d): '%s'\n",client->receiver_num,command);

    token=strtok(command," \r\n");
    if(token!=NULL) {
        if(strcmp(token,"attach")==0) {
			//COMMAND: 'attach <side>'            
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
                //COMMAND: 'attach rx#'
                int rx=atoi(token);
                const char *resp=attach_receiver(rx,client);
                if (strcmp(resp, "Error") == 0) 
                    return resp;
                else 
                    return attach_transmitter(client, resp);                    
            }
        } else if(strcmp(token,"detach")==0) {
            //COMMAND: 'detach <side>' 
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
                int rx=atoi(token);
                return detach_receiver(rx,client);
            } else {
                return INVALID_COMMAND;
            }
        } else if(strcmp(token,"frequency")==0) {
            //COMMAND: 'frequency <long freq>'
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
               long f=atol(token);
               return set_frequency(client,f);
            } else {
                return INVALID_COMMAND;
            }
        } else if(strcmp(token,"start")==0) {
			//COMMAND: 'start <stream> <port#>'
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
                if(strcmp(token,"iq")==0) {
					//COMMAND: 'start iq <port#>'
                    token=strtok(NULL," \r\n");
                    if(token!=NULL) {
                        client->iq_port=atoi(token);
                    }                    
                    //NOTE:as it is now the usrp-server, only receiver 0's is supported
                    //then it is the only one to issue 'start' command.
                    if (usrp_start_rx_forwarder_thread(client)!=0) exit(1);
                    sleep(1); //some settling time...
                    
                    //This is executed only once
                    if (! usrp_is_started()) {
                        if (usrp_start (client))
                            fprintf(stderr,"Started USRP for Client %d\n",client->receiver_num);
                        else {
                            fprintf(stderr,"USRP threads start FAILED for rx %d\n",client->receiver_num);
                            exit(1);
                        }
                    }
                    sleep(1); //some settling time
                    //Start the server side tx audio stream receiving thread
                    //Exit program if failed (...?)
                    if (start_tx_audio_thread(client)!=0) exit(1);
                    
                    return OK;
                } else if(strcmp(token,"bandscope")==0) {
					//COMMAND: 'start bandscope <port#>'
                    token=strtok(NULL," \r\n");
                    if(token!=NULL) {
                        client->bs_port=atoi(token);
                    }
                    attach_bandscope(client);
                    return OK;
                } else {
                    // invalid command string
                    return INVALID_COMMAND;
                }
            } else {
                // invalid command string
                return INVALID_COMMAND;
            }
        } else if(strcmp(token,"stop")==0) {
			//COMMAND: 'stop <stream>'
            token=strtok(NULL," \r\n");
            if(token!=NULL) {
                if(strcmp(token,"iq")==0) {
					//COMMAND: 'stop iq'
                    token=strtok(NULL," \r\n");
                    if(token!=NULL) {
                        client->iq_port=-1;
                    }
                    return OK;
                } else if(strcmp(token,"bandscope")==0) {
					//COMMAND: 'stop bandscope'
                    client->bs_port=-1;
                    detach_bandscope(client);
                } else {
                    // invalid command string
                    return INVALID_COMMAND;
                }
            } else {
                // invalid command string
                return INVALID_COMMAND;
            }
        } else if(strcmp(token,"mox")==0) {
            //Toggle the mox
            int v=toggle_mox(client);
            fprintf(stderr,"Toggled mox to %d for Client %d\n",v, client->receiver_num);            
            return OK;
            
        } else if(strcmp(token,"preamp")==0) {
            return NOT_IMPLEMENTED_COMMAND;
        } else if(strcmp(token,"record")==0) {
            return NOT_IMPLEMENTED_COMMAND;
        } else if(strcmp(token,"ocoutput")==0) {
            return NOT_IMPLEMENTED_COMMAND;
        } else {
            // invalid command string
            return INVALID_COMMAND;
        }
    }
    // empty command string
    return INVALID_COMMAND;
}

    



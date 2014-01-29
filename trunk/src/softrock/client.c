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
* This client stuff is what happens when a client is connected (see listener.c).
* The client_thread is created in listener.c when a client connects.  The client
* is a dspserver or perhaps in the future ghpsdr or monitor, but it is not 
* directly Qtradio (only through dspserver).
* See for example:
* http://openhpsdr.org/wiki/index.php?title=Ghpsdr3
* 
*/

#define SMALL_PACKETS

#include "client.h"
#include "receiver.h"
#include "messages.h"
#include "softrock.h"
#include "buffer.h"

short audio_port=AUDIO_PORT;

char* parse_command(CLIENT* client,char* command);
void* audio_thread(void* arg);

void* client_thread(void* arg) {
    CLIENT* client=(CLIENT*)arg;
    char command[80];
    int bytes_read;
    char* response;

    fprintf(stderr,"client connected: %s:%d\n",inet_ntoa(client->address.sin_addr),ntohs(client->address.sin_port));
	
    client->state=RECEIVER_DETACHED;

    while(1) {
        bytes_read=recv(client->socket,command,sizeof(command),0);
        if(bytes_read<=0) {
            break;
        }
        command[bytes_read]=0;
        response=parse_command(client,command);
        send(client->socket,response,strlen(response),0);
		fprintf(stderr,"response: '%s'\n",response);
    }

    if(client->state==RECEIVER_ATTACHED) {
        receiver[client->receiver].client=(CLIENT*)NULL;
        client->state=RECEIVER_DETACHED;
    }

    close(client->socket);
    fprintf(stderr,"client disconnected: %s:%d\n",inet_ntoa(client->address.sin_addr),ntohs(client->address.sin_port));
	
    free(client);
    return 0;
}

char* parse_command(CLIENT* client,char* command) {
	// Commands from the client are parsed here to 
	// produce a response to be sent back to the 
	// client.
	char* token;

	fprintf(stderr,"parse_command: '%s'\n",command);

	token=strtok(command," \r\n");
	if(token!=NULL) {
		if(strcmp(token,"attach")==0) {
			// select receiver
			token=strtok(NULL," \r\n");
			if(token!=NULL) {
				int rx=atoi(token);
				return attach_receiver(rx,client);
			} else {
				return INVALID_COMMAND;
			}
		} else if(strcmp(token,"detach")==0) {
			// select receiver
			token=strtok(NULL," \r\n");
			if(token!=NULL) {
				int rx=atoi(token);
				return detach_receiver(rx,client);
			} else {
				return INVALID_COMMAND;
			}
		} else if(strcmp(token,"frequency")==0) {
			// set frequency
			token=strtok(NULL," \r\n");
			if(token!=NULL) {
				long f=atol(token);
				return set_frequency(client,f);
			} else {
				return INVALID_COMMAND;
			}
			/*****************************************************************************/
		} else if(strcmp(token,"mox")==0) {
			// set ptt
			token=strtok(NULL," \r\n");
			if(token!=NULL) {
				int p=atoi(token);
				return set_ptt(client,p);
			} else {
				return INVALID_COMMAND;
			}
			/*****************************************************************************/
		} else if(strcmp(token,"start")==0) {
			token=strtok(NULL," \r\n");
			if(token!=NULL) {
				if(strcmp(token,"iq")==0) {
					token=strtok(NULL," \r\n");
					if(token!=NULL) {
						client->iq_port=atoi(token);
					}
				}
				if(pthread_create(&receiver[client->receiver].audio_thread_id,NULL,audio_thread,&receiver[client->receiver])!=0) {
					if (softrock_get_verbose()) fprintf(stderr,"failed to create audio thread for tx %d\n",client->receiver);
					exit(1);
				}
				return OK;
			 if(strcmp(token,"bandscope")==0) {
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
		} else if(strcmp(token,"stop")==0) {
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
		} else {
			// invalid command string
			return INVALID_COMMAND;
		}
	}
	// empty command string
	return INVALID_COMMAND;
}



void* audio_thread(void* arg) {
	RECEIVER *rx=(RECEIVER*)arg;
	struct sockaddr_in audio;
	int audio_length=sizeof(audio);
	int old_state, old_type;
	int bytes_read;
	int on=1;

#ifdef USE_PIPES
	int  pipe_left = *softrock_get_jack_write_pipe_left(rx->client->receiver);
	int  pipe_right = *softrock_get_jack_write_pipe_right(rx->client->receiver);
#else // Use ringbuffers
	jack_ringbuffer_t *rb_left = softrock_get_jack_rb_left(rx->client->receiver);
	jack_ringbuffer_t *rb_right = softrock_get_jack_rb_right(rx->client->receiver);
#endif
	int num_bytes = sizeof(float)*BUFFER_SIZE;
	
	static int second_time = 0;

	BUFFER buffer;
	unsigned long sequence=0L;
	unsigned short offset=0;;


	if (softrock_get_verbose()) fprintf(stderr,"audio_thread port=%d\n",audio_port+(rx->id*2));

	pthread_setcancelstate(PTHREAD_CANCEL_ENABLE,&old_state);
	pthread_setcanceltype(PTHREAD_CANCEL_ASYNCHRONOUS,&old_type);

	rx->audio_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
	if(rx->audio_socket<0) {
		perror("create socket failed for server audio socket");
		exit(1);
	}

	setsockopt(rx->audio_socket, SOL_SOCKET, SO_REUSEADDR, &on, sizeof(on));

	memset(&audio,0,audio_length);
	audio.sin_family=AF_INET;
	audio.sin_addr.s_addr=htonl(INADDR_ANY);
	audio.sin_port=htons(audio_port+(rx->id*2));

	if(bind(rx->audio_socket,(struct sockaddr*)&audio,audio_length)<0) {
		perror("bind socket failed for server audio socket");
		exit(1);
	}

	if (softrock_get_verbose()) fprintf(stderr,"listening for tx %d IQ audio on port %d\n",rx->id,audio_port+(rx->id*2));

	while(1) {
		// get audio from a client
#ifdef SMALL_PACKETS  
		while(1) {
			bytes_read=recvfrom(rx->audio_socket,(char*)&buffer,sizeof(buffer),0,(struct sockaddr*)&audio,(socklen_t *)&audio_length);
			if(bytes_read<0) {
				perror("recvfrom socket failed for tx iq buffer");
				exit(1);
			}

			//fprintf(stderr,"rcvd UDP packet: sequence=%lld offset=%d length=%d\n", buffer.sequence, buffer.offset, buffer.length);

			if(buffer.offset==0) {
				offset=0;
				sequence=buffer.sequence;
				// start of a frame
				memcpy((char *)&rx->output_buffer[buffer.offset/4],(char *)&buffer.data[0],buffer.length);
				offset+=buffer.length;
			} else {
				if((sequence==buffer.sequence) && (offset==buffer.offset)) {
					memcpy((char *)&rx->output_buffer[buffer.offset/4],(char *)&buffer.data[0],buffer.length);
					offset+=buffer.length;
					if(offset==sizeof(rx->output_buffer)) {
						offset=0;
						break;
					}
				} else {
					fprintf(stderr,"missing TX IQ frames expected %ld.%d got %ld.%d\n", (long)sequence,(int)offset,(long)buffer.sequence,(int)buffer.offset);
				}
			}
		}
#else
		bytes_read=recvfrom(rx->audio_socket,rx->output_buffer,sizeof(rx->output_buffer),0,(struct sockaddr*)&audio,(socklen_t *)&audio_length);
		if(bytes_read<0) {
			perror("recvfrom socket failed for audio buffer");
			exit(1);
		}
#endif
		if (softrock_get_jack () == 1) { 
			if (second_time == 32) {// This is for testing only.  It will fail the second connection from QtRadio.
				softrock_set_client_active_rx(rx->client->receiver, ADD_RX);	
			}
			second_time++;
			/*if ( second_time == 4000 ) {
				fprintf(stderr, "reached 4000.\n");
				exit(0);
			}*/	

#ifdef USE_PIPES
			//fprintf(stderr,"client.c pipe_left is: %d \n",pipe_left);
			//fprintf(stderr,"rx->client->receiver is: %d\n",rx->client->receiver);
			error_no = write(pipe_left, &rx->output_buffer[0], num_bytes);
			if (error_no == -1) {
				if ( softrock_get_verbose () == 1) perror("There were problems writing the left pipe for Jack in client.c");
				//fprintf(stderr, "Note: resource temporarilly unavailable indicates write would have blocked.  , blocked %d times\n",blocked_num);
				fprintf(stderr, "blocked %d times\nwritten %d times\n",blocked_num,second_time);
				blocked_num++;
			}
			//else fprintf(stderr,"Wrote %d bytes on left channel.\n",num_bytes);
			error_no = write(pipe_right, &rx->output_buffer[BUFFER_SIZE], num_bytes);
			if (error_no == -1) {
				if ( softrock_get_verbose () == 1) perror("There were problems writing the right pipe for Jack in client.c.");
				fprintf(stderr, "Note: resource temporarilly unavailable indicates write would have blocked.\n");
			}
#else  // Use ringbuffers
			if (( jack_ringbuffer_write_space (rb_left) >= num_bytes ) &&  (jack_ringbuffer_write_space (rb_right) >= num_bytes ))
			{
				jack_ringbuffer_write (rb_left, (const char *) &rx->output_buffer[0], num_bytes);
				jack_ringbuffer_write (rb_right, (const char *) &rx->output_buffer[BUFFER_SIZE], num_bytes);
			}
			else
			{
				fprintf(stderr, "No space left to write in jack ringbuffers.\n");
			}
#endif
		} else {
			process_softrock_output_buffer(rx->output_buffer,&rx->output_buffer[BUFFER_SIZE]);
		}
	}
	softrock_set_client_active_rx (rx->client->receiver, DEC_RX); //How do we ever get here?
}

			

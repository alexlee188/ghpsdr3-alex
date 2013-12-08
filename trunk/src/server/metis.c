/**
* @file metis.c
* @brief Metis protocol implementation
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
#ifdef __linux
#include <sys/types.h>
#include <sys/socket.h>
#include <sys/ioctl.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <net/if_arp.h>
#include <net/if.h>
#include <ifaddrs.h>
#include <pthread.h>
#include <unistd.h>
#else
#include "pthread.h"
#endif

#include <string.h>
#include <errno.h>

#include "metis.h"
#include "ozy.h"
#include "util.h"

#define MAX_METIS_CARDS 10
METIS_CARD metis_cards[MAX_METIS_CARDS];

#define DISCOVER_IDLE 0
#define DISCOVER_SENT 1
//static int discover_state=DISCOVER_IDLE;

#define PORT 1024
#define DISCOVERY_SEND_PORT PORT
#define DISCOVERY_RECEIVE_PORT PORT
#define DATA_PORT PORT

static int discovery_socket;
static struct sockaddr_in discovery_addr;
static int discovery_length;

static int discovering;

static unsigned char hw_address[6];
static long ip_address;

//static int data_socket;
static struct sockaddr_in data_addr;
static int data_addr_length;

static unsigned char buffer[70];

static pthread_t receive_thread_id;
static pthread_t watchdog_thread_id;
static int found=0;

int ep;
long sequence=-1;

void* metis_receive_thread(void* arg);
void* metis_watchdog_thread(void* arg);
void metis_send_buffer(unsigned char* buffer,int length);

#define inaddrr(x) (*(struct in_addr *) &ifr->x[sizeof sa.sin_port])

static int get_addr(int sock, char * ifname) {

  struct ifreq *ifr;
  struct ifreq ifrr;
  struct sockaddr_in sa;
  unsigned char      *u;
  int i;

  ifr = &ifrr;
  ifrr.ifr_addr.sa_family = AF_INET;
  strncpy(ifrr.ifr_name, ifname, sizeof(ifrr.ifr_name));

  if (ioctl(sock, SIOCGIFADDR, ifr) < 0) {
    printf("No %s interface.\n", ifname);
    return -1;
  }

  ip_address=inaddrr(ifr_addr.sa_data).s_addr;

  if (ioctl(sock, SIOCGIFHWADDR, ifr) < 0) {
    printf("No %s interface.\n", ifname);
    return -1;
  }

  u = (unsigned char *) &ifr->ifr_addr.sa_data;

  for(i=0;i<6;i++)
      hw_address[i]=u[i];


  return 0;
}

void metis_discover(char* interface,char* metisip) {
    int rc;
    int i;
    int on=1;
    //struct ifreq ifr;

    fprintf(stderr,"Looking for Metis card on interface %s\n",interface);

    discovering=1;
    
    // send a broadcast to locate metis boards on the network
    discovery_socket=socket(PF_INET,SOCK_DGRAM,IPPROTO_UDP);
    if(discovery_socket<0) {
        perror("create socket failed for discovery_socket\n");
        exit(1);
    }

    // get my MAC address and IP address
    if(get_addr(discovery_socket,interface)<0) {
        exit(1);
    }

    printf("%s IP Address: %ld.%ld.%ld.%ld\n",
              interface,
              ip_address&0xFF,
              (ip_address>>8)&0xFF,
              (ip_address>>16)&0xFF,
              (ip_address>>24)&0xFF);

    printf("%s MAC Address: %02x:%02x:%02x:%02x:%02x:%02x\n",
         interface,
         hw_address[0], hw_address[1], hw_address[2], hw_address[3], hw_address[4], hw_address[5]);


    // start a receive thread to get discovery responses
    rc=pthread_create(&receive_thread_id,NULL,metis_receive_thread,NULL);
    if(rc != 0) {
        fprintf(stderr,"pthread_create failed on metis_receive_thread: rc=%d\n", rc);
        exit(1);
    }

    // bind to this interface
    struct sockaddr_in name={0};
    name.sin_family = AF_INET;
    name.sin_addr.s_addr = ip_address;
    name.sin_port = htons(DISCOVERY_SEND_PORT);
    bind(discovery_socket,(struct sockaddr*)&name,sizeof(name));


    // allow broadcast on the socket
    rc=setsockopt(discovery_socket, SOL_SOCKET, SO_BROADCAST, &on, sizeof(on));
    if(rc != 0) {
        fprintf(stderr,"cannot set SO_BROADCAST: rc=%d\n", rc);
        exit(1);
    }

    discovery_length=sizeof(discovery_addr);
    memset(&discovery_addr,0,discovery_length);
    discovery_addr.sin_family=AF_INET;
    discovery_addr.sin_port=htons(DISCOVERY_SEND_PORT);
    discovery_addr.sin_addr.s_addr=htonl(INADDR_BROADCAST);

    buffer[0]=0xEF;
    buffer[1]=0xFE;
    buffer[2]=0x02;
    for(i=0;i<60;i++) {
        buffer[i+3]=0x00;
    }

    if(sendto(discovery_socket,buffer,63,0,(struct sockaddr*)&discovery_addr,discovery_length)<0) {
        perror("sendto socket failed for discovery_socket\n");
        exit(1);
    }
}

int metis_found() {
    return found;
}

char* metis_ip_address(int entry) {
    if(entry>=0 && entry<found) {
        return metis_cards[entry].ip_address;
    }
    return NULL;
}

char* metis_mac_address(int entry) {
    if(entry>=0 && entry<found) {
        return metis_cards[entry].mac_address;
    }
    return NULL;
}

/*
    Start Command

    Metis will start sending UDP/IP data to the ‘from port’, IP address and MAC address of a
    PC that sends a Start Command. The Start Command is a UDP/IP frame sent to the
    Ethernet address assigned to the Metis card and port 1024. The command has the
    following format:

        <0xEFFE><0x04><Command>< 60 bytes of 0x00>

     where

     Command = 1 byte (bit [0] set starts I&Q + Mic data and bit [1] set starts the wide bandscope data)

*/

void metis_start_receive_thread() {
    int i;
    int rc;
    struct hostent *h;

    fprintf(stderr,"Metis starting receive thread\n");

    discovering=0;

    h=gethostbyname(metis_cards[0].ip_address);
    if(h==NULL) {
        fprintf(stderr,"metis_start_receiver_thread: unknown host %s\n",metis_cards[0].ip_address);
        exit(1);
    }

    data_addr_length=sizeof(data_addr);
    memset(&data_addr,0,data_addr_length);
    data_addr.sin_family=AF_INET;
    data_addr.sin_port=htons(DATA_PORT);
    memcpy((char *)&data_addr.sin_addr.s_addr,h->h_addr_list[0],h->h_length);

    ozy_prime();
    
    // send a packet to start the stream
    buffer[0]=0xEF;
    buffer[1]=0xFE;
    buffer[2]=0x04;    // data send state send (0x00=stop)
    buffer[3]=0x01;    // I/Q only

    for(i=0;i<60;i++) {
        buffer[i+4]=0x00;
    }

    metis_send_buffer(&buffer[0],64);

fprintf(stderr,"starting metis_watchdog_thread\n");
    // start a watchdog to make sure we are receiving frames
    rc=pthread_create(&watchdog_thread_id,NULL,metis_watchdog_thread,NULL);
    if(rc != 0) {
        fprintf(stderr,"pthread_create failed on metis_watchdog_thread: rc=%d\n", rc);
        exit(1);
    }

}

static unsigned char input_buffer[2048];

/* 
   Metis Discovery reply format

   Upon receipt of this broadcast a Metis board will reply with a UDP/IP frame sent to the
   ‘from port’ on the IP Address and MAC Address of the PC originating the Discovery
   broadcast.

   The payload of the UDP/IP reply frame is as follows:

      <0xEFFE><Status>< Metis MAC Address><Code Version><Board_ID><49 bytes of 0x00>

   where

   Status            = 1 byte, 0x02 if Metis is not sending data and 0x03 if it is
   Metis MAC Address = 6 bytes (MAC Address of the Metis responding to the Discovery Broadcast)
   Code Version      = 1 byte, version number of code currently loaded into Metis
   Board_ID          = 1 byte, 0x00 = Metis, 0x01 = Hermes, 0x02 = Griffin

*/

void* metis_receive_thread(void* arg) {
    struct sockaddr_in addr;
    unsigned int length;
    int bytes_read;

    length=sizeof(addr);
    while(1) {
   	bytes_read=recvfrom(discovery_socket,input_buffer,sizeof(input_buffer),0,(struct sockaddr*)&addr,&length);
        if(bytes_read<0) {
            perror("recvfrom socket failed for metis_receive_thread");
            exit(1);
        }

        if(input_buffer[0]==0xEF && input_buffer[1]==0xFE) {
            switch(input_buffer[2]) {
                case 1:
                    if(!discovering) {
                        // get the end point
                        ep=input_buffer[3]&0xFF;

                        // get the sequence number
                        sequence=((input_buffer[4]&0xFF)<<24)+((input_buffer[5]&0xFF)<<16)+((input_buffer[6]&0xFF)<<8)+(input_buffer[7]&0xFF);
                        //fprintf(stderr,"received data ep=%d sequence=%ld\n",ep,sequence);

                        switch(ep) {
                            case 6:
                                // process the data
                                process_ozy_input_buffer(&input_buffer[8]);
                                process_ozy_input_buffer(&input_buffer[520]);
                                break;
                            case 4:
                                //fprintf(stderr,"EP4 data\n");
                                break;
                            default:
                                fprintf(stderr,"unexpected EP %d length=%d\n",ep,bytes_read);
                                break;
                        }
                    } else {
                        fprintf(stderr,"unexpected data packet when in discovery mode\n");
                    }
                    break;
                case 2:  // response to a discovery packet - hardware is not yet sending
                case 3:  // response to a discovery packet - hardware is already sending
                    if(discovering) {
                        if(found<MAX_METIS_CARDS) {
                            // get MAC address from reply
                            sprintf(metis_cards[found].mac_address,"%02X:%02X:%02X:%02X:%02X:%02X",
                                       input_buffer[3]&0xFF,input_buffer[4]&0xFF,input_buffer[5]&0xFF,input_buffer[6]&0xFF,input_buffer[7]&0xFF,input_buffer[8]&0xFF);
                            fprintf(stderr,"Metis MAC address %s\n",metis_cards[found].mac_address);
    
                            // get ip address from packet header
                            sprintf(metis_cards[found].ip_address,"%d.%d.%d.%d",
                                       addr.sin_addr.s_addr&0xFF,
                                       (addr.sin_addr.s_addr>>8)&0xFF,
                                       (addr.sin_addr.s_addr>>16)&0xFF,
                                       (addr.sin_addr.s_addr>>24)&0xFF);
                            fprintf(stderr,"Metis IP address %s\n",metis_cards[found].ip_address);
                            metis_cards[found].code_version = input_buffer[9];
                            switch (input_buffer[10]) {
                               case 0x00:
                                  metis_cards[found].board_id = "Metis";
                                  break;
                               case 0x01:
                                  metis_cards[found].board_id = "Hermes";
                                  break;
                               case 0x02:
                                  metis_cards[found].board_id = "Griffin";
                                  break;
                               default: 
                                  metis_cards[found].board_id = "unknown";
                                  break;
                            }       
                            fprintf(stderr,"Board id: %s",metis_cards[found].board_id);
                            fprintf(stderr," version:  %1.2F\n",metis_cards[found].code_version /10.0);

                            found++;
                            if(input_buffer[2]==3) {
                                fprintf(stderr,"Metis is sending\n");
                            }
                        } else {
                            fprintf(stderr,"too many metis cards!\n");
                        }
                    } else {
                        fprintf(stderr,"unexepected discovery response when not in discovery mode\n");
                    }
                    break;
                default:
                    fprintf(stderr,"unexpected packet type: 0x%02X\n",input_buffer[2]);
                    break;
            }
        } else {
            fprintf(stderr,"received bad header bytes on data port %02X,%02X\n",input_buffer[0],input_buffer[1]);
        }

    }
    
}

static unsigned char output_buffer[1032];
static long send_sequence=-1;
static int offset=8;

int metis_write(unsigned char ep,unsigned char* buffer,int length) {
    int i;

//fprintf(stderr,"metis_write\n");
    if(offset==8) {

        send_sequence++;
        output_buffer[0]=0xEF;
        output_buffer[1]=0xFE;
        output_buffer[2]=0x01;
        output_buffer[3]=ep;
        output_buffer[4]=(send_sequence>>24)&0xFF;
        output_buffer[5]=(send_sequence>>16)&0xFF;
        output_buffer[6]=(send_sequence>>8)&0xFF;
        output_buffer[7]=(send_sequence)&0xFF;

        // copy the buffer over
        for(i=0;i<512;i++) {
            output_buffer[i+offset]=buffer[i];
        }
        offset=520;
    } else {
        // copy the buffer over
        for(i=0;i<512;i++) {
            output_buffer[i+offset]=buffer[i];
        }
        offset=8;

        // send the buffer
        metis_send_buffer(&output_buffer[0],1032);

    }

    return length;
}

void metis_send_buffer(unsigned char* buffer,int length) {
//fprintf(stderr,"metis_send_buffer: length=%d\n",length);
    if(sendto(discovery_socket,buffer,length,0,(struct sockaddr*)&data_addr,data_addr_length)<0) {
        perror("sendto socket failed for metis_send_data\n");
        exit(1);
    }
}

void* metis_watchdog_thread(void* arg) {
    long last_sequence=-1;
    // sleep for 1 second
    // check if packets received
    fprintf(stderr,"running metis_watchdog_thread\n");
    while(1) {
//fprintf(stderr,"watchdog sleeping...\n");
        sleep(1);
        if(sequence==last_sequence) {
            fprintf(stderr,"No metis packets for 1 second: sequence=%ld\n",sequence);
            dump_metis_buffer("last frame received",sequence,input_buffer);
            dump_metis_buffer("last frame sent",send_sequence,output_buffer);
            exit(1);
        }
        last_sequence=sequence;
   }

   fprintf(stderr,"exit watchdog thread\n");
}

//register.c

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <libconfig.h>

#include "register.h"


char *dspstatus = "Idle";
char *call = "Unknown";
char *location = "Unknown";
char *band = "Unknown";
char *rig = "Unknown";
char *ant = "Unknown";


/* Converts an integer value to its hex character*/
char to_hex(char code) {
  static char hex[] = "0123456789abcdef";
  return hex[code & 15];
}

/* Returns a url-encoded version of str */
/* IMPORTANT: be sure to free() the returned string after use */
/* From geekhideout.com/urlcode.shtml */
char *url_encode(char *str) {
  char *pstr = str, *buf = malloc(strlen(str) * 3 + 1), *pbuf = buf;
  while (*pstr) {
    if (isalnum(*pstr) || *pstr == '-' || *pstr == '_' || *pstr == '.' || *pstr == '~') 
      *pbuf++ = *pstr;
    else if (*pstr == ' ') 
      *pbuf++ = '+';
    else 
      *pbuf++ = '%', *pbuf++ = to_hex(*pstr >> 4), *pbuf++ = to_hex(*pstr & 15);
    pstr++;
  }
  *pbuf = '\0';
  return buf;
}


void *doReg(){
  int result;
  char sCmd[255];
  while(1){
	  sprintf(sCmd,"wget -q -O - --post-data 'call=%s&location=%s&band=%s&rig=%s&ant=%s&status=%s'  http://qtradio.napan.ca/qtradio/qtradioreg.pl ", call, location, band, rig, ant, dspstatus);
	  result = system(sCmd);
	  sleep(300);  //seconds between web updates
	}
}


void init_register(){
      
  // try to open share_config_file for reading
  // if it fails try writing a default config file    
	  FILE *file;
	  if (file = fopen(share_config_file, "r")) { 
		  fclose(file);
	  }else{
		  if (file = fopen(share_config_file,"w")) {
			fprintf(file,"%s\n","# Simple config file for ghpsdr3's dspserver.");
			fprintf(file,"%s\n","# default file is located at ~/dspserver.conf when dsp server is started with --share");
			fprintf(file,"%s\n","# The information below will be supplied to a web database which will aid QtRadio");
			fprintf(file,"%s\n","# users find active dspservers to connect to.  You may also see the current list at");
			fprintf(file,"%s\n","# http://napan.ca/qtradio/qtradio.pl");
			fprintf(file,"%s\n","# valid fields are call, location, band, rig and ant");
			fprintf(file,"%s\n","# lines must end with ; and any characters after a # is a comment and ignored");
			fprintf(file,"%s\n","# field values must be enclosed with \" ie: \"xxxx\"");
			fprintf(file,"%s\n","# This default file will be created if dspserver is started with the --share option and ~/dspserver.cfg does not exist");
			fprintf(file,"%s\n","# You may also start dspserver with an alternate config file by starting dspserver with --shareconfig /home/alternate_filename.conf");
			fprintf(file,"%s\n","# Note field names are all lowercase!");

			fprintf(file,"\n%s\n","call = \"Unknown\";");
			fprintf(file,"%s\n","location = \"Unknown\";");
			fprintf(file,"%s\n","band = \"Unknown\";");
			fprintf(file,"%s\n","rig = \"Unknown\";");
			fprintf(file,"%s\n","ant = \"Unknown\";");
			fclose(file);
			fprintf(stderr,"%s\n", "**********************************************************");
			fprintf(stderr,"%s\n", "  A new  dspserver config file template has been created at: ");
			fprintf(stderr,"  %s\n", share_config_file);
			fprintf(stderr,"%s\n", "  Please edit this file and fill in your station details!");
			fprintf(stderr,"%s\n", "***********************************************************");
		  } else{
			  fprintf(stderr, "Error Can't create config file %s\n", share_config_file);
		  }
		  
	  }           
  
  
  
  config_t cfg;
  config_setting_t *setting;
  const char *str;
  config_init(&cfg);
  if(! config_read_file(&cfg, share_config_file))
  {
    fprintf(stderr, "Error - %s\n",  config_error_text(&cfg));
    config_destroy(&cfg);
    
  }else{
	 if(config_lookup_string(&cfg, "call", &str)){
         call = str;
     }
     if(config_lookup_string(&cfg, "location", &str)){
         location = str;
     }
     if(config_lookup_string(&cfg, "band", &str)){
         band = str;
     }
	 if(config_lookup_string(&cfg, "rig", &str)){
         rig = str;
     }
     if(config_lookup_string(&cfg, "ant", &str)){
         ant = str;
     }
	 //fprintf(stderr, "\nCall: %s\n", call);
	 //fprintf(stderr, "Location: %s\n", location);  
	 //fprintf(stderr, "Band: %s\n", band);
	 //fprintf(stderr, "Rig: %s\n", rig);
	 //fprintf(stderr, "Ant: %s\n\n", ant);
  }	    
  
    call = url_encode(call);
    location = url_encode(location);
    band = url_encode(band);
    rig = url_encode(rig);
    ant = url_encode(ant); 
    
    pthread_t thread1;
    int t_ret1;
    t_ret1 = pthread_create( &thread1, NULL, doReg, (void*) NULL);
}

void *doUpdate(){
	int result;
    char sCmd[255];
    sprintf(sCmd,"wget -q -O - --post-data 'call=%s&location=%s&band=2%s&rig=%s&ant=%s&status=%s'  http://qtradio.napan.ca/qtradio/qtradioreg.pl ", call, location, band, rig, ant, dspstatus);
	result = system(sCmd);

}


void updateStatus(char *status){
	dspstatus = status;
	pthread_t thread2;
    int t_ret2;
    t_ret2 = pthread_create( &thread2, NULL, doUpdate, (void*) NULL);
}


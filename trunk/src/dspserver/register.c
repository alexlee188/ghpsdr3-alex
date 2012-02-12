//register.c

#include <pthread.h>
#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <fcntl.h>
#include <stdlib.h>
#include <libconfig.h>
#include <unistd.h>
#include <ctype.h>
#include "register.h"
#include "defs.h"
#include "client.h"

char *dspstatus = "0 Client(s)";
char *call = "Unknown";
char *location = "Unknown";
char *band = "Unknown";
char *rig = "Unknown";
char *ant = "Unknown";
//config_t cfg;
struct config_t cfg;

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
	  strcat(servername,"Unknown");
	  FILE *file;
	  file = fopen(share_config_file, "r");
	  if (file) { 
		  fclose(file);
	  }else{
		  file = fopen(share_config_file,"w");
		  if (file) {
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
			fprintf(file,"%s\n","share = \"yes\"; # Can be yes, no");
			fprintf(file,"%s\n","lookupcountry = \"no\"; # Can be yes, no");
			fprintf(file,"\n%s\n","#### Following are new TX options #####");
			fprintf(file,"\n%s\n","tx = \"no\";  #Can be: no, yes, password");
			fprintf(file,"\n%s\n","#ve9gj = \"secretpassword\";  #add users/passwords one per line (Remove leading # ! max 20 characters each");
			fprintf(file,"\n%s\n","groupnames = [\"txrules1\"];  #add group or rulesset names in [\"name1\", \"name2\"]; format (max 20 characters each");
			fprintf(file,"\n%s\n", "# Add user names in [\"call1\", \"call2\"]; format to list members for each groupname above with an suffix of \"_members\" ");
			fprintf(file,"%s\n","txrules1_members = [\"ve9gj\", \"call2\"]; ");
			fprintf(file,"\n%s\n","# Rule sets are defined as group or rulesset name =( (\"mode\", StartFreq in Mhz, End Freq in Mhz),(\"mode\", StartFreq in Mhz, End Freq in Mhz) );");
			fprintf(file,"%s\n","#  Valid modes are  * SSB, CW, AM, DIG, FM, DRM ,SAM, SPEC Where * means any mode is OK");
			fprintf(file,"%s\n","#  The two rules below allow any mode on 20M and CW only on the bottom 100Khz of 80M");
			fprintf(file,"%s\n","#  You can make as many rules and rulesets as you wish. The first matching rule will allow TX");
			fprintf(file,"\n%s\n","txrules1 = (\n     (\"*\",14.0,14.350), # mode, StartFreq Mhz, EndFreq Mhz");
			fprintf(file,"%s\n","     (\"CW\",3.5,3.6)");
			fprintf(file,"%s\n","          );");
			
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
  
  
  
  
  const char *str;
  config_init(&cfg);
  if(! config_read_file(&cfg, share_config_file))
  {
    fprintf(stderr, "Error - %s\n",  config_error_text(&cfg));
    config_destroy(&cfg);
    
  }else{
	 if(config_lookup_string(&cfg, "call", &str)){
         call = str;
         strncpy(servername,str, 20);
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
     if(config_lookup_string(&cfg, "share", &str)){
         if (strcmp(str, "yes") == 0){
			 toShareOrNotToShare = 1;
         }else{
             toShareOrNotToShare = 0;
         }
     }else{
		fprintf(stderr, "Conf File Error - %s%s%s\n",  "Your ",share_config_file, " is missing a share= setting!!" );
	 } 
     if(config_lookup_string(&cfg, "lookupcountry", &str)){
         if (strcmp(str, "yes") == 0){
			 setprintcountry();
         }
     }else{
		fprintf(stderr, "Conf File Error - %s%s%s\n",  "Your ",share_config_file, " is missing a lookupcountry= setting!!" );
	 } 
     if(config_lookup_string(&cfg, "tx", &str)){
         if (strcmp(str, "yes") == 0){
			 txcfg = TXALL;
         }else if(strcmp(str, "password") == 0){
             txcfg = TXPASSWD;
         }else{
			 txcfg = TXNONE;
		 }
     }else{
		fprintf(stderr, "Conf File Error - %s%s%s\n",  "Your ",share_config_file, " is missing a tx= setting!!\n TX is now disabled" );
		txcfg = TXNONE;
	 } 
     
  }	    
  
    if (strcmp(call, "Unknown") == 0){
		fprintf(stderr,"%s\n", "**********************************************************");
		fprintf(stderr,"%s\n", "  Your config file located at: ");
		fprintf(stderr,"  %s\n", share_config_file);
		fprintf(stderr,"%s\n", "  Contains Unknown for a Call");
		fprintf(stderr,"%s\n", "  Please edit this file and fill in your station details!");
		fprintf(stderr,"%s\n", "***********************************************************");
	}
    call = url_encode(call);
    location = url_encode(location);
    band = url_encode(band);
    rig = url_encode(rig);
    ant = url_encode(ant); 
    
    pthread_t thread1;
    int ret;
    if (toShareOrNotToShare){
      ret = pthread_create( &thread1, NULL, doReg, (void*) NULL);
      ret = pthread_detach(thread1);
    }
}

void *doUpdate(){
	if(toShareOrNotToShare){
	  int result;
      char sCmd[255];
      sprintf(sCmd,"wget -q -O - --post-data 'call=%s&location=%s&band=%s&rig=%s&ant=%s&status=%s'  http://qtradio.napan.ca/qtradio/qtradioreg.pl ", call, location, band, rig, ant, dspstatus);
	  result = system(sCmd);
	}
    return 0;
}


void updateStatus(char *status){
	dspstatus = status;
	pthread_t thread2;
    int ret;
    ret = pthread_create( &thread2, NULL, doUpdate, (void*) NULL );
    ret = pthread_detach(thread2);
}

void doRemove(){
	int result;
    char sCmd[255];
    sprintf(sCmd,"wget -q -O - --post-data 'call=%s&location=%s&band=%s&rig=%s&ant=%s&status=Down'  http://qtradio.napan.ca/qtradio/qtradioreg.pl ", call, location, band, rig, ant);
	result = system(sCmd);
    
}

void close_register(){
	config_destroy(&cfg);
	
}

int chkPasswd(char *user, char *pass){
  const char *val;
	// check and see if this user/pass is valid in conf file
  if(config_lookup_string(&cfg, user, &val)){
         if(strcmp(val,pass) == 0){
			 //It's good
			 return 0;
		 }
     }
  return 1; //no good
}

int chkFreq(char *user,  long long freq2chk, int mode){
	// look through settings in conf file until an OK is found
	// returns on first matching rule
	// remove //// for debug fprintf
	
	char grpname[31];
	char grpmembers[41];
	int n, n1, n2;
	int  memberscount;
	int  groupnamescount;
	char rulemode[5];
	int modeOK = 1;
	int rulecount;
	
	float freq;
	freq = freq2chk * .000001;
	////fprintf(stderr,"checkfreq:%lld freq:%f\n",freq2chk, freq);
	if (txcfg == TXNONE) {
		return 1;
	}
	if (txcfg == TXALL) {
		return 0;
	}
	config_setting_t *groupnames= NULL;
	config_setting_t *members= NULL;
	config_setting_t *rules= NULL;
	config_setting_t *rule= NULL;
	
	groupnames = config_lookup(&cfg, "groupnames"); 
	if (!groupnames){
	   fprintf(stderr, "Conf File Error - %s%s%s\n",  "Your ",share_config_file, " is missing a groupnames= [\"txgroup1\"]setting!!\n TX is disabled" );
	   return 2;
	}	
    groupnamescount = config_setting_length(groupnames);
    for (n = 0; n < groupnamescount; n++) {
      // Look through these groupnames_members and see if our user is a member
      strncpy(grpname,config_setting_get_string_elem(groupnames, n),20);
      ////fprintf(stderr,"Groupname:%d of %d %s\n",n, groupnamescount, grpname);
      strcpy( grpmembers,grpname);
      strcat( grpmembers,"_members");
      members = config_lookup(&cfg, grpmembers); 
      if (!members){
	    fprintf(stderr, "Conf File Error - %s%s%s\n",  "Your ",share_config_file, " is missing a groupname_members= [\"member1\",\"member2\"]setting!!\n" );
	  }else{	
	    memberscount = config_setting_length(members);
        for (n1 = 0; n1 < memberscount; n1++) {
			 ////fprintf(stderr,"     %s %d of %d %s\n",grpmembers, n1, memberscount, config_setting_get_string_elem(members, n1));
		     if (strcmp(user,config_setting_get_string_elem(members, n1)) == 0 ){
		       //our user is a member of this group
			   ////fprintf(stderr, "     %s is a member of %s\n",config_setting_get_string_elem(members, n1),grpname);
			   rules = config_lookup(&cfg, grpname);
			   if (!rules){
	              fprintf(stderr, "Conf File Error - %s%s%s%s%s",  "Your ",share_config_file, " is missing a set of rules for ",grpname, "= (\n (\"mode\",statfreq,endfreq )\n);\n");
	           }else{
				  // look through each rule and if it matches then we can TX
				  rulecount = config_setting_length(rules);
				  ////fprintf(stderr,"Rule count for %s = %d\n",grpname, rulecount);
				  for (n2 = 0; n2 < rulecount; n2++) {
					rule = config_setting_get_elem (rules,n2);
					   if (!rule || config_setting_length(rule) != 3){
						   fprintf(stderr, "Conf File Error - %s%s%s%s%s%d%s",  "Your ",share_config_file, " is misconfigured for rulegroup ",grpname, " Rule Number ",n2," has a problem\n");
						   return 4; 
				       }else{
					       //Check this rule
					       // Do mode first Kind of long but we wat to allow users to write modes by name and not int in conf file
					       modeOK = 1; // set to nomatch first
					       strncpy(rulemode,config_setting_get_string_elem(rule,0),4); //mode is max of 4 chars
					       if(strcmp("*",rulemode) == 0){
							   // * any mode is OK
							   modeOK = 0;
				           }else if(mode == LSB && strcmp("SSB",rulemode) == 0 ){
							   modeOK = 0;
						   }else if(mode == USB && strcmp("SSB",rulemode) == 0 ){
							   modeOK = 0;
						   }else if(mode == DSB && strcmp("SSB",rulemode) == 0 ){
							   modeOK = 0;
						   }else if(mode == CWL && strcmp("CW",rulemode) == 0 ){
							   modeOK = 0;
						   }else if(mode == CWU && strcmp("CW",rulemode) == 0 ){
							   modeOK = 0;
						   }else if(mode == FMN && strcmp("FM",rulemode) == 0 ){
							   modeOK = 0;
						   }else if(mode == AM && strcmp("AM",rulemode) == 0 ){
							   modeOK = 0;
						   }else if(mode == DIGU && strcmp("DIG",rulemode) == 0 ){
							   modeOK = 0;
						   }else if(mode == SPEC && strcmp("SPEC",rulemode) == 0 ){
							   modeOK = 0;
						   }else if(mode == DIGL && strcmp("DIG",rulemode) == 0 ){
							   modeOK = 0;
						   }else if(mode == SAM && strcmp("SAM",rulemode) == 0 ){
							   modeOK = 0;
						   }else if(mode == DRM && strcmp("DRM",rulemode) == 0 ){
							   modeOK = 0;
						   }
						   ////fprintf(stderr,"  %s rule %d 0f %d modeOK:%d myfreq:%f rulestart:%f ruleend:%f ", grpname, n2, rulecount, modeOK, freq,config_setting_get_float_elem(rule,1),config_setting_get_float_elem(rule,2) );
						   if ( modeOK==0 && config_setting_get_float_elem(rule,1) <= freq && config_setting_get_float_elem(rule,2) >= freq ){
							   ////fprintf(stderr," Pass\n");
							   return 0; // good to TX
						   }else{
							   ////fprintf(stderr," Fail\n");
						   }
				       } 
			      }   
	  	       }
		   }else{
			   ////fprintf(stderr, "     %s is NOT a member of %s\n",config_setting_get_string_elem(members, n1),grpname);
		   }
        } 
      }
	}
	return 1;
}



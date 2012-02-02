//register.h


#if ! defined __REGISTER_H__
#define __REGISTER_H__

void init_register();
void updateStatus(char *status);
void close_register();
void doRemove();
int chkPasswd(char *user, char *pass);
int chkFreq(char *user,  long long freq2chk, int mode);

char share_config_file[256];
char *home;
int toShareOrNotToShare;

#define TXALL  0
#define TXPASSWD  1
#define TXNONE  2

char user[21];
char passwd[21];
int  txcfg;

#endif

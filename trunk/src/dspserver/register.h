//register.h


#if ! defined __REGISTER_H__
#define __REGISTER_H__

void init_register(const char *scfile);
void updateStatus(char *status);
void close_register();
void doRemove();
int chkPasswd(char *user, char *pass);
int chkFreq(char *user,  long long freq2chk, int mode);

extern int toShareOrNotToShare;

#define TXALL  0
#define TXPASSWD  1
#define TXNONE  2

extern int  txcfg;

#endif

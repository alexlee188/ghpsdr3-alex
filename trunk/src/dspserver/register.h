//register.h


#if ! defined __REGISTER_H__
#define __REGISTER_H__

void init_register();
void updateStatus(char *status);
void close_register();
void doRemove();
char share_config_file[256];
char *home;

#endif

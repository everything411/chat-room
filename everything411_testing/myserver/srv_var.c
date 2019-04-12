#include "srv_header.h"
/* global vars */
userdata_t users[FD_SETSIZE];
client_t client[FD_SETSIZE];
FILE *userconf;
int idmax = 0;
/* global vars for praser and related function */
char buf[MAXLINE];
char tmpbuf[4096];
char send_buffer[40960];
char cmd[4096];
char arg1[4096];
char arg2[4096];
int uid;
fd_set *pset;
//char arg3[100];
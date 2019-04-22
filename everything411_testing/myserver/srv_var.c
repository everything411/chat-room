#include "srv_header.h"
/* global vars */
userdata_t users[FD_SETSIZE];
client_t client[FD_SETSIZE];
FILE *userconf;
int idmax = 0;
/* global vars for praser and related function */
char buf[MAXLINE];
char tmpbuf[MAXLINE * 2 + 512];
char send_buffer[MAXLINE * 2 + 512];
char content_buf[MAXLINE * 2 + 512];
char cmd[MAXLINE];
char arg1[MAXLINE];
char arg2[MAXLINE];
int uid;
int KEYLEN;
fd_set *pset;
//char arg3[100];
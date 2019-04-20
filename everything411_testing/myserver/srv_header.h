#ifndef __SRV_HEADER
#define __SRV_HEADER
#include <stdio.h>
#include <errno.h>
#include <stdlib.h>
#include <strings.h>
#include <string.h>
#include <ctype.h>
#include <unistd.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define KEY "\x0f\x0e\x19\x13\x16\x12"
#define KEYLEN 6
#define SERV_PORT 9877
#define MAXLINE 4096
#define MAXCONN 1024
#define HELPINFO "{\n\"type\":\"help\",\n\"commands\":[\n"              \
                 "{\"login\":\"login your_user your_password\"},\n"     \
                 "{\"useradd\":\"useradd your_user your_password\"},\n" \
                 "{\"send\":\"send user_name your_content\"},\n"        \
                 "{\"broadcast\":\"broadcast your_content\"},\n"        \
                 "{\"userlist\":\"userlist\"},\n"                       \
                 "{\"useraddr\":\"useraddr user_name\"}\n"              \
                 "{\"passwd\":\"passwd new_user old_password\"},\n"     \
                 "]\n}\n"
/* info about client and socket */
typedef struct __client
{
    int fd;
    int uid;
    int new_conn;
    struct sockaddr_in sockaddr;
} client_t;

/* user info structure */
typedef struct __user
{
    char name[50];
    // char name_len;
    char password[50];
    int cliindex;
    signed char offline;
} userdata_t;

/* Write "n" bytes to a descriptor. */
ssize_t writen(int fd, const void *vptr, size_t n);
/* user name to uid */
int nametoid(char *name, int maxi);
/* send buffer to client */
void sendclient(int connindex);
/* preprocess the input send_buffer */
void bufinit(char *buf, int n);
/* read user from conf file */
void userinit(void);
/* escape char replace */
void escchar(char *str);
/* illeagal name check */
int illeagalchar(char *str);
/* this funtion prase the input and execute commands according to the input */
void parse(char *buf, int connindex, int maxi);
void login(char *buf, int connindex, int maxi);
void useradd(char *buf, int connindex);
void broadcast(char *buf, int connindex, int maxi);
void sendmessage(char *buf, int connindex);
void userlist(int connindex);
void useraddr(char *buf, int connindex);
void passwd(char *buf, int connindex);

void encode(char *str);
void decode(char *str);

extern FILE *userconf;
extern userdata_t users[FD_SETSIZE];
extern client_t client[FD_SETSIZE];
extern int idmax;
extern char buf[MAXLINE];

/* global vars for praser and related function */
extern char tmpbuf[MAXLINE * 2 + 512];
extern char send_buffer[MAXLINE * 2 + 512];
extern char content_buf[MAXLINE * 2 + 512];
extern char cmd[MAXLINE];
extern char arg1[MAXLINE];
extern char arg2[MAXLINE];
extern int uid;
extern fd_set *pset;
#endif

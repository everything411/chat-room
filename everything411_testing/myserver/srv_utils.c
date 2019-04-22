#include "srv_header.h"
#include "srv_error.h"
static int encode(char *str)
{
    static int len;
    if (send_buffer[0] == '{')
    {
        for (len = 0; str[len]; len++)
            str[len] ^= KEY[len % KEYLEN];
    }
    return len;
}
static void decode(char *str, int len)
{
    int isvalid = 0;
    for (int i = 0; i < len; i++)
    {
        if ((str[i] ^= KEY[i % KEYLEN]) == ' ')
            isvalid = 1;
        if (i > 20 && !isvalid)
        {
            str[i] = 0;
            return;
        }
    }
}
static void del_half_utf8(char *str)
{
    int cnt = 0;
    while (*str)
    {
        if (*str < 0)
        {
            cnt++;
        }
        str++;
    }
    if (cnt % 3)
    {
        str[-cnt % 3] = 0;
    }
}

ssize_t writen(int fd, const void *vptr, size_t n)
{
    size_t nleft;
    ssize_t nwritten;
    const char *ptr;

    ptr = vptr;
    nleft = n;
    while (nleft > 0)
    {
        if ((nwritten = write(fd, ptr, nleft)) <= 0)
        {
            if (nwritten < 0 && errno == EINTR)
                nwritten = 0; /* and call write() again */
            else
                return -1; /* error */
        }

        nleft -= nwritten;
        ptr += nwritten;
    }
    return n;
}

int nametoid(char *name, int maxi)
{
    for (int i = 0; i < idmax; i++)
    {
        if (!strcmp(name, users[i].name))
        {
            return i;
        }
    }
    return -1;
}

void bufinit(char *buf, int n)
{
    buf[n] = 0;
    decode(buf, n);
    while (buf[n - 1] == '\n')
    {
        buf[--n] = 0; //delete '\n' from line end
    }
    if (n == MAXLINE - 32)
    {
        del_half_utf8(buf);
    }
}
void sendclient(int connindex)
{
    int len = encode(send_buffer);
    writen(client[connindex].fd, send_buffer, len);
}

void userinit(void)
{
    // int conf_idmax;
    userconf = fopen("user.conf", "r+");
    if (userconf == NULL)
        return;
    // fscanf(fp, "%d", &conf_idmax);
    while (~fscanf(userconf, "%s%s", users[idmax].name, users[idmax].password))
        idmax++;
}
void escchar(char *str)
{

    int pos = 0;
    int tmppos = 0;
    while ((content_buf[tmppos] = str[pos]))
    {
        switch (str[pos])
        {
        case '\n':
            content_buf[tmppos++] = '\\';
            content_buf[tmppos] = 'n';
            break;
        case '\t':
            content_buf[tmppos++] = '\\';
            content_buf[tmppos] = 't';
            break;
        case '\"':
            content_buf[tmppos++] = '\\';
            content_buf[tmppos] = '\"';
            break;
        case '\\':
            content_buf[tmppos++] = '\\';
            content_buf[tmppos] = '\\';
            break;
        }
        tmppos++;
        pos++;
    }
}
int illeagalchar(char *str)
{
    while (*str)
    {
        if (!isalnum(*str) && *str != '_' && *str != '-')
            return 1;
        str++;
    }
    return 0;
}
void closeclient(int connindex)
{
    close(client[connindex].fd);
    FD_CLR(client[connindex].fd, pset);
    if (client[connindex].uid != -1)
    {
        users[client[connindex].uid].offline = -1;
    }
    client[connindex].fd = -1;
    client[connindex].uid = -1;
    client[connindex].new_conn = -1;
}
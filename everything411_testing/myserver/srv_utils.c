#include "srv_header.h"
#include "srv_error.h"
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
    while (buf[n - 1] == '\n')
    {
        buf[--n] = 0; //delete '\n' from line end
    }
}
void sendclient(int connindex)
{
    writen(client[connindex].fd, send_buffer, strlen(send_buffer));
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
void escchar(char *str, int len)
{
    static char tmp_buf[40960];
    int pos = 0;
    int tmppos = 0;
    while ((tmp_buf[tmppos] = str[pos]))
    {
        switch (str[pos])
        {
        case '\n':
            tmp_buf[tmppos++] = '\\';
            tmp_buf[tmppos] = 'n';
            break;
        case '\t':
            tmp_buf[tmppos++] = '\\';
            tmp_buf[tmppos] = 't';
            break;
        case '\"':
            tmp_buf[tmppos++] = '\\';
            tmp_buf[tmppos] = '\"';
            break;
        case '\\':
            tmp_buf[tmppos++] = '\\';
            tmp_buf[tmppos] = '\\';
            break;
            // case '\'':
            //     tmp_buf[tmppos++] = '\\';
            //     tmp_buf[tmppos] = '\'';
            //     break;
        }
        tmppos++;
        pos++;
        // puts(tmp_buf);
    }
    strcpy(str, tmp_buf);
    // strncpy(str, tmp_buf, len);
}
int illeagalchar(char *str)
{
    while (*str)
        if (!isalnum(*str) && *str != '_' && *str != '-')
            return 1;
    return 0;
}
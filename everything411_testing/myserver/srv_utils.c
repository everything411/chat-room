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
        // puts("half utf8 detacted!");
    }
}

void bufinit(char *buf, int n)
{
    buf[n] = 0;
    decode(buf);
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
    if (send_buffer[0] == '{')
        encode(send_buffer);
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
            // case '\'':
            //     tmp_buf[tmppos++] = '\\';
            //     tmp_buf[tmppos] = '\'';
            //     break;
        }
        tmppos++;
        pos++;
        // puts(tmp_buf);
    }
    // strcpy(str, tmp_buf);
    // strncpy(str, tmp_buf, len);
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
void encode(char *str)
{
    for (int i = 0; str[i]; i++)
        str[i] ^= KEY[i % KEYLEN];
}
void decode(char *str)
{
    for (int i = 0; str[i]; i++)
        str[i] ^= KEY[i % KEYLEN];
    // puts(str);
    // printf("%d\n", i);
}
#include "srv_header.h"
#include "srv_error.h"

void parse(char *buf, int connindex, int maxi)
{
    tmpbuf[0] = send_buffer[0] = cmd[0] = arg1[0] = arg2[0] = 0;
    if (sscanf(buf, "%s", cmd) != 1)
    {
        syntax_err();
    }
    else if (!strncmp(cmd, "\x36\x4a\x79", 3))
    {
        /* I'm not HTTP server! */
        strcpy(send_buffer, "HTTP/1.1 418 I\'m a teapot\n\n<h1>I\'m a teapot!</h1>\n");
        writen(client[connindex].fd, send_buffer, 51);
        closeclient(connindex);
    }
    else if (!strcmp(cmd, "help"))
    {
        strcpy(send_buffer, HELPINFO);
        sendclient(connindex);
    }
    else if (!strcmp(cmd, "login"))
    {
        login(buf, connindex, maxi);
    }
    else if (!strcmp(cmd, "useradd"))
    {
        not_implemented_err();
        // useradd(buf, connindex);
    }
    else if (!strcmp(cmd, "send"))
    {
        if (client[connindex].uid == -1)
        {
            not_login_err(); /* client must login to execute this commands */
            return;
        }
        sendmessage(buf, connindex);
    }
    else if (!strcmp(cmd, "broadcast"))
    {
        if (client[connindex].uid == -1)
        {
            not_login_err(); /* client must login to execute this commands */
            return;
        }
        broadcast(buf, connindex, maxi);
    }
    else if (!strcmp(cmd, "userlist"))
    {
        if (client[connindex].uid == -1)
        {
            not_login_err(); /* client must login to execute this commands */
            return;
        }
        userlist(connindex);
    }
    else if (!strcmp(cmd, "useraddr"))
    {
        if (client[connindex].uid == -1)
        {
            not_login_err(); /* client must login to execute this commands */
            return;
        }
        useraddr(buf, connindex);
    }
    else if (!strcmp(cmd, "passwd"))
    {
        if (client[connindex].uid == -1)
        {
            not_login_err(); /* client must login to execute this commands */
            return;
        }
        not_implemented_err();
        // passwd(buf, connindex);
    }
    else
    {
        strcpy(send_buffer, "Protocol mismatch\n");
        writen(client[connindex].fd, send_buffer, 18);
        closeclient(connindex);
        // syntax_err();
    }
}
void login(char *buf, int connindex, int maxi)
{
    if (sscanf(buf, "%*s%s%s", arg1, arg2) != 2)
    {
        syntax_err();
    }
    else if ((uid = nametoid(arg1, idmax)) == -1)
    {
        user_not_found_err();
    }
    else if (strcmp(arg2, users[uid].password))
    {
        wrong_pass_err();
    }
    else if (!client[connindex].new_conn)
    {
        already_login_cli_err();
    }
    else if (!users[uid].offline)
    {
        already_login_err();
    }
    else
    {
        client[connindex].new_conn = 0;
        client[connindex].uid = uid;
        users[uid].offline = 0;
        users[uid].cliindex = connindex;
        sprintf(send_buffer,
                "{\n\"type\":\"login\",\n\"status\":\"success\",\n\"uid\":%d,\n\"user\":\"%s\"\n}\n", uid, users[uid].name);
        sendclient(connindex);

        sprintf(send_buffer, "{\n\"type\":\"logininfo\",\n\"user\":\"%s\",\n\"uid\":%d\n}\n",
                users[client[connindex].uid].name, client[connindex].uid);
        for (int j = 0; j <= maxi; j++)
            if (client[j].fd != -1 && client[j].fd != client[connindex].fd && client[j].new_conn != -1)
                sendclient(j); /* broadcast login infomation */
    }
}
void useradd(char *buf, int connindex)
{
    if (sscanf(buf, "%*s%s%s", arg1, arg2) != 2)
    {
        syntax_err();
    }
    else if ((uid = nametoid(arg1, idmax)) != -1)
    {
        already_reg_err();
    }
    else if (illeagalchar(arg1))
    {
        illeagal_char_err();
    }
    else
    {
        strncpy(users[idmax].name, arg1, 50);
        strncpy(users[idmax].password, arg2, 50);
        users[idmax].name[49] = users[idmax].password[49] = 0;
        // users[idmax].name_len = strlen(users[idmax].name);
        sprintf(send_buffer,
                "{\n\"type\":\"registration\",\n\"status\":\"success\","
                "\n\"uid\":%d,\n\"user\":\"%s\"\n}\n",
                idmax, users[idmax].name);
        sendclient(connindex);
        if (userconf)
        {
            fprintf(userconf, "%s %s\n", users[idmax].name, users[idmax].password);
            fflush(userconf);
        }
        idmax++;
    }
}

void broadcast(char *buf, int connindex, int maxi)
{
    while (*buf == ' ') // ignore blank in line start
        buf++;
    while (*buf && *buf++ != ' ') // skip 'broadcast'
        ;
    escchar(buf);
    sprintf(send_buffer, "{\n\"type\":\"broadcast\",\n\"user\":\"%s\",\n\"uid\":%d,\n\"content\":\"%s\"\n}\n",
            users[client[connindex].uid].name, client[connindex].uid, content_buf);
    for (int j = 0; j <= maxi; j++)
        if (client[j].fd != -1 && client[j].new_conn != -1)
            sendclient(j);
}

void sendmessage(char *buf, int connindex)
{
    if (sscanf(buf, "%*s%s", arg1) != 1)
    {
        syntax_err();
    }
    else if ((uid = nametoid(arg1, idmax)) == -1)
    {
        user_not_found_err();
    }
    else if (users[uid].offline)
    {
        not_online_err();
    }
    else
    {
        while (*buf == ' ')
            buf++;
        while (*buf && *buf++ != ' ') // skip 'send'
            ;
        while (*buf && *buf++ != ' ') // skip username
            ;
        escchar(buf);
        sprintf(send_buffer,
                "{\n\"type\":\"message\",\n\"user\":\"%s\",\n\"uid\":%d,\n"
                "\"to\":\"%s\",\n\"touid\":%d,\n\"content\":\"%s\"\n}\n",
                users[client[connindex].uid].name, client[connindex].uid,
                arg1, uid, content_buf);
        if (client[connindex].uid != uid)
            sendclient(users[uid].cliindex);
        sendclient(connindex);
    }
}

void userlist(int connindex)
{
    sprintf(send_buffer, "{\n\"type\":\"userlist\",\n\"users\":[\n");
    for (int i = 0; i < idmax; i++)
    {
        if (!users[i].offline)
        {
            sprintf(tmpbuf, "{\"uid\":%d, \"user\":\"%s\"},\n", i, users[i].name);
            strcat(send_buffer, tmpbuf);
        }
    }
    int len = strlen(send_buffer);
    if (send_buffer[len - 2] == ',')
        send_buffer[len - 2] = '\n';
    strcpy(&send_buffer[len - 1], "]\n}\n");
    sendclient(connindex);
}

void useraddr(char *buf, int connindex)
{
    if (sscanf(buf, "%*s%s", arg1) != 1)
    {
        syntax_err();
    }
    else if ((uid = nametoid(arg1, idmax)) == -1)
    {
        user_not_found_err();
    }
    else if (users[uid].offline)
    {
        not_online_err();
    }
    else
    {
        inet_ntop(AF_INET, &client[users[uid].cliindex].sockaddr.sin_addr, buf, MAXLINE);
        sprintf(send_buffer, "{\n\"type\":\"useraddr\",\n\"uid\":%d,\n"
                             "\"user\":\"%s\",\n\"address\":\"%s\",\n\"port\":%d\n}\n",
                uid, users[uid].name, buf, ntohs(client[users[uid].cliindex].sockaddr.sin_port));
        sendclient(connindex);
    }
}
void passwd(char *buf, int connindex)
{
    if (sscanf(buf, "%*s%s%s", arg1, arg2) != 2)
    {
        syntax_err();
    }
    else if (strcmp(arg2, users[uid].password))
    {
        wrong_pass_err();
    }
    else
    {
        strncpy(users[client[connindex].uid].password, arg1, 50);
        sprintf(send_buffer,
                "{\n\"type\":\"password\",\n\"status\":\"success\",\n\"uid\":%d,\n\"user\":\"%s\"\n}\n",
                client[connindex].uid, users[client[connindex].uid].name);
        sendclient(connindex);
    }
}

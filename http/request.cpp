#if defined(_MSC_VER)
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
//disable _CRT_SECURE_NO_WARNINGS message to build this in VC++
#pragma warning(disable:4996)
#pragma warning(disable:4703)
#endif

#include <stdio.h>
#include <string.h>
#include "server.h"
#include <stdlib.h>

//network
#if !defined(WIN32)
#include <unistd.h> //malloc
#include <stdbool.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <arpa/inet.h>
#endif

int get_request_type(char *buf)
{
    char retval[10] = {0};
    sscanf(buf, "%s ", &retval);

    if (!strcmp(retval, "GET"))
        return GET;
    else if (!strcmp(retval, "POST"))
        return POST;
    else if (!strcmp(retval, "PUT"))
        return PUT;
    else
        return RQ_UNDEF;
}

char *get_request_value(char *buf)
{
    char retval[100] = {0};

    sscanf(buf, "%s %s ", &retval, &retval);  // tee hee

    if (retval[strlen(retval)-1] == '/')
        strcat(retval, "index.html");

    return strdup(retval);
}

REQUEST *GetRequest(int sock)
{
    REQUEST *request;
    int msg_len;
    char buf[REQUEST_SIZE];

    msg_len = recv(sock, buf, sizeof(buf), 0);
    //printf("Bytes Received: %d, message: %s from %s\n", msg_len, buf, inet_ntoa(client.sin_addr));

    request         = (REQUEST*)malloc(sizeof(REQUEST));
    request->type   = get_request_type(buf);
    request->value  = get_request_value(buf);
    request->length = msg_len;

    return request;
}

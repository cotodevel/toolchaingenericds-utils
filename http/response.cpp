#include <winsock2.h>
#include <stdio.h>
#include "server.h"
//#include <unistd.h>
#include "..\winDir.h"
#pragma comment(lib,"ws2_32.lib")

//disable _CRT_SECURE_NO_WARNINGS message to build this in VC++
#pragma warning(disable:4996)
#pragma warning(disable:4703)

char *get_full_path(char *name){
    char filename[1024] = {0};
    int i;

	#ifndef WIN32
	getcwd(filename, 1024);
	#endif
	
	#ifdef WIN32
	getCWDWin(filename, "");
	#endif

    #ifndef WIN32
    if ((filename[strlen(filename)] != '\\') && 
        (name[strlen(name)] != '/') &&
        (name[strlen(name)] != '\\'))
    {
        strcat(filename, "\\");
    }
	#endif

    for (i = 0; name[i]; i++){
        if (name[i] == '/'){
            name[i] = '\\';
		}
	}
    strcat(filename, name);
    return strdup(filename);
}

RESPONSE *GetResponse(REQUEST *request)
{
    RESPONSE *response;

    response = (RESPONSE*)malloc(sizeof(RESPONSE));
    response->error    = 0;
    response->filename = request->value;
    response->filepath = get_full_path(request->value);
    response->header   = get_header(response);

    return response;
}

int SendResponse(SOCKET sock, RESPONSE *response)
{
	char buf[1024] = {0};
    int msg_len;
	int result = 0;
	FILE *f;
    if (response->error) {
        send(sock, DEFAULT_ERROR_404, strlen(DEFAULT_ERROR_404), 0);
        return 1;
    }

    f = fopen(response->filepath, "rb");
    

    if (!f) {
        send(sock, "HTTP/1.1 500 Internal Server Error\r\nContent-Length: 0\r\n\r\n", 57, 0);
        return 1;
    }

    send(sock, response->header, strlen(response->header), 0);

    while ((result = fread(buf, 1, 1024, f)) > 0)
    {
        msg_len = send(sock, buf, result, 0);

        if (msg_len == SOCKET_ERROR) {
            //error_live("send()");
            printf("Error sending data, reconnecting...\n");
            closesocket(sock);
            return -1;
        }
        else if (!msg_len)
        {
            printf("Client closed connection\n");
            closesocket(sock);
            return 0;
            //WSACleanup();
        }
    }

    printf("Served file %s\n", response->filepath);

    return 1;
}

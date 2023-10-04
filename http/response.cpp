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
#include <stdbool.h>
#include <unistd.h> //malloc
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <arpa/inet.h>
#endif

#include "../winDir.h"

char *get_full_path(char *name){
    char filename[1024] = {0};
    int i;

	getCWDWin(filename, "");

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

int SendResponse(int sock, RESPONSE *response, bool exitAfterSentSingleFile)
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
	
	#ifdef WIN32
        if (msg_len == SOCKET_ERROR) {
        #endif
        
        #if !defined(WIN32)
        if (msg_len == SO_ERROR) {
        #endif
        
            printf("Error sending data, reconnecting...\n");
            #if !defined(_MSC_VER)
			close(sock);
			#endif
			#if defined(_MSC_VER)
			closesocket(sock);
			#endif
			return -1;
        }
        else if (!msg_len)
        {
            printf("Client closed connection\n");
            #if !defined(_MSC_VER)
			close(sock);
			#endif
            #if defined(_MSC_VER)
			closesocket(sock);
			#endif
			return 0;
        }
    }

    printf("Served file %s\n", response->filepath);

    //Single file transfer? Close server
    if(exitAfterSentSingleFile == true){
		#if !defined(_MSC_VER)
		close(sock);
		#endif
		#if defined(_MSC_VER)
		closesocket(sock);
		#endif
		
		return 0;
    }
    return 1;
}

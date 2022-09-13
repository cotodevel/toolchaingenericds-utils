#if defined(_MSC_VER)
#include <winsock2.h>
#pragma comment(lib,"ws2_32.lib")
//disable _CRT_SECURE_NO_WARNINGS message to build this in VC++
#pragma warning(disable:4996)
#pragma warning(disable:4703)
#endif

#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include "server.h"
#include "../utilities.h"

//network
#if !defined(WIN32)
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/ip.h> /* superset of previous */
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <arpa/inet.h>
#endif

int mainHTTPServer(int argc, char **argv){
    bool quitAfterSentSingleFileToClient=false;
    if( (argv[1] != NULL) && (strncmp(argv[1], "-quit", strlen("-quit")) == 0)){
	quitAfterSentSingleFileToClient=true;
    }
    int addr_len;
    struct sockaddr_in local, client_addr;
    int count = 0;
    
    #ifdef WIN32
    SOCKET sock, msg_sock;
    WSADATA wsaData;
    #endif
    
    #if !defined(WIN32)
    unsigned int sock, msg_sock;
    #endif
    
    #ifdef WIN32
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) == SOCKET_ERROR){
        printf("WSAStartup()");
    }
    #endif
    
    // Fill in the address structure
    local.sin_family        = AF_INET;
    local.sin_addr.s_addr   = INADDR_ANY;
    local.sin_port          = htons(DEFAULT_PORT);

    sock = socket(AF_INET, SOCK_STREAM, 0);

    #ifdef WIN32
    if (sock == INVALID_SOCKET){
    #endif
    #if !defined(WIN32)
    if (sock == -1){
    #endif
        printf("socket()");
    }
    
    #ifdef WIN32
    if (bind(sock, (struct sockaddr *)&local, sizeof(local)) == SOCKET_ERROR){
    #endif
    #if !defined(WIN32)
    if (bind(sock, (struct sockaddr *)&local, sizeof(local)) == -1){
    #endif
        printf("bind()");
    }
listen_goto:
	
    #ifdef WIN32
    if (listen(sock, 10) == SOCKET_ERROR){
    #endif
    #if !defined(WIN32)
    if (listen(sock, 10) == -1){
    #endif
        printf("listen()");
    }
    
	{
		u32 thisIp = Wifi_GetIP();
		uint8 bytes[4];
		bytes[0] = thisIp & 0xFF;
		bytes[1] = (thisIp >> 8) & 0xFF;
		bytes[2] = (thisIp >> 16) & 0xFF;
		bytes[3] = (thisIp >> 24) & 0xFF;
		printf("\n\nHTTP 1.0 Server. Port: %d - Mounted at: %d.%d.%d.%d\n", DEFAULT_PORT, bytes[0], bytes[1], bytes[2], bytes[3]);
	}
	if(quitAfterSentSingleFileToClient == true){
	printf("Quit inmediately after file transfer: ENABLED\n");
    }
    else{
	printf("Quit inmediately after file transfer: DISABLED\n");
    }
    printf("Waiting for connection...\n");

    forever
    {
        addr_len = sizeof(client_addr);
        
        #ifdef WIN32
        msg_sock = accept(sock, (struct sockaddr*)&client_addr, &addr_len);
		#endif
		#if !defined(WIN32)
		msg_sock = accept(sock, (struct sockaddr*)&client_addr, (unsigned int*)&addr_len);
		#endif
        if (
            #ifdef WIN32	
            msg_sock == INVALID_SOCKET || 
            #endif
            msg_sock == -1){
            	printf("accept()");
	}
        printf("\n\n#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$#$ %d\n\n", ++count);
        printf("Connected to %s:%d\n", inet_ntoa(client_addr.sin_addr), htons(client_addr.sin_port));

        REQUEST *request = GetRequest(msg_sock);
        printf("Client requested %d %s\n", request->type, request->value);

        if (request->length == 0)
            continue;

        RESPONSE *response = GetResponse(request);
        int sent = SendResponse(msg_sock, response, quitAfterSentSingleFileToClient);

        #if !defined(_MSC_VER)
		close(msg_sock);
		#endif
        #if defined(_MSC_VER)
		closesocket(msg_sock);
		#endif
        
		if (sent == 0)
            break;
        else if (sent == -1)
            goto listen_goto;

    }
	
    #ifdef WIN32
    WSACleanup();
    #endif
    return 0;
}

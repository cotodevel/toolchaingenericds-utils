#if defined(_MSC_VER)
#include <winsock2.h>
#endif

#include <stdio.h>
#include "server.h"

#if !defined(_MSC_VER)
#include <errno.h>
#endif

void error_live(const char *s)
{
    #ifdef WIN32
    fprintf(stderr, "Error: %s failed with error %d\n", s, WSAGetLastError());
    WSACleanup();
    #endif
    
    #if !defined(WIN32)
    fprintf(stderr, "Error: %s failed with error %d\n", s, errno);
    #endif
}

void error_die(const char *s)
{
    error_live(s);
    #ifdef WIN32
    exit(EXIT_FAILURE);
    #endif
}

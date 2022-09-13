#if defined(_MSC_VER)
#include <windows.h>
#include <atlstr.h> 
//disable _CRT_SECURE_NO_WARNINGS message to build this in VC++
#pragma warning(disable:4996)
#pragma warning(disable:4703)
#endif

#include <stdio.h>
#include "server.h"
#include "../utilities.h"

bool FilExists(char * szPath){
	return existFilePosix(szPath);
}

char *get_content_type(char *name)
{
    char *extension = strchr(name, '.');

    if (!strcmp(extension, ".html"))
        return "text/html";
    else if (!strcmp(extension, ".ico"))
        return "image/webp";
    else if (!strcmp(extension, ".css"))
        return "text/css";
    else if (!strcmp(extension, ".jpg"))
        return "image/jpeg";
    else if (!strcmp(extension, ".js"))
        return "text/javascript";

    return "*/*";
}

char *get_header(RESPONSE *rs)
{
	char header[1024] = {0};
	if (!FilExists(rs->filepath)) {
        printf("404 Not Found: %s\n", rs->filename);
        rs->error = 404;
        return "HTTP/1.1 404 Not Found\r\nContent-Length: 0\r\n\r\n";
    }

    
    sprintf(header, "HTTP/1.1 200 OK\r\nContent-Type: %s; charset=UTF-8\r\n\r\n", get_content_type(rs->filename));
    return strdup(header);
}

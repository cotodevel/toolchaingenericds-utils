#ifndef __server_h
#define __server_h

#define forever while(1)

typedef struct {
    int  type;
    char *value;
    int length;
} REQUEST;

typedef struct {
    char *header;
    char *filename, *filepath;
    int  error;
} RESPONSE;

#define REQUEST_SIZE 4096

#undef DELETE
enum response_types { RQ_UNDEF,GET,POST,PUT } ;

extern const char *DEFAULT_ERROR_404;

extern char *get_header(RESPONSE *);
extern REQUEST *GetRequest(int sock);
extern int SendResponse(int sock, RESPONSE *response, bool exitAfterSentSingleFile);
extern RESPONSE *GetResponse(REQUEST *);
extern void error_live(const char *);
extern void error_die(const char *);
extern int mainHTTPServer(int argc, char **argv);

#endif

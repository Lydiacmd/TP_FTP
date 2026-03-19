#ifndef COMMON_H
#define COMMON_H
#define SERVER_DIR "./server_files/"
#define CLIENT_DIR "./client_files/"
#define MAX_FILENAME 256


typedef enum {
    GET = 0,
    PUT = 1,
    LS = 2
} typereq_t;

typedef struct {
    typereq_t type;
    char nom_Fichier[MAX_FILENAME];
} request_t;



#endif

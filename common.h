#ifndef COMMON_H
#define COMMON_H
#define SERVER_DIR "./server_files/"
#define CLIENT_DIR "./client_files/"
#define MAX_FILENAME 256
#define BLOCK_SIZE 4096
#define TRANSFER_TMP "./client_files/transfer.tmp" //pour sauv ou on est arrive en cas de crash
#define NB_SLAVES 2
#define SLAVE_BASE_PORT 2122   // escalve 0 = 2122, esclave 1 = 2123 ... ect 

typedef enum {
    GET = 0,
    PUT = 1,
    LS = 2
} typereq_t;

typedef struct {
    typereq_t type;
    char nom_Fichier[MAX_FILENAME];
    long offset;  // 0 = depuis le début, sinon reprendre depuis la
} request_t;

typedef struct {
    int code;  // 0 = succes, -1 = erreur
    long filesize;
} response_t;

typedef struct {
    char ip[INET_ADDRSTRLEN]; // cte : taille max d'une adresse IPv4
    int port;
} slave_info_t;

#endif

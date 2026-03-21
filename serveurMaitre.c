#include "csapp.h"
#include "common.h"

#define MAX_NAME_LEN 256
#define MASTER_PORT 2121


int main(int argc, char **argv) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <ip_esclaves>\n", argv[0]);
        exit(1);
    }

    char *ip = argv[1];

    // Q12 -> connexion aux esclave et stoker leur infos 
    slave_info_t slaves[NB_SLAVES];
    for (int i = 0; i < NB_SLAVES; i++) {
        int port = SLAVE_BASE_PORT + i;   // debut 2122
        int fd = Open_clientfd(ip, port); // verifier qu'il est up
        Close(fd);
        strncpy(slaves[i].ip, ip, INET_ADDRSTRLEN);
        slaves[i].port = port;
        printf("Esclave %d connecté sur %s:%d\n", i, ip, port);
    }


    // Q13 -> ecoute des clients et redirections
    int listenfd = Open_listenfd(MASTER_PORT);
    printf("Maître en écoute sur le port %d\n", MASTER_PORT);


    int tour = 0; // round-robin
    while (1) {
        socklen_t clientlen;
        struct sockaddr_in clientaddr;
        clientlen = sizeof(clientaddr);

        int connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        // envoyer les infos de l'esclave choisi
        Rio_writen(connfd, &slaves[tour], sizeof(slave_info_t));
        printf("Client redirigé vers esclave %d (port %d)\n", tour, slaves[tour].port);

        tour = (tour + 1) % NB_SLAVES; // round-robin : boucle indefiniment entre 0 et NB_SLAVE-1 a 3 escalve 0->1->2->0...ect
        Close(connfd);
    }

}
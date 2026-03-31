#include "csapp.h"
#include "common.h"

#define MAX_NAME_LEN 256
#define MASTER_PORT 2121

// AJOUT : handler SIGINT pour arrêt propre
void handle_sigint(int sig) {
    printf("\nArrêt propre du maître.\n");
    exit(0);
}

int main(int argc, char **argv) {

    if (argc < 2) {
        fprintf(stderr, "Usage: %s <ip_esclaves>\n", argv[0]);
        exit(1);
    }

    char *ip = argv[1];

    // Q12 -> connexion aux esclave et stoker leur infos 
    slave_info_t slaves[NB_SLAVES];
    for (int i = 0; i < NB_SLAVES; i++) {
        int port = SLAVE_BASE_PORT + i;
        int fd = Open_clientfd(ip, port);
        Close(fd);
        strncpy(slaves[i].ip, ip, INET_ADDRSTRLEN);
        slaves[i].port = port;
        printf("Esclave %d connecté sur %s:%d\n", i, ip, port);
    }

    // Q13 -> ecoute des clients et redirections
    int listenfd = Open_listenfd(MASTER_PORT);
    printf("Maître en écoute sur le port %d\n", MASTER_PORT);

    // AJOUT : enregistrement du handler SIGINT
    signal(SIGINT, handle_sigint);

    int tour = 0;
    while (1) {
        socklen_t clientlen;
        struct sockaddr_in clientaddr;
        clientlen = sizeof(clientaddr);

        int connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

        // Q14 bonus : trouver un esclave up en round-robin
        int trouve = 0;
        for (int i = 0; i < NB_SLAVES; i++) {
            int idx = (tour + i) % NB_SLAVES;
            int testfd = open_clientfd(slaves[idx].ip, slaves[idx].port); // minuscule = pas de exit
            if (testfd >= 0) {
                Close(testfd);
                Rio_writen(connfd, &slaves[idx], sizeof(slave_info_t));
                printf("Client redirigé vers esclave %d (port %d)\n", idx, slaves[idx].port);
                tour = (idx + 1) % NB_SLAVES;
                trouve = 1;
                break;
            }
            printf("Esclave %d down, on essaie le suivant\n", idx);
        }

        if (!trouve) {
            printf("Aucun esclave disponible !\n");
            slave_info_t vide = {"", -1};
            Rio_writen(connfd, &vide, sizeof(slave_info_t));
        }

        Close(connfd);
    }
}
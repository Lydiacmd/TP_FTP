/*
 * echoserveri.c - An iterative echo server
 */

#include "csapp.h"
#include "common.h"

#define MAX_NAME_LEN 256
#define NB_PROC 10
#define PORT 2121

void transfert(int connfd){
    request_t req;
    response_t rep;
    while (1) {
    // Recevoir la requete
    int n = Rio_readn(connfd, &req, sizeof(request_t));
    if (n <= 0) break; // client déconnecté (bye)
    if (req.type == GET){
        // le chemin
        char chemin[MAX_NAME_LEN];
        // construit une chaine de caracteres dans le chemin
        snprintf(chemin, MAX_NAME_LEN, "%s%s", SERVER_DIR, req.nom_Fichier);

        // Ouvrire le fichier
        int fd = open(chemin, O_RDONLY);

        if (fd < 0){
            // fichier introuvable
            rep.code = -1;
            Rio_writen(connfd, &rep, sizeof(response_t));
            continue;
        }
        // sauter les octets déjà reçus par le client
        lseek(fd, req.offset, SEEK_SET);

        // Obtenir la taille du fichier
        struct stat st;
        stat(chemin,&st);
        long filesize = st.st_size- req.offset;

        // fichier trouve -> envoyer
        rep.code =0;
        rep.filesize = filesize;
        // envois exactement N bytes sur la socket (garentit tout les byte bien envoyer )
        Rio_writen(connfd, &rep, sizeof(response_t));

        // charger tout le fichier en mémoire et envoyer
       /* char *buf = malloc(filesize);
        read(fd, buf, filesize);
        Rio_writen(connfd, buf, filesize);

        free(buf);*/
        char buf[BLOCK_SIZE];
        long restant = filesize;

        while (restant > 0) {
            long a_lire = (restant < BLOCK_SIZE) ? restant : BLOCK_SIZE; // lire par bloc de 4096 bytes sauf si le restant est plus petit que 4096

            read(fd, buf, a_lire);
            Rio_writen(connfd, buf, a_lire);
            restant -= a_lire;
        }
        close(fd);
    }else {
        // type invalide
        rep.code = -1;
        Rio_writen(connfd, &rep, sizeof(response_t));
    }
}
}
/*
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */

int pids[NB_PROC] = {0};


void handler_sigint(int sig){

    for (int i = 0; i < NB_PROC; i++) {
        if (pids[i] > 0)
            Kill(pids[i], SIGINT);
    }
    while(waitpid(-1, NULL, 0) > 0);
    exit(0);


}

int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];
    pid_t pid;

    Signal(SIGINT, handler_sigint);


    clientlen = (socklen_t)sizeof(clientaddr);

    // Cree un socket -> Associe l'adr + port et se met an ecoute att une connexion
    listenfd = Open_listenfd(PORT);

    for(int i = 0; i< NB_PROC ; i++){
        pids[i] = Fork();
        pid = pids[i];
        // si le fils ne fork pas
        if (pids[i]==0){
            break; // sortie de boucle
        }
    }

    if (pid == 0){  // le fils

        // quand il recois un SIGINT il termine simplement evite les boucle infinie
        Signal(SIGINT, SIG_DFL);

        // La boule êrmet de servir les clients un par un, indefinimen
        // ici pas de close(listenfd) il doit etre enlevé dans le fils car des le pool les fils ont besoin de
        //listenfd pour fait accept a chaque tour

        while(1){

        // Accepte une connexion
        connfd = Accept(listenfd, (SA *)&clientaddr, &clientlen);

            /* determine the name of the client */
        Getnameinfo((SA *) &clientaddr, clientlen,
                    client_hostname, MAX_NAME_LEN, 0, 0, 0);

        /* determine the textual representation of the client's IP address */
        Inet_ntop(AF_INET, &clientaddr.sin_addr, client_ip_string,
                INET_ADDRSTRLEN);

        printf("server connected to %s (%s)\n", client_hostname,
            client_ip_string);

        // on le sers
        transfert(connfd);

        close(connfd);


        }

    }else {
        while(waitpid(-1,NULL,0)>0);
    }
}
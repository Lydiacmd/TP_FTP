#include "csapp.h"
#include "common.h"
#include <sys/prctl.h>  // AJOUT : pour prctl

#define MAX_NAME_LEN 256
#define NB_PROC 10
#define PORT 2121

void transfert(int connfd){
    request_t req;
    response_t rep;
    int authentifie = 0;


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

        char buf[BLOCK_SIZE];
        long restant = filesize;

        while (restant > 0) {
            long a_lire = (restant < BLOCK_SIZE) ? restant : BLOCK_SIZE; // lire par bloc de 4096 bytes sauf si le restant est plus petit que 4096

            read(fd, buf, a_lire);
            Rio_writen(connfd, buf, a_lire);
            restant -= a_lire;
        }
        close(fd);
    }else if (req.type == LS){
        FILE *fp = popen("ls " SERVER_DIR, "r");
        if (fp == NULL) {
            rep.code = -1;
            Rio_writen(connfd, &rep, sizeof(response_t));
            continue;
        }
        char result[4096*10];//blocksize * 10 pour stocker le resultat de ls
        size_t total_size = 0;
        int n;

        while((n=fread(result+total_size, 1, sizeof(result)-total_size, fp)) > 0) {
            total_size += n;
        }
        pclose(fp);
        rep.code = 0;
        rep.filesize = total_size;
        Rio_writen(connfd, &rep, sizeof(response_t));
        Rio_writen(connfd, result, total_size);
    } else if (req.type == RM) {
        if (!authentifie) {
        rep.code = -2;  // code spécial = non autorisé
        Rio_writen(connfd, &rep, sizeof(response_t));
        continue;
    }
    char chemin[MAX_NAME_LEN];
    snprintf(chemin, MAX_NAME_LEN, "%s%s", SERVER_DIR, req.nom_Fichier);

    if (remove(chemin) == 0) {
        rep.code = 0;
    } else {
        rep.code = -1;
    }
    Rio_writen(connfd, &rep, sizeof(response_t));
}   else if (req.type == PUT) {
        if (!authentifie) {
        rep.code = -2;  // code spécial = non autorisé
        Rio_writen(connfd, &rep, sizeof(response_t));
        continue;
    }
        // ouvrir le fichier
        char chemin[MAX_NAME_LEN];
        snprintf(chemin, MAX_NAME_LEN, "%s%s", SERVER_DIR, req.nom_Fichier);

        int fd = open(chemin, O_WRONLY | O_CREAT | O_TRUNC, 0644);
        if (fd < 0) {
            rep.code = -1;
            Rio_writen(connfd, &rep, sizeof(response_t));
            continue;
        }

        // recevoir le fichier par bloc
        char buf[BLOCK_SIZE];
        long restant = req.filesize;

        while (restant > 0) {
            long a_lire = (restant < BLOCK_SIZE) ? restant : BLOCK_SIZE;
            Rio_readn(connfd, buf, a_lire);
            write(fd, buf, a_lire);
            restant -= a_lire;
        }
        close(fd);

        // 3. envoyer confirmation
        rep.code = 0;
        Rio_writen(connfd, &rep, sizeof(response_t));
    }    else if (req.type == LOGIN) {
            if (strcmp(req.login, AUTH_USER) == 0 &&
                strcmp(req.password, AUTH_PASS) == 0) {
                authentifie = 1;
                rep.code = 0;
                printf("Client authentifié\n");
            } else {
                rep.code = -1;
            }
            Rio_writen(connfd, &rep, sizeof(response_t));
        }
else {
        // type invalide
        rep.code = -1;
        Rio_writen(connfd, &rep, sizeof(response_t));
    }
}
}

int pids[NB_PROC] = {0};

void handler_sigint(int sig){
    for (int i = 0; i < NB_PROC; i++) {
        if (pids[i] > 0)
            kill(pids[i], SIGKILL);  // tue chaque fils directement par PID
    }
    _exit(0);  // _exit et pas exit, plus safe depuis un handler
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
    Signal(SIGTERM, handler_sigint);

    if (argc < 2) {
    fprintf(stderr, "Usage: %s <port>\n", argv[0]);
    exit(1);
    }

    int port = atoi(argv[1]);


    clientlen = (socklen_t)sizeof(clientaddr);

    // Cree un socket -> Associe l'adr + port et se met an ecoute att une connexion
    listenfd = Open_listenfd(port);

    for(int i = 0; i< NB_PROC ; i++){
        pids[i] = Fork();
        pid = pids[i];
        // si le fils ne fork pas
        if (pids[i]==0){
            break; // sortie de boucle
        }
    }

    if (pid == 0){  // le fils

        prctl(PR_SET_PDEATHSIG, SIGKILL);  // AJOUT : meurt instantanément si le parent meurt
        // quand il recois un SIGINT il termine simplement evite les boucle infinie
        Signal(SIGINT, SIG_DFL);
        Signal(SIGTERM, SIG_DFL);  // AJOUT : ne pas propager le handler aux fils

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
#include "csapp.h"
#include "common.h"
#include <time.h>
#define MAX_NAME_LEN 256
#define NB_PROC 10
#define PORT 2121

int main(int argc, char **argv) {

    if (argc < 2){
        fprintf(stderr,"Pas assez d'arguments\n");
        exit(1);
    }

    char *serveur = argv[1];        // Recupere le serveur 
    char cmd[MAX_NAME_LEN];
    char nom_fichier[MAX_NAME_LEN];

    // cree une sockets coté client 
    /** Open_client fait 
     * socket() → crée la socket
     * getaddrinfo() → résout le nom du serveur en adresse IP
     * connect() → se connecte au serveur
     */
    int clientfd = Open_clientfd(serveur,PORT);
    printf("Connected to %s\n",serveur);

    // Debut du Prompt interactifs 
    printf("FTP >> ");

    // lis depuis stdin -> tourne tanq l'utilisateur tape des cmds -> arrete avec CTRL+D (fin fichier)
    while(fgets(cmd,MAX_NAME_LEN,stdin) != NULL){   
        // parser "get nom_fichier"
        if (sscanf(cmd,"get %s",nom_fichier)==1){
            
            // 1. construire et envoye la requete 
            request_t req;
            req.type = GET;
            strncpy(req.nom_Fichier, nom_fichier, MAX_FILENAME);
            Rio_writen(clientfd, &req, sizeof(request_t));

            // 2. recevoir la reponse 
            response_t rep;
            Rio_readn(clientfd, &rep, sizeof(response_t));

            if (rep.code == -1) {
                fprintf(stderr, "Erreur : fichier introuvable sur le serveur\n");
            } else {
                // 3. recevoir le fichier et le sauvegarder
                char chemin[MAX_NAME_LEN];
                snprintf(chemin, MAX_NAME_LEN, "%s%s", CLIENT_DIR, nom_fichier);

                int fd = open(chemin, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                char *buf = malloc(rep.filesize);
                
                clock_t debut = clock();
                Rio_readn(clientfd, buf, rep.filesize);
                write(fd, buf, rep.filesize);
                clock_t fin = clock();

                free(buf);
                close(fd);

                // 4. afficher les stats
                double temps = (double)(fin - debut) / CLOCKS_PER_SEC;
                printf("Transfer successfully complete.\n");
                printf("%ld bytes received in %.2f seconds (%.2f Kbytes/s)\n",
                    rep.filesize, temps, (rep.filesize / 1024.0) / temps);
            }
        }
        else {
            fprintf(stderr,"cmd inconnue");
        }
        printf("FTP >> ");

    }

    Close(clientfd);
    return 0;

}
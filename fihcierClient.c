#include "csapp.h"
#include "common.h"

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
            //  a faire apres 
        }
        else {
            fprintf(stderr,"cmd inconnue");
        }
        printf("FTP >> ");

    }

    Close(clientfd);
    return 0;

}
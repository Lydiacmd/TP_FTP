#include "csapp.h"
#include "common.h"
#include <time.h>
#define MAX_NAME_LEN 256
#define NB_PROC 10
#define PORT 2121

int main(int argc, char **argv) {

    FILE *tmp = fopen(TRANSFER_TMP, "r");
    if (tmp != NULL) {
        // le fichier existe donc transfert interrompu
        char nom[MAX_FILENAME];
        long deja_recu;
        fscanf(tmp, "%s %ld", nom, &deja_recu);
        fclose(tmp);
        printf("Transfert interrompu détecté : %s (%ld octets déjà reçus)\n", nom, deja_recu);
    }

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
    //1 . connexion au maitre 
    int masterfd = Open_clientfd(serveur,PORT);

    //2. Recevoir les infos de l'esclave 
    slave_info_t slave;
    Rio_readn(masterfd, &slave, sizeof(slave_info_t));
    Close(masterfd);
    printf("Redirige vers esclave %s:%d\n", slave.ip, slave.port);

    //3. connexion a l'esclave 
    int clientfd = Open_clientfd(slave.ip, slave.port);
    printf("Connected to %s\n", serveur);

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

            req.offset = 0;  // par défaut

            // vérifier si transfert interrompu pour CE fichier
            FILE *tmp = fopen(TRANSFER_TMP, "r");
            if (tmp != NULL) {
                char nom_sauve[MAX_FILENAME];
                long deja_recu;
                fscanf(tmp, "%s %ld", nom_sauve, &deja_recu);
                fclose(tmp);

                if (strcmp(nom_sauve, nom_fichier) == 0) {
                    // même fichier -> reprendre !
                    req.offset = deja_recu;
                }
            }

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
                int flags = (req.offset == 0) ? (O_WRONLY | O_CREAT | O_TRUNC) : (O_WRONLY | O_APPEND);
                int fd = open(chemin, flags, 0644);
                // int fd = open(chemin, O_WRONLY | O_CREAT | O_TRUNC, 0644);

                /*char *buf = malloc(rep.filesize);

                clock_t debut = clock();
                Rio_readn(clientfd, buf, rep.filesize);
                write(fd, buf, rep.filesize);
                clock_t fin = clock();

                free(buf);*/
//-----------------------------------------------------------------------
                // Sauvegarder l'état du transfert
                FILE *tmp = fopen(TRANSFER_TMP, "w");
                fprintf(tmp, "%s %ld", nom_fichier, 0L);
                fclose(tmp);
//-----------------------------------------------------------------------


                char buf[BLOCK_SIZE];
                long restant = rep.filesize;
                clock_t debut = clock();
                while (restant > 0) {
                    long a_ecrir = (restant < BLOCK_SIZE) ? restant : BLOCK_SIZE;
                    Rio_readn(clientfd, buf, a_ecrir);
                    write(fd, buf, a_ecrir);
                    restant -= a_ecrir;

                    tmp = fopen(TRANSFER_TMP, "w");
                    fprintf(tmp, "%s %ld", nom_fichier, rep.filesize - restant);
                    fclose(tmp);
                }
                clock_t fin = clock();
                close(fd);
                remove(TRANSFER_TMP);// supprimer le fichier temporaire de sauvegarde une fois le transfert terminé
                // 4. afficher les stats
                double temps = (double)(fin - debut) / CLOCKS_PER_SEC;
                printf("Transfer successfully complete.\n");
                printf("%ld bytes received in %.2f seconds (%.2f Kbytes/s)\n",
                    rep.filesize, temps, (rep.filesize / 1024.0) / temps);
            }
        }

        else if (strncmp(cmd, "bye", 3) == 0) {
    printf("ciao azul !\n");// a changer
    break;
}
        else {
            fprintf(stderr,"cmd inconnue");
        }
        printf("FTP >> ");
    }

    Close(clientfd);
    return 0;

}
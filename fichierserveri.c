/*
 * echoserveri.c - An iterative echo server
 */

#include "csapp.h"
#include "common.h"

#define MAX_NAME_LEN 256
#define NB_PROC 10
#define PORT 2121

void transfert(int connfd);
/* 
 * Note that this code only works with IPv4 addresses
 * (IPv6 is not supported)
 */


int main(int argc, char **argv)
{
    int listenfd, connfd;
    socklen_t clientlen;
    struct sockaddr_in clientaddr;
    char client_ip_string[INET_ADDRSTRLEN];
    char client_hostname[MAX_NAME_LEN];
    pid_t pid;
    int pids[NB_PROC];
    
    
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
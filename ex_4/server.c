#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>

#define BUF_LEN 102411
#define REQUEST_LEN 4  // REQ\012

int main(int argc, char* argv[]){

    int ret, newfd, listener, addrlen, i, len; 

    // Set di descrittori da monitorare    
    fd_set master;                
    
    // Set dei descrittori pronti
    fd_set read_fds;             
    
    // Descrittore max 
    int fdmax;                    
    
    struct sockaddr_in my_addr, cl_addr;
    char buffer[BUF_LEN];
    
    // Uso la struttura time_t per l'orario
    time_t rawtime;
    struct tm* timeinfo;
    
    /* Creazione socket */
    listener = socket(AF_INET, SOCK_STREAM, 0);

    /* Creazione indirizzo di bind */
    memset(&my_addr, 0, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(4242);
    my_addr.sin_addr.s_addr = INADDR_ANY;
    
    /* Aggancio */
    ret = bind(listener, (struct sockaddr*)&my_addr, sizeof(my_addr));
    if(ret < 0){
        perror("Bind non riuscita\n");
        exit(0);
    }
    
    /* Apro la coda */
    listen(listener, 10);
    
    // Reset dei descrittori
    FD_ZERO(&master);
    FD_ZERO(&read_fds);

    // Aggiungo il socket di ascolto 'listener' ai socket monitorati
    FD_SET(listener, &master); 
    // Tengo traccia del nuovo fdmax
    fdmax = listener;          
    
    while(1){
        
        // Imposto il set di socket da monitorare in lettura per la select()
        // NOTA: select() modifica il set 'read_fds' lasciando solo i descrittori pronti.
        // ma non modifica il set 'master' dei descrittori monitorati!
        
        read_fds = master;     
        
        // Mi blocco (potenzialmente) in attesa di descrittori pronti
        // Attesa ***senza timeout*** (ultimo parametro attuale 'NULL')
        
        select(fdmax+1, &read_fds, NULL, NULL, NULL);
        
        // Scorro ogni descrittore 'i'
        for(i=0; i<=fdmax; i++) {  
                        
            // Se il descrittore 'i' è rimasto nel set 'read_fds', cioè se la select() 
            // ce lo ha lasciato, allora 'i' è pronto
            
            if(FD_ISSET(i, &read_fds)) { 
            
                // Se il descrittore pronto 'i' è il listening socket 'listener'
                // ho ricevuto una richiesta di connessione
            
                if(i == listener) { 
                   
                    // Calcolo la lunghezza dell'indirizzo del client
                    addrlen = sizeof(cl_addr);
                    
                    // Accetto la connessione e creo il socket connesso ('newfd')
                    newfd = accept(listener, (struct sockaddr *)&cl_addr, &addrlen);
                    
                    // Aggiungo il socket connesso al set dei descrittori monitorati
                    FD_SET(newfd, &master); 
                    
                    // Aggiorno l'ID del massimo descrittore
                    if(newfd > fdmax){ 
                        fdmax = newfd; 
                    } 
                    
                } 
                // Altrimenti, ho ricevuto una richiesta di servizio (orario)
                else {

                    // Ricevo il messaggio di richiesta (so che è di 4 byte)
                    ret = recv(i, (void*)buffer, REQUEST_LEN, 0);

                    // Recupero l'ora corrente1
                    time(&rawtime);

                    // Converto l'ora1
                    timeinfo = localtime(&rawtime);

                    // Creo la risposta nel formato 'hh:mm:ss'1
                    sprintf(buffer, "%d:%d:%d",
                            timeinfo->tm_hour, timeinfo->tm_min, timeinfo->tm_sec);

                    // Invio la risposta, compreso '\0'
                    ret = send(i, (void*) buffer, strlen(buffer)+1, 0);
                    if(ret < 0){
                        perror("Errore in fase di comunicazione con il client: \n");
                    }

                    // Chiudo il socket connesso 'i', che ho servito1
                    close(i);
                    // Rimuovo il socket 'i' dal set dei socket monitorati
                    FD_CLR(i, &master);

                }
            }
        } // fine for che scorre i descrittori (torno alla linea 75)
    } // fine while(1)

    close(listener);
}
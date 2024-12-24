#include <stdlib.h>
#include <unistd.h>

#include "pbx.h"
#include "server.h"
#include "debug.h"
#include "csapp.h"

static void terminate(int status);

/*
 * "PBX" telephone exchange simulation.
 *
 * Usage: pbx <port>
 */ 

//Signal Handling (Sighup_handler and volatile flag!)
volatile int sighup_recieved = 0; 
void sighup_handler(){
    sighup_recieved = 1; //Will be used to tell us if we've recieved sighup so we can terminate! 
}
int main(int argc, char* argv[]){
    // Option processing should be performed here.
    // Option '-p <port>' is required in order to specify the port number
    // on which the server should listen. 

    //For this portion we will be running getopt in order to get the port number! 
    char* PORT = NULL;  
    int cli; 

    while((cli = getopt(argc, argv, "p:"))!= -1){
        switch(cli){
            case 'p':  
                PORT = optarg; 
                break; 
            default: 
                fprintf(stderr, "PORT WAS NOT GIVEN, Usage: %s, -p <port>\n", argv[0]); 
                exit(EXIT_FAILURE); 
        }
    } 
    if(PORT == NULL){
        fprintf(stderr, "PORT WAS NOT GIVEN, Usage: %s, -p <port>\n", argv[0]); 
        exit(EXIT_FAILURE); 
    } 

    sigset_t mask; 
    sigemptyset(&mask); 
    sigaddset(&mask, SIGHUP); 
    sigaddset(&mask, SIGINT); 

    struct sigaction handle_sighup; 
    handle_sighup.sa_handler = sighup_handler;  
    sigemptyset(&handle_sighup.sa_mask);  
    handle_sighup.sa_flags = 0;  

    if(sigaction(SIGHUP, &handle_sighup, NULL) < 0){
        fprintf(stderr, "SIGACTION FAILED");  
        exit(EXIT_FAILURE); 
    } 
    // Perform required initialization of the PBX module.
    debug("Initializing PBX...");
    pbx = pbx_init();

    // TODO: Set up the server socket and enter a loop to accept connections
    // on this socket.  For each connection, a thread should be started to
    // run function pbx_client_service().  In addition, you should install
    // a SIGHUP handler, so that receipt of SIGHUP will perform a clean
    // shutdown of the server.   

    //ADDING COMMENT TO TEST YML FILE CHANGE! 
    //TextBook Code 
    int * connfdp; 
    socklen_t clientlen;  
    struct sockaddr_storage clientaddr; 
    pthread_t tid;  

    int listenfd = open_listenfd(PORT); 
    if(listenfd < 0){
        fprintf(stderr, "ERROR OPENING LISTENFD"); 
        terminate(EXIT_FAILURE); 
    } 
    volatile sig_atomic_t run = 1; 
    while(run){
        clientlen = sizeof(struct sockaddr_storage); 
        connfdp = malloc(sizeof(int)); 
        if(connfdp < 0){
            fprintf(stderr, "Error mallocing space for connfdp");  
            continue; 
        } 
        
        if(sighup_recieved){
            //We recieved the flag needed to terminate, so let's terminate cleanly!  
            run = 0; 
            free(connfdp); 
            close(listenfd); 
            terminate(EXIT_SUCCESS); 
            break; 
        } 

        if ((*connfdp = accept(listenfd, (struct sockaddr *)&clientaddr, &clientlen)) < 0){
            //fprintf(stderr, "ERROR accepting new connection!"); 
            free(connfdp); 
            continue; 
        } 

        if(pthread_create(&tid, NULL, pbx_client_service,connfdp) < 0){
            free(connfdp); 
            continue; 
            //exit(EXIT_FAILURE); 
        } 
    }
    // fprintf(stderr, "You have to finish implementing main() "
	//     "before the PBX server will function.\n");
    terminate(EXIT_FAILURE);
}

/*
 * Function called to cleanly shut down the server.
 */
static void terminate(int status) {
    debug("Shutting down PBX...");
    pbx_shutdown(pbx);
    debug("PBX server terminating");
    exit(status);
}

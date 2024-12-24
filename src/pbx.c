// /*
//  * PBX: simulates a Private Branch Exchange.
//  */
// #include <stdlib.h>

#include "pbx.h"
#include "debug.h"
#include <semaphore.h> 
#include <sys/socket.h>  
#include <stdlib.h>
/*
 * Initialize a new PBX.
 *
 * @return the newly initialized PBX, or NULL if initialization fails.
 */
// #if 0 
/*HW DOC COMMENTS: 
Has to maintain registry of options! Needs to map each extension number to TU Object
it will need to provide appropriate synchronization (for example, using mutexes and/or
semaphores) to ensure correct and reliable operation.
Finally, the pbx_shutdown() function is required to shut down the network connections
to all registered clients (the shutdown(2) function can be used to shut down a socket
for reading, writing, or both, without closing the associated file descriptor)
and it is then required to wait for all the client service threads to unregister
the associated TUs before returning. Consider using a semaphore, possibly in conjunction
with additional bookkeeping variables, for this purpose.*/ 

//First let's create our pbx struct! (We will be using nodes to keep track of our TUs!) 
typedef struct pbx_node {
    TU* telephone; 
    struct pbx_node* next; 
    struct pbx_node* prev; 
}PBX_NODE;  

struct pbx{
    PBX_NODE* head; //Head for Linked List Containing All Registered Clients/Telephones!
    sem_t mutex; //mutex for locking! 
    int num_extensions; //Keeping Track of Number of Extensions!    
}; 
/*
 * Initialize a new PBX.
 *
 * @return the newly initialized PBX, or NULL if initialization fails.
 */
PBX *pbx_init() { 
    //First we want to malloc space for our PBX 
    PBX* created_pbx = malloc(sizeof(PBX));  
    if(created_pbx == NULL){return NULL;} 
    //Now we can start initalizing the fields and the mutex! 
    created_pbx->head = NULL; 
    created_pbx->num_extensions = 0;  

    if(sem_init(&created_pbx->mutex, 0, 1) != 0){
        //We failed to initalize our mutext so we should free our pbx and return NULL 
        free(created_pbx); 
        return NULL; 
    } 
    return created_pbx; 
}
// // #endif

/*
 * Shut down a pbx, shutting down all network connections, waiting for all server
 * threads to terminate, and freeing all associated resources.
 * If there are any registered extensions, the associated network connections are
 * shut down, which will cause the server threads to terminate.
 * Once all the server threads have terminated, any remaining resources associated
 * with the PBX are freed.  The PBX object itself is freed, and should not be used again.
 *
 * @param pbx  The PBX to be shut down.
 */

void pbx_shutdown(PBX *pbx) {
    // TO BE IMPLEMENTED 
    if(pbx == NULL){
        return; 
    } 
    //We need to lock now as we are ediitng the pbx's! 
    sem_wait(&pbx->mutex); //sem_wait is lock, sem_post is unlock! 
    PBX_NODE* current = pbx->head; 
    while(current != NULL){
        PBX_NODE* next = current->next; 
        //sem_post(&pbx->mutex);
        //We need to shutdown each registered extensions which is in this doubly linked list 
        if(shutdown(tu_fileno(current->telephone), SHUT_RDWR) == -1){
            //fprintf(stderr, "ERROR SHUTTING DOWN A REGISTERED TELEPHONE");  
            //continue; 
        } 
        // sem_post(&pbx->mutex);
        //pbx_unregister(pbx, current->telephone);  --> DONT NEED PBX_UNREGISTER HERE! 
        //sem_wait(&pbx->mutex);  

        current = next; 
    } 
    sem_post(&pbx->mutex); 
    sem_destroy(&pbx->mutex);  
    free(pbx); 
    // abort();
}
// #endif

/*
 * Register a telephone unit with a PBX at a specified extension number.
 * This amounts to "plugging a telephone unit into the PBX".
 * The TU is initialized to the TU_ON_HOOK state.
 * The reference count of the TU is increased and the PBX retains this reference
 *for as long as the TU remains registered.
 * A notification of the assigned extension number is sent to the underlying network
 * client.
 *
 * @param pbx  The PBX registry.
 * @param tu  The TU to be registered.
 * @param ext  The extension number on which the TU is to be registered.
 * @return 0 if registration succeeds, otherwise -1.
 */
// #if 0
int pbx_register(PBX *pbx, TU *tu, int ext) {
    // TO BE IMPLEMENTED 
    if(pbx == NULL){
        return -1; 
    } 
    if(tu == NULL){
        return -1; 
    } 
    if(pbx->num_extensions >= PBX_MAX_EXTENSIONS){
        return -1; 
    } 

    PBX_NODE* node = malloc(sizeof(PBX_NODE)); 
    if(node == NULL){
        //sem_post(&pbx->mutex);  
        return -1; 
    } 
    sem_wait(&pbx->mutex); //LOCK Since we are about to edit the critical doubly linked list! 
    node->next = pbx->head; //Adding to front of list!   
    node->prev = NULL; 
    node->telephone = tu;
    if(pbx->head != NULL){
        pbx->head->prev = node; 
    }
    pbx->head = node;  
    pbx->num_extensions++; 
    sem_post(&pbx->mutex); //UNLOCK!

    //Now we need to just set the extension and make sure to increase the refernce of the TU  
    //We will need to lock in tu_ref for this I THINK! -> THIS WAS WRONG, WE DO NOT NEED TO LOCK IN TU_REF
    //tu_ref(tu, "REGISTERING TU!"); 
    tu_set_extension(tu, ext); 

    // pbx->num_extensions++; 
    // sem_post(&pbx->mutex); //UNLOCK!
    return 0; 

    //abort();
}
// // #endif

/*
 * Unregister a TU from a PBX.
 * This amounts to "unplugging a telephone unit from the PBX".
 * The TU is disassociated from its extension number.
 * Then a hangup operation is performed on the TU to cancel any
 * call that might be in progress.
 * Finally, the reference held by the PBX to the TU is released.
 *
 * @param pbx  The PBX.
 * @param tu  The TU to be unregistered.
 * @return 0 if unregistration succeeds, otherwise -1.
 */
// #if 0
int pbx_unregister(PBX *pbx, TU *tu) {
    // TO BE IMPLEMENTED 
    //Should be the opposite of register essentially so let's do the same error checking! 
     if(pbx == NULL){
        return -1; 
    } 
    if(tu == NULL){
        return -1; 
    } 
    sem_wait(&pbx->mutex); 

    PBX_NODE* current = pbx->head; 
    while(current != NULL){
        if(current->telephone == tu){
            //We hangup and we decrement the reference!  

            //Updating Linked List since we are freeing this telephone! 
            if(current->prev != NULL){ 
                current->prev->next = current->next; 
            }else{
                pbx->head = current->next; 
            } 
            if(current->next != NULL){
                current->next->prev = current->prev; 
            } 
            pbx->num_extensions--;  
            PBX_NODE* freeing = current;  

            sem_post(&pbx->mutex);   

            tu_hangup(tu); 
            tu_unref(tu, "UNREGISTERING PHONE!");  

            free(freeing);  
            return 0; 
        } 
        current = current->next;
    } 
    sem_post(&pbx->mutex);  
    return -1; 
}
// #endif

/*
 * Use the PBX to initiate a call from a specified TU to a specified extension.
 *
 * @param pbx  The PBX registry.
 * @param tu  The TU that is initiating the call.
 * @param ext  The extension number to be called.
 * @return 0 if dialing succeeds, otherwise -1.
 */
// #if 0
int pbx_dial(PBX *pbx, TU *tu, int ext) {
    // TO BE IMPLEMENTED 
    if(pbx == NULL){
        return -1; 
    } 
    if(tu == NULL){
        return -1; 
    } 

    sem_wait(&pbx->mutex); 

    PBX_NODE* current = pbx->head; 
    while(current != NULL){
        if(tu_extension(current->telephone) == ext){ 
            sem_post(&pbx->mutex);  
            tu_dial(tu, current->telephone); 
            return 0; 
        } 
        current = current->next;
    } 
    sem_post(&pbx->mutex);  
    tu_dial(tu, NULL);  
    /*according to TU DIAL SPECIFICATIONS ->  If the caller of this function was unable to determine a target TU 
    to be called, it will pass NULL as the target TU*/
    return -1; 
}
// #endif
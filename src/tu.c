/*
 * TU: simulates a "telephone unit", which interfaces a client with the PBX.
 */
#include <stdlib.h>

#include "pbx.h"
#include "debug.h"
#include <semaphore.h> 
#include <sys/socket.h>  
#include <stdlib.h>
/*
 * Initialize a TU
 *
 * @param fd  The file descriptor of the underlying network connection.
 * @return  The TU, newly initialized and in the TU_ON_HOOK state, if initialization
 * was successful, otherwise NULL.
 */ 
struct tu{ 
    TU* peer;  
    TU_STATE state; 
    sem_t mutex;   
    int fd; 
    int extension;  
    volatile int ref_count; 
};

// #if 0
TU *tu_init(int fd) {
    // TO BE IMPLEMENTED 
    TU* tu = malloc(sizeof(TU));  
    if(tu == NULL){return NULL;} 
    tu->fd = fd; 
    tu->extension = -1; 
    tu->state = TU_ON_HOOK;  
    tu->peer = NULL; 
    tu->ref_count = 0; 
    if(sem_init(&tu->mutex, 0, 1) != 0){ 
        free(tu); 
        return NULL;
    } 
    tu_ref(tu, "Intializing TU!"); 
    return tu; 
}
// #endif

/*
 * Increment the reference count on a TU.
 *
 * @param tu  The TU whose reference count is to be incremented
 * @param reason  A string describing the reason why the count is being incremented
 * (for debugging purposes).
 */
// #if 0
void tu_ref(TU *tu, char *reason) { 
    //If locking and unlocking causes problems here get rid of it!  
    if(tu == NULL) return; 
    //sem_wait(&tu->mutex); 
    //Since tu_ref count is a volatile flag we must increment it in an interesting way 
    int new_ref = tu->ref_count; 
    new_ref += 1; 
    tu->ref_count = new_ref;  
    debug("Increasing ref count because %s for TU %d (%d -> %d)", reason, tu->extension, new_ref-1, new_ref); 
    //sem_post(&tu->mutex); 
}
// #endif

/*
 * Decrement the reference count on a TU, freeing it if the count becomes 0.
 *
 * @param tu  The TU whose reference count is to be decremented
 * @param reason  A string describing the reason why the count is being decremented
 * (for debugging purposes).
 */
// #if 0
void tu_unref(TU *tu, char *reason) {  
    if(tu == NULL) return;  
    //Again we will be locking and unlocking the TU in here as this seems safest! 
    //sem_wait(&tu->mutex); 
    int new_ref = tu->ref_count; 
    new_ref -= 1; 
    tu->ref_count = new_ref; 
    debug("Decreasing ref count because %s for TU %d (%d -> %d)", reason, tu->extension, new_ref+1, new_ref); 
    if(tu->ref_count == 0){
        //sem_post(&tu->mutex); 
        if(tu->peer){
            tu->peer->peer = NULL; //To be safe, we should make sure that if this had a peer it's reference to this is gone
        }
        sem_destroy(&tu->mutex); 
        free(tu); 
        return; 
    } 
    //sem_post(&tu->mutex); 
}
// #endif

/*
 * Get the file descriptor for the network connection underlying a TU.
 * This file descriptor should only be used by a server to read input from
 * the connection.  Output to the connection must only be performed within
 * the PBX functions.
 *
 * @param tu
 * @return the underlying file descriptor, if any, otherwise -1.
 */
// #if 0
int tu_fileno(TU *tu) { 
    if(tu == NULL){
        return -1; 
    } 
    return tu->fd; 
}
// #endif

/*
 * Get the extension number for a TU.
 * This extension number is assigned by the PBX when a TU is registered
 * and it is used to identify a particular TU in calls to tu_dial().
 * The value returned might be the same as the value returned by tu_fileno(),
 * but is not necessarily so.
 *
 * @param tu
 * @return the extension number, if any, otherwise -1.
 */
// #if 0
int tu_extension(TU *tu) { 
    if(tu == NULL){return -1;} 
    if(tu->extension == -1){
        return -1; 
    } 
    return tu->extension; 
}
// #endif

/*
 * Set the extension number for a TU.
 * A notification is set to the client of the TU.
 * This function should be called at most once one any particular TU.
 *
 * @param tu  The TU whose extension is being set.
 */
// #if 0
int tu_set_extension(TU *tu, int ext) {
    // TO BE IMPLEMENTED 
    if(tu == NULL){return -1;} 
    sem_wait(&tu->mutex);  
    tu->extension = ext; 
    dprintf(tu->fd, "%s %d\n", tu_state_names[tu->state], ext); 
    sem_post(&tu->mutex); 
    return 0; 
}
// #endif

/*
 * Initiate a call from a specified originating TU to a specified target TU.
 *   If the originating TU is not in the TU_DIAL_TONE state, then there is no effect.
 *   If the target TU is the same as the originating TU, then the TU transitions
 *     to the TU_BUSY_SIGNAL state.
 *   If the target TU already has a peer, or the target TU is not in the TU_ON_HOOK
 *     state, then the originating TU transitions to the TU_BUSY_SIGNAL state.
 *   Otherwise, the originating TU and the target TU are recorded as peers of each other
 *     (this causes the reference count of each of them to be incremented),
 *     the target TU transitions to the TU_RINGING state, and the originating TU
 *     transitions to the TU_RING_BACK state.
 *
 * In all cases, a notification of the resulting state of the originating TU is sent to
 * to the associated network client.  If the target TU has changed state, then its client
 * is also notified of its new state.
 *
 * If the caller of this function was unable to determine a target TU to be called,
 * it will pass NULL as the target TU.  In this case, the originating TU will transition
 * to the TU_ERROR state if it was in the TU_DIAL_TONE state, and there will be no
 * effect otherwise.  This situation is handled here, rather than in the caller,
 * because here we have knowledge of the current TU state and we do not want to introduce
 * the possibility of transitions to a TU_ERROR state from arbitrary other states,
 * especially in states where there could be a peer TU that would have to be dealt with.
 *
 * @param tu  The originating TU.
 * @param target  The target TU, or NULL if the caller of this function was unable to
 * identify a TU to be dialed.
 * @return 0 if successful, -1 if any error occurs that results in the originating
 * TU transitioning to the TU_ERROR state. 
 */
// #if 0
int tu_dial(TU *tu, TU *target) { 
    if(tu == NULL){
        return -1; 
    }
    // TO BE IMPLEMENTED, let's handle this by cases 
    //1) If target is NULL we should transition to error! 
    //2) if target is not TU_DIAL_TONE we just print again
    //3) if TU is target then go to BUSY SIGNAL   
    //4) If the target TU already has a peer,  
    //5) or the target TU is not in the TU_ON_HOOK state, then the originating TU transitions to the TU_BUSY_SIGNAL state.
    sem_wait(&tu->mutex); 
    //Case 1:   
    if(tu->state != TU_DIAL_TONE){
        //No effect so print current state! 
        if(tu->state == TU_ON_HOOK){
            dprintf(tu->fd, "%s %d\n", tu_state_names[tu->state], tu->extension);
            sem_post(&tu->mutex); 
            return 0; 
        }
        dprintf(tu->fd, "%s\n", tu_state_names[tu->state]); 
        sem_post(&tu->mutex);  
        return 0; 
    }  
    //Case 2:
    if(target == NULL){
        tu->state = TU_ERROR; 
        dprintf(tu->fd, "%s\n", tu_state_names[TU_ERROR]); 
        sem_post(&tu->mutex); 
        return -1; 
    } 
    //Case 3: 
    sem_post(&tu->mutex); 
    if(tu == target){
        sem_wait(&tu->mutex); 
        tu->state = TU_BUSY_SIGNAL; 
        dprintf(tu->fd, "%s\n", tu_state_names[TU_BUSY_SIGNAL]); 
        sem_post(&tu->mutex); 
        return 0;
    } 
    // TU* first = (tu < target) ? tu: target; 
    // TU* second = (tu < target) ? target: tu;   
    sem_wait(&tu->mutex); 
    sem_wait(&target->mutex);   
    //Case 4:
    if(target->state != TU_ON_HOOK){
        tu->state = TU_BUSY_SIGNAL; 
        dprintf(tu->fd, "%s\n",tu_state_names[TU_BUSY_SIGNAL]);  
        sem_post(&target->mutex);
        sem_post(&tu->mutex); 
        return 0;
    }  
    //Case 5: 
    if(target->peer){
        tu->state = TU_BUSY_SIGNAL; 
        dprintf(tu->fd, "%s\n",tu_state_names[TU_BUSY_SIGNAL]);  
        sem_post(&target->mutex);
        sem_post(&tu->mutex); 
        return 0;
    } 
    //Last Case of normal ring back! 
    tu->state = TU_RING_BACK;  
    target->state = TU_RINGING; 
    tu->peer = target; 
    target->peer = tu;  
    tu_ref(tu, "Set Reference to TU From Peer when Dialing!"); 
    tu_ref(target, "Set Reference to TU From Peer when Dialing!");
    dprintf(tu->fd, "%s\n",tu_state_names[TU_RING_BACK]); 
    dprintf(target->fd, "%s\n",tu_state_names[TU_RINGING]); 
    sem_post(&target->mutex); 
    sem_post(&tu->mutex);  
    return 0; 
}
// #endif 


/*
 * Take a TU receiver off-hook (i.e. pick up the handset).
 *   If the TU is in neither the TU_ON_HOOK state nor the TU_RINGING state,
 *     then there is no effect.
 *   If the TU is in the TU_ON_HOOK state, it goes to the TU_DIAL_TONE state.
 *   If the TU was in the TU_RINGING state, it goes to the TU_CONNECTED state,
 *     reflecting an answered call.  In this case, the calling TU simultaneously
 *     also transitions to the TU_CONNECTED state.
 *
 * In all cases, a notification of the resulting state of the specified TU is sent to
 * to the associated network client.  If a peer TU has changed state, then its client
 * is also notified of its new state.
 *
 * @param tu  The TU that is to be picked up.
 * @return 0 if successful, -1 if any error occurs that results in the originating
 * TU transitioning to the TU_ERROR state. 
 */
// #if 0
int tu_pickup(TU *tu) {
    // TO BE IMPLEMENTED 
    if(tu == NULL){return -1;} 
    sem_wait(&tu->mutex); 
    //Neither Ringing or ON_HOOK we ignore 
    if(tu->state != TU_ON_HOOK && tu->state != TU_RINGING){ 
        if(tu->state == TU_CONNECTED){
            dprintf(tu->fd, "%s %d\n",tu_state_names[tu->state], tu->peer->extension);  
            sem_post(&tu->mutex);  
            return 0; 
        }
        dprintf(tu->fd, "%s\n",tu_state_names[tu->state]);  
        sem_post(&tu->mutex); 
        return 0; 
    } 
    //TU_ON_HOOK -> DIAL  
    if(tu->state == TU_ON_HOOK){
        tu->state = TU_DIAL_TONE; 
        dprintf(tu->fd, "%s\n",tu_state_names[TU_DIAL_TONE]);   
        sem_post(&tu->mutex); 
        return 0;
    }  
    // if(tu->state != TU_RINGING){
    //     dprintf(tu->fd, "%s\n",tu_state_names[tu->state]);  
    //     sem_post(&tu->mutex); 
    //     return 0; 
    // }   
    TU* peer = tu->peer;  
    if(peer == NULL){ 
        sem_post(&tu->mutex); 
        return -1; 
    }
    sem_post(&tu->mutex);
    //IF NOT TU_RINING MAKE SURE WE RETURN 0 SINCE NO EFFECT   
    // TU* first = (tu < target) ? tu: target; 
    // TU* second = (tu < target) ? target: tu;    
    sem_wait(&tu->mutex); 
    sem_wait(&peer->mutex); 
    if(tu->state != TU_RINGING || tu->peer != peer){
        //We somehow got into an error, free locks  
        sem_post(&peer->mutex); 
        sem_post(&tu->mutex); 
        return -1; 
    } 
    //Now we can deal with TU Ringing and connecting to a peer!  
    tu->state = TU_CONNECTED; 
    peer->state = TU_CONNECTED; 
    dprintf(tu->fd, "%s %d\n",tu_state_names[TU_CONNECTED], peer->fd);   
    dprintf(peer->fd, "%s %d\n",tu_state_names[TU_CONNECTED], tu->fd);   
    sem_post(&peer->mutex); 
    sem_post(&tu->mutex);  
    return 0; 
    abort();
}
// #endif

/*
 * Hang up a TU (i.e. replace the handset on the switchhook).
 *
 *   If the TU is in the TU_CONNECTED or TU_RINGING state, then it goes to the
 *     TU_ON_HOOK state.  In addition, in this case the peer TU (the one to which
 *     the call is currently connected) simultaneously transitions to the TU_DIAL_TONE
 *     state.
 *   If the TU was in the TU_RING_BACK state, then it goes to the TU_ON_HOOK state.
 *     In addition, in this case the calling TU (which is in the TU_RINGING state)
 *     simultaneously transitions to the TU_ON_HOOK state.
 *   If the TU was in the TU_DIAL_TONE, TU_BUSY_SIGNAL, or TU_ERROR state,
 *     then it goes to the TU_ON_HOOK state.
 *
 * In all cases, a notification of the resulting state of the specified TU is sent to
 * to the associated network client.  If a peer TU has changed state, then its client
 * is also notified of its new state.
 *
 * @param tu  The tu that is to be hung up.
 * @return 0 if successful, -1 if any error occurs that results in the originating
 * TU transitioning to the TU_ERROR state. 
 */
// #if 0
int tu_hangup(TU *tu) { 
    if(tu == NULL){
        return -1; 
    }
    sem_wait(&tu->mutex);  
    TU* peer = tu->peer; 
    if(tu->state == TU_CONNECTED){
        if(peer != NULL){
            sem_wait(&peer->mutex); 
            tu->state = TU_ON_HOOK; 
            peer->state = TU_DIAL_TONE; 
            tu->peer = NULL; 
            peer->peer = NULL;   
            tu_unref(tu, "UNREFERNCING TU FROM HANGUP");  
            tu_unref(peer, "UNREFERNCING TU FROM HANGUP"); 
            dprintf(tu->fd, "%s %d\n", tu_state_names[TU_ON_HOOK], tu->extension);
            dprintf(peer->fd, "%s\n", tu_state_names[TU_DIAL_TONE]);
            sem_post(&peer->mutex);
            sem_post(&tu->mutex); 
            return 0;
        }  
    } 
    if(tu->state == TU_RINGING){ 
        if(peer != NULL){
        sem_wait(&peer->mutex); 
        tu->state = TU_ON_HOOK; 
        tu->peer = NULL;  
        peer->peer = NULL;  
        tu_unref(tu, "UNREFERNCING TU FROM HANGUP");  
        tu_unref(peer, "UNREFERNCING TU FROM HANGUP"); 
        peer->state = TU_DIAL_TONE; 
        dprintf(tu->fd, "%s %d\n", tu_state_names[TU_ON_HOOK], tu->extension);
        dprintf(peer->fd, "%s\n", tu_state_names[TU_DIAL_TONE]); 
        sem_post(&tu->mutex);  
        sem_post(&peer->mutex);
        return 0; 
        } 
    }
    if(tu->state == TU_RING_BACK){
        if(peer != NULL) {
            sem_wait(&peer->mutex);
            tu->state = TU_ON_HOOK;
            peer->state = TU_ON_HOOK;
            tu->peer = NULL;
            peer->peer = NULL; 
             tu_unref(tu, "UNREFERNCING TU FROM HANGUP");  
            tu_unref(peer, "UNREFERNCING TU FROM HANGUP"); 
            dprintf(tu->fd, "%s %d\n", tu_state_names[TU_ON_HOOK], tu->extension);
            dprintf(peer->fd, "%s %d\n", tu_state_names[TU_ON_HOOK], peer->extension);
            sem_post(&peer->mutex);
            sem_post(&tu->mutex); 
            return 0; 
        }
    } 
    if(tu->state == TU_DIAL_TONE){
        tu->peer = NULL; 
        tu->state = TU_ON_HOOK;
        dprintf(tu->fd, "%s %d\n", tu_state_names[TU_ON_HOOK], tu->extension);
        sem_post(&tu->mutex); 
        return 0; 
    }
    if(tu->state == TU_BUSY_SIGNAL){
        tu->peer = NULL; 
        tu->state = TU_ON_HOOK;
        dprintf(tu->fd, "%s %d\n", tu_state_names[TU_ON_HOOK], tu->extension);
        sem_post(&tu->mutex); 
        return 0; 
    }
    if(tu->state == TU_ERROR){
        tu->peer = NULL; 
        tu->state = TU_ON_HOOK;
        dprintf(tu->fd, "%s %d\n", tu_state_names[TU_ON_HOOK], tu->extension);
        sem_post(&tu->mutex); 
        return 0; 
    } 
    if(tu->state == TU_ON_HOOK){
        tu->peer = NULL; //Shouldn't have a peer anyways 
        //State won't change 
        dprintf(tu->fd, "%s %d\n", tu_state_names[TU_ON_HOOK], tu->extension);
        sem_post(&tu->mutex); 
        return 0; 
    }
    return 0; 
    // TO BE IMPLEMENTED
} 

// #endif

/*
 * "Chat" over a connection.
 *
 * If the state of the TU is not TU_CONNECTED, then nothing is sent and -1 is returned.
 * Otherwise, the specified message is sent via the network connection to the peer TU.
 * In all cases, the states of the TUs are left unchanged and a notification containing
 * the current state is sent to the TU sending the chat.
 *
 * @param tu  The tu sending the chat.
 * @param msg  The message to be sent.
 * @return 0  If the chat was successfully sent, -1 if there is no call in progress
 * or some other error occurs.
 */
// #if 0
int tu_chat(TU *tu, char *msg) { 
    // TO BE IMPLEMENTED 
    if(tu == NULL) return -1;
    sem_wait(&tu->mutex);
    if(tu->state != TU_CONNECTED || tu->peer == NULL) { 
        if(tu->state == TU_ON_HOOK){
            dprintf(tu->fd, "%s %d\n", tu_state_names[TU_ON_HOOK], tu->extension); 
            sem_post(&tu->mutex); 
            return -1;
        }
        dprintf(tu->fd, "%s\n", tu_state_names[tu->state]);
        sem_post(&tu->mutex);
        return -1;
    }
    TU *peer = tu->peer;
    sem_wait(&peer->mutex); 
    if(msg == NULL) msg = "";
    dprintf(peer->fd, "CHAT %s\n", msg);
    dprintf(tu->fd, "%s %d\n", tu_state_names[tu->state], peer->fd);
    sem_post(&tu->mutex);
    sem_post(&peer->mutex);
    return 0;
}
// #endif
 
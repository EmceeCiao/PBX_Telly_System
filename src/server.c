/*
 * "PBX" server module.
 * Manages interaction with a client telephone unit (TU).
 */
#include <stdlib.h>
#include <string.h>
#include "debug.h"
#include "pbx.h"
#include "server.h"
#include "csapp.h" 
/*
 * Thread function for the thread that handles interaction with a client TU.
 * This is called after a network connection has been made via the main server
 * thread and a new thread has been created to handle the connection.
 */
//#if 0
void *pbx_client_service(void *arg) {
    // TO BE IMPLEMENTED  
    /* 
    The pbx_client_service function is invoked as the thread function for a thread that is created (using pthread_create()) to service a
    client connection. The argument is a pointer to the integer file descriptor to be used to communicate with the client.   
    Once this file descriptor has been retrieved, the storage it occupied needs to be freed. 
    The thread must then become detached, so that it does not have to be explicitly reaped, it must initialize a new TU with that file descriptor,
    and it must register the TU with the PBX module under a particular extension number.  The demo program uses the file descriptor as the
    extension number, but you may choose a different scheme if you wish. Finally, the thread should enter a service loop in which it repeatedly
    receives a message sent by the client, parses the message, and carries out the specified command.
    The actual work involved in carrying out the command is performed by calling the functions provided by the PBX module. 
    These functions will also send the required response back to the client (each syntactically correct command will elicit a single response that contains the resulting state of the TU)
    so the server module need not be directly concerned with that.
    */  
    //To start we need to make sure to detach the thread just like it's done in lecture!  
    if(arg == NULL){
        return NULL; 
    }
    int connfdp = *((int *)arg); 
    pthread_detach(pthread_self());  
    free(arg); //This was malloced in our main.c as connfdp! 
    //Initializing new TU
    TU* telephone = tu_init(connfdp);   
    //Registering TU 
    pbx_register(pbx, telephone, connfdp);  
    //Now we can write the service loop! 

    //Setting up Buffers Before Entering While Loop 
    //One Buffer to Parse Commands and One Buffer For Chat MSGs + size_t to hold size of chat_msg (Going to use strncpy!)  
    //NEED TO DOUBLE CHECK BEHAVIOR OF DEMO WITH DIAL W/O SPACE! 
    char cmd_buffer[1024]; 
    char* chat_msg_buffer = NULL;  
    size_t chat_msg_length = 0;  

    while(1){
        //Need to 0 out our cmd_buffer (I could calloc it but that would require me to rmbr to free!) 
        memset(cmd_buffer, 0, sizeof(cmd_buffer));  
        //Now we need to read character by character until we hit what we need!  
        size_t total_read = 0; 
        char character; 
        while(total_read < sizeof(cmd_buffer) - 1){ 
            size_t curr_char = read(connfdp, &character, 1);  
            //Checking Stream for end!  
            if (curr_char <= 0){
                break; 
            }  
            //Since we are reading byte by byte, I need to check for \n to break and \r I need to skip
            if(character == '\n'){
                break; 
            } 
            if(character == '\r'){
                continue; 
            } 
            cmd_buffer[total_read++] = character; 
        } 
        cmd_buffer[total_read] = '\0'; 

        //Now that we have what we read into our cmd_buffer as well as the total_number of bytes read we can use this! 
        // if(total_read <= 0){
        //     break; 
        // }    
        //Should not break from empty!  
        //Now we unecessary info/the EOL from our buffer!  
        while(total_read > 0 && (cmd_buffer[total_read - 1] == '\0' || cmd_buffer[total_read -1] == '\n')){
            cmd_buffer[--total_read] = '\0';  //Replace with NULL TERMINATOR to make our earlier reading easier! 
        } 
        //It's ok if our total_read is now 0, that just means we have to read more! Before we broke as that suggested empty string! 
        if(total_read == 0){
            continue; 
        }  
        //Now we have to handle each case, TU_DIAL, TU_CHAT, TU_HANGUP, TU_PICKUP  
        //TU_PICKUP! 
        if(strcmp(cmd_buffer, tu_command_names[TU_PICKUP_CMD]) == 0){
            tu_pickup(telephone); 
        }  
        //TU_HANGUP 
        else if(strcmp(cmd_buffer, tu_command_names[TU_HANGUP_CMD]) == 0){ 
            tu_hangup(telephone); 
        } 
        //TU_DIAL 
        else if(strncmp(cmd_buffer, tu_command_names[TU_DIAL_CMD], strlen(tu_command_names[TU_DIAL_CMD])) == 0){ 
            char *start_extension = cmd_buffer + 4;  
            //We get rid of any spaces for the dial tone!  
            if(*start_extension != ' '){
                continue; 
            }
            while(*start_extension == ' '){start_extension++;}   
                //We check if it's a number! Otherwise we pass in -1 to PBX_DIAL and we'll make it return -1 if it sees that! 
                int num_flag = 0;  
                int ext = 0;
                char* end; 
                long val = strtol(start_extension, &end, 10); 
                if (*end != '\0' || val <= 0) { 
                    num_flag = -1; 
                }
                if(num_flag == -1){
                    ext = -1; //We will pass this into PBX_DIAL and check for it! 
                }else{
                    ext = (int)val; 
                }
                if(pbx_dial(pbx, telephone, ext)< 0){ 
                    // break; This break statement broke my code/ended the connection when it shouldnt! 
                }
        } 
        else if(strncmp(cmd_buffer, tu_command_names[TU_CHAT_CMD], strlen(tu_command_names[TU_CHAT_CMD])) == 0){ 
            //This is when we use chat_msg_buffer! 
            char* chat_msg = cmd_buffer + 4;  
            while(*chat_msg == ' '){chat_msg++;}    
            free(chat_msg_buffer); 
            chat_msg_buffer = NULL; 
            chat_msg_length = 0;  
            chat_msg_length = strlen(chat_msg) + 1; 
            chat_msg_buffer = malloc(chat_msg_length); 
            strncpy(chat_msg_buffer, chat_msg, chat_msg_length); 
            chat_msg_buffer[chat_msg_length-1] = '\0'; 
            tu_chat(telephone, chat_msg_buffer); 
        }
    }
    free(chat_msg_buffer); 
    close(connfdp);  
    pbx_unregister(pbx, telephone); 
    //tu_unref(telephone, "ENDED Server/Thread!");  //Maybe I want to move this into pbx_unregister! 
    return NULL;  
}


/*************************************************************
** Program: posixmsg_server.c
** Author: Kai Shi
** Date: 1/30/2017
** Description: client/server processes that will do something 
                along the lines of an client/server pair 
                of programs through POSIX Message Queue.

** Input: ./posixmsg_server 
** Output: Waiting for clients' connection requests.
** Version: 1.0
**************************************************************/


#include <stdio.h>
#include <stdlib.h>
#include <mqueue.h>
#include <limits.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <sys/types.h>
#include <signal.h>
#include "posixmsg.h"
#include <pthread.h>


char clientReader_name[PAYLOAD_LENGTH];
char clientWriter_name[PAYLOAD_LENGTH];
char server_name[PAYLOAD_LENGTH];
   
    mqd_t mq1;
    //mqd_t rec;
    
static pthread_mutex_t mutex;
void handler(int sigl);

void static *threadFunc(void *mes)
{   
    struct message_s rec;
    
    mqd_t mq;
    mqd_t mq2;

    //struct mq_attr attrdir; 

    ssize_t t;
    ssize_t t1;

    char *r;

    FILE *stream;

   // void *thr;

    int file; //put
    int file2; //get
//int i;
    char temp[PAYLOAD_LENGTH];

    char current_dir[PAYLOAD_LENGTH];
    char server_dir[PAYLOAD_LENGTH];

    //char buf3[PAYLOAD_LENGTH];

    //************ get dir *************
    getcwd(current_dir, PAYLOAD_LENGTH );
    getcwd(server_dir, PAYLOAD_LENGTH );

    rec=*(struct message_s*)mes;
    pthread_detach(pthread_self());

    //************ create messagequeue *************
    CREATE_CLIENT_READER_NAME(clientReader_name, (int)atol(rec.command));
    mq=mq_open(clientReader_name, O_WRONLY,QUEUE_PERMISSIONS,NULL);
    if (mq <0 ) {
        perror ("Failed to create client for reading");
        exit(EXIT_FAILURE);
    }

    CREATE_CLIENT_WRITER_NAME(clientWriter_name, (int)atol(rec.command));
    mq2=mq_open(clientWriter_name, O_RDONLY,QUEUE_PERMISSIONS,NULL);
    if (mq2<0 ) {
        perror ("Failed to create client for writing");
        exit(EXIT_FAILURE);
    }
    //************ create completely *************

    memset(rec.command,0, sizeof(COMMAND_LENGTH));

    for(;;) {
        
        t=mq_receive(mq2,(char *)&rec,sizeof(rec),NULL);
        
        
        if (t>0) {

            //home
            if (strncmp(rec.command,CMD_REMOTE_HOME,strlen(CMD_REMOTE_HOME))==0){
                
                pthread_mutex_lock(&mutex);
                
                
                if ( chdir ( current_dir ) != -1 ) {
                    
                    getcwd( current_dir, PAYLOAD_LENGTH);
                }
                
                r=getenv("HOME");
                chdir(getenv("HOME"));
                
                getcwd( current_dir, PAYLOAD_LENGTH);
                sprintf(rec.payload, "%s", r);
                
               
                if(mq_send(mq,(char *)&rec,sizeof(rec),0)==-1){
                    perror("Could not write the home to the client!");
                    
                    
                    pthread_mutex_unlock(&mutex);
                    exit(1);
                }
                
                if ( chdir (server_dir) != -1 ) {
                    
                    getcwd( server_dir, PAYLOAD_LENGTH);
                }
               
                pthread_mutex_unlock(&mutex);
                
                memset(rec.command,0, sizeof(COMMAND_LENGTH));
                memset(rec.payload,0, sizeof(PAYLOAD_LENGTH));
                continue;
            }
            
            //pwd
            else if (strncmp(rec.command,CMD_REMOTE_PWD,strlen(CMD_REMOTE_PWD))==0){
                pthread_mutex_lock(&mutex);
                if ( chdir ( current_dir ) != -1 ) {
                    
                    getcwd( current_dir, PAYLOAD_LENGTH);
                }
                
                getcwd(rec.payload,PAYLOAD_LENGTH);
                
                if(mq_send(mq,(char *)&rec,sizeof(rec),0)==-1){
                    perror("Could not write the pmd to the server!");
                    
                    
                    pthread_mutex_unlock(&mutex);
                    exit(1);
                }
                if ( chdir ( server_dir ) != -1 ) {
                    
                    getcwd( server_dir, PAYLOAD_LENGTH);
                }
                
                pthread_mutex_unlock(&mutex);
                memset(rec.command,0, sizeof(COMMAND_LENGTH));
                memset(rec.payload,0, sizeof(PAYLOAD_LENGTH));
                continue;
            }

            
            //dir
            else if (strncmp(rec.command, CMD_REMOTE_DIR, strlen(CMD_REMOTE_DIR)) == 0) {
                
                
                pthread_mutex_lock(&mutex);
                
                if ( chdir ( current_dir ) != -1 ) {
                    
                    getcwd( current_dir, PAYLOAD_LENGTH);
                }
                
                stream = popen ( CMD_LS_POPEN, "r" );
                
                if (stream != NULL ) {
                    while(fgets(rec.payload,PAYLOAD_LENGTH,stream) ) {

                        
                      
                        if(mq_send(mq,(char *)&rec,sizeof(rec),0)==-1){
                            perror("Could not write the pmd to the server!");
                            
                            pthread_mutex_unlock(&mutex);
                            exit(1);
                        }
                    
                    }
                    
                }

                






                if(pclose(stream)==-1){
                    perror("Could not close the client pclose(dir)!");
                    
                    pthread_mutex_unlock(&mutex);
                    
                    exit(1);
                }
                
                if ( chdir ( server_dir ) != -1 ) {
                    
                    getcwd(server_dir, PAYLOAD_LENGTH);
                }
                pthread_mutex_unlock(&mutex);

                memset(rec.command,0, sizeof(COMMAND_LENGTH));
                memset(rec.payload,0, sizeof(PAYLOAD_LENGTH));
                continue;
            }

            //cd
            else if (strncmp(rec.command,CMD_REMOTE_CHDIR,strlen(CMD_REMOTE_CHDIR))==0){

                pthread_mutex_lock(&mutex);
                
                if ( chdir ( current_dir ) != -1 ) {
                    
                    getcwd( current_dir, PAYLOAD_LENGTH);
                }
                

                
                if(chdir(rec.payload)==-1)
                {
                    rec.message_type=MESSAGE_TYPE_ERROR;
                    memset(rec.command,0, sizeof(COMMAND_LENGTH));
                    memset(rec.payload,0, sizeof(PAYLOAD_LENGTH));
                    
                    if(mq_send(mq,(char *)&rec,sizeof(rec),0)==-1){
                        printf("Could not do this to the server!\n");
                        
                        pthread_mutex_unlock(&mutex);
                        exit(1);
                    }
                    rec.message_type=MESSAGE_TYPE_NORMAL;
                }
                

                if(chdir(rec.payload)==0)
                {
                    getcwd(rec.payload,PAYLOAD_LENGTH);
                    if(mq_send(mq,(char *)&rec,sizeof(rec),0)==-1){
                        perror("Could not write the pmd to the server!");
                        
                        pthread_mutex_unlock(&mutex);
                        exit(1);
                    }
                    getcwd(current_dir,PAYLOAD_LENGTH);
                    
                }
                
                
                
                if ( chdir ( server_dir ) != -1 ) {
                    
                    getcwd( server_dir, PAYLOAD_LENGTH);
                }
                
                pthread_mutex_unlock(&mutex);
                memset(rec.command,0, sizeof(COMMAND_LENGTH));
                memset(rec.payload,0, sizeof(PAYLOAD_LENGTH));
                continue;
            }
            
            //put
            if (strncmp(rec.command, CMD_PUT, strlen(CMD_PUT)) == 0) {
               
                pthread_mutex_lock(&mutex);


                 if ( chdir ( current_dir ) != -1 ) {
                    
                    getcwd( current_dir, PAYLOAD_LENGTH);
                }


                sprintf(temp, "%s", rec.payload);

                file = open ( temp, O_CREAT | O_WRONLY | O_TRUNC, SEND_FILE_PERMISSIONS);  //创建文件  一切正常可以建
              
                     memset(&rec,'\0', sizeof(rec));
                  
                    
                //t1=mq_receive(mq2,(char*)&rec,sizeof(rec),NULL);
               
               while((mq_receive(mq2,(char*)&rec,sizeof(rec),NULL)>0)&&(rec.message_type!=MESSAGE_TYPE_SEND_END))
               {

                
                write(file,rec.payload,rec.num_bytes);
                memset(&rec,'\0',sizeof(rec));


               }

                close (file);
                pthread_mutex_unlock(&mutex);
                continue;

            }

                //get
            else if ( strncmp ( rec.command, CMD_GET, strlen(CMD_GET)) == 0 )
            {
                pthread_mutex_lock(&mutex);


                 if ( chdir ( current_dir ) != -1 ) {
                    
                    getcwd( current_dir, PAYLOAD_LENGTH);
                }
                

                //printf("%s\n", rec.payload);
                snprintf ( temp, strlen(rec.payload)+1, "%s", rec.payload ); 
                //printf("%s\n",temp );
                file2 = open ( temp, O_RDONLY);
                if (file2<0)
                {
                    rec.message_type=MESSAGE_TYPE_ERROR;
                    memset( rec.payload, 0, sizeof(PAYLOAD_LENGTH) );
                    mq_send(mq,(char *)&rec,sizeof(rec),0);
                    rec.message_type=MESSAGE_TYPE_NORMAL;
                    continue;

                }
                //printf("%d\n",file2 );
            
                for(;;){

                    
                    memset( rec.payload, 0, sizeof(PAYLOAD_LENGTH) );
                 
                    t=read(file2, rec.payload, sizeof(PAYLOAD_LENGTH) );
                    rec.num_bytes=t;
                    //printf("%d\n", t);

                    if ( t > 0 ) {
                    
                        t1=mq_send(mq,(char *)&rec,sizeof(rec),0);
                  
                        } //if
                    
                    else {
                    
                    rec.message_type= MESSAGE_TYPE_SEND_END;
  
                    mq_send(mq,(char *)&rec,sizeof(rec),0);
                    
                    
                   
                   printf("    send file complete\n");
                    break;
                }  



                }//for

                close ( file2);
                pthread_mutex_unlock(&mutex);
            continue;


            }



            //exit
            else if (strncmp(rec.command,CMD_EXIT,strlen(CMD_EXIT))==0){
                
                printf ("%s\n","aaaaa");
                printf ("%s\n",rec.command);
                
                
                pthread_exit(NULL);
                
                
                
            }
            
            
        }
    }


    
}





int main(int argc, const char * argv[]) {
   
    struct mq_attr attr;
    //struct mq_attr attrrec;

    struct message_s receiver;

    pthread_t t1;
    int s;

    signal(SIGINT, handler);

    umask(0);
    attr.mq_flags=0;
    attr.mq_maxmsg=10;
    attr.mq_msgsize=sizeof(struct message_s);
    attr.mq_curmsgs=0;


    CREATE_SERVER_QUEUE_NAME(server_name);
    mq1=mq_open(server_name,O_RDONLY|O_CREAT|O_EXCL,QUEUE_PERMISSIONS , &attr);
    if (mq1 <0 ) {
        perror ("Failed to open server msgqueue");
        exit(EXIT_FAILURE);
    }

    for(;;){
        
        mq_receive(mq1,(char *) &receiver,sizeof(receiver),NULL);

        receiver.command[strlen(receiver.command)]='\0';
        receiver.payload[strlen(receiver.payload)]='\0';

        if(receiver.command>=0){
        
       
        s=pthread_create(&t1,NULL,threadFunc,(void *)&receiver);
        
        
        }//if
    }//for

    //return 0;
}



void handler(int sigl) {

    if (mq_close(mq1) != 0) {
        perror("Failed in close(mq1)");
    }
    if (mq_unlink(server_name) != 0) {
        perror("Failed in mq_unlink(mq1)");
    }
    exit(EXIT_SUCCESS);
    
}


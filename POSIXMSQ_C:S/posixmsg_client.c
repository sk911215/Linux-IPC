/*************************************************************
** Program: posixmsg_client.c
** Author: Kai Shi
** Date: 1/30/2017
** Description: client/server processes that will do something 
                along the lines of an client/server pair 
                of programs through POSIX Message Queue.

** Input: ./posixmsg_client -> commands s.t. cd dir pwd etc.
** Output: Executing commands and get results
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



char clientReader_name[1024];
char clientWriter_name[1024];
char server_name[1024];
    mqd_t mq;
    mqd_t mq1;
    mqd_t mq2;
   
void handler_simple(int sigl);

void handler_simple(int sigl) {
    
     mq_close(mq);
     mq_close(mq2);

     mq_unlink(clientReader_name);
     mq_unlink(clientWriter_name);
     exit(EXIT_SUCCESS);
}


int main(int argc, const char * argv[]) {
   
    
    
    int len;
    unsigned int prio;

    char *r;
    char buf[2048];
    char buf2[2048];
    char read_buffer[2048];
    char buf3[PAYLOAD_LENGTH];

    pid_t pid;

    ssize_t bytes_read;
    ssize_t t1;
    
    struct message_s sender;
    struct message_s sender1;
    struct message_s sender2;
    struct message_s *ptr;
    
    struct message_s receiver;

    int t;
    int file;
    int file2;
    struct mq_attr attr;
    struct mq_attr attrdir; 

    int count=0;
    umask(0);
    attr.mq_flags=0;
    attr.mq_maxmsg=10;
    attr.mq_msgsize=sizeof(struct message_s);
    attr.mq_curmsgs=0;

    if (signal(SIGPIPE, SIG_IGN) == SIG_ERR)
        perror("signal");
    
    signal(SIGINT,handler_simple);
     

     //************ create messagequeue *************

    CREATE_CLIENT_READER_NAME(clientReader_name, (int)getpid());
    mq=mq_open(clientReader_name, O_RDONLY|O_CREAT|O_EXCL, QUEUE_PERMISSIONS , &attr);
    if (mq <0 ) {
        perror ("Failed to create client for reading");
        exit(EXIT_FAILURE);
    }

    CREATE_CLIENT_WRITER_NAME(clientWriter_name, (int)getpid());
    mq2=mq_open(clientWriter_name, O_WRONLY|O_CREAT|O_EXCL, QUEUE_PERMISSIONS, &attr);
    if (mq2<0 ) {
        perror ("Failed to create client for writing");
        exit(EXIT_FAILURE);
    }

    CREATE_SERVER_QUEUE_NAME(server_name);
    mq1=mq_open(server_name,O_WRONLY);
    if (mq1 <0 ) {
        perror ("Failed to open server msgqueue");
        exit(EXIT_FAILURE);
    }

     //************ create completely *************

     //************ send PID *************
    prio=0;
    pid=(int)getpid();
    
    sprintf(sender.command, "%d\n",pid);
    sender.message_type = MESSAGE_TYPE_NORMAL;
    
    len=sizeof(sender.command)/sizeof(char);
    
    ptr=&sender;
    mq_send(mq1,(char*)ptr,len,prio);

    
    if(mq_close(mq1)==-1)
    {
        perror("Client coud not close server Message!");
        exit(1);
    }
    //************ send PID completely *************

    for (;;)
    {
        
        for (;;) {
            printf (PROMPT);
            fflush(stdout);
            
            fgets(read_buffer,sizeof(read_buffer),stdin);
            
            
            //ldir
            if ((strncmp ( read_buffer, CMD_LOCAL_DIR, strlen(CMD_LOCAL_DIR) ) == 0)&& strlen(read_buffer)-1==strlen(CMD_LOCAL_DIR)) {
                pclose ( popen ( CMD_LS_POPEN, "w" ) );
                continue;
            }
            
            //lpwd
            if (( strncmp ( read_buffer, CMD_LOCAL_PWD, strlen(CMD_LOCAL_PWD)) == 0)&&strlen(read_buffer)-1==strlen(CMD_LOCAL_PWD)){
                getcwd(buf, 256);
                printf("%s\n","The working directory for the client is:");
                printf("\t%s\n",buf);
                memset(read_buffer,0, 1024);
                continue;
            }
            
            //lcd
            if ( strncmp ( read_buffer, CMD_LOCAL_CHDIR, strlen(CMD_LOCAL_CHDIR)) == 0){
                
                snprintf ( buf2, (strlen(read_buffer))- 4, "%s", read_buffer+4 );
                
                if(chdir(buf2)==-1)
                {   memset(read_buffer,0, 1024);
                    printf("Can not do this.\n");
                    continue;
                }

                memset(read_buffer,0, 1024);
                printf("%s\n","The working directory for the client is:");
                printf("\t%s\n",buf2);
                continue;
            }
            
            //lhome
            if ((strncmp(read_buffer,CMD_LOCAL_HOME,strlen(CMD_LOCAL_HOME))== 0 )&&strlen(read_buffer)-1==strlen(CMD_LOCAL_HOME)){
                r=getenv("HOME");
                chdir(r);
                printf("%s\n","The working directory for the client is:");
                printf ( "\t%s\n",r);
                memset(read_buffer,0, 1024);
                continue;
            }
            
            //exit
            if (strncmp(read_buffer,CMD_EXIT,strlen(CMD_EXIT))== 0 ){
                sprintf(sender1.command, "%s", CMD_EXIT);

                if(mq_send(mq2,(char *)&sender1,sizeof(sender1),0)==-1){
                    perror("Could not write the exit to the server!");
                    exit(1);
                    }
                    printf("\n\n");
                printf("\t%s\n","Sayonara...");
                printf("\n\n");
                raise(SIGINT);
                exit(1);
            }
            
            //help
            if (strncmp(read_buffer,CMD_HELP,strlen(CMD_HELP))==0&&strlen(read_buffer)-1==strlen(CMD_HELP)){
                printf("Available client commands:\n"
                       "\thelp      : print this help text\n"
                       "\texit      : exit the client, causing the client server to also exit\n"
                       "\tcd <dir>  : change to directory <dir> on the server side\n"
                       "\tlcd <dir> : change to directory dir> on the client side\n"
                       "\tdir       : show a `ls -lF` of the server side\n"
                       "\tldir      : show a `ls -lF` on the client side\n"
                       "\thome      : change current directory of the client server to the user's home directory\n"
                       "\tlhome     : change current directory of the client to the user's home directory\n"
                       "\tpwd       : show the current directory from the server side\n"
                       "\tlpwd      : show the current directory on the client side\n"
                       "\tget <file>: send <file> from server to client (extra credit)\n"
                       "\tput <file>: send <file> from client to server (extra credit)\n");
                continue;
            }
            //home
            if (strncmp(read_buffer,CMD_REMOTE_HOME,strlen(CMD_REMOTE_HOME))==0&&(strlen(read_buffer)-1==strlen(CMD_REMOTE_HOME))){
                
                    sprintf(sender1.command, "%s", CMD_REMOTE_HOME);
                    

                    if(mq_send(mq2,(char *)&sender1,sizeof(sender1),0)==-1){
                    perror("Could not write the home to the server!");
                    exit(1);
                    }
                     memset(read_buffer,0, 1024);
                     memset(sender1.command,0, sizeof(COMMAND_LENGTH));
                     break;
            }
            //pwd
            if (strncmp(read_buffer,CMD_REMOTE_PWD,strlen(CMD_REMOTE_PWD))==0&&(strlen(read_buffer)-1==strlen(CMD_REMOTE_PWD))){
                
                sprintf(sender1.command, "%s", CMD_REMOTE_PWD);

                if(mq_send(mq2,(char *)&sender1,sizeof(sender1),0)==-1){
                    perror("Could not write the pmd to the server!");
                    exit(1);
                }
                memset(read_buffer,0, 1024);
                memset(sender1.command,0, sizeof(COMMAND_LENGTH));
                break;
            }

            //cd
            if (strncmp(read_buffer,CMD_REMOTE_CHDIR,strlen(CMD_REMOTE_CHDIR))==0){
                
                sprintf(sender1.command ,"%s", CMD_REMOTE_CHDIR);
                snprintf(sender1.payload, strlen(read_buffer)-strlen(CMD_REMOTE_CHDIR)-1,"%s", read_buffer+strlen(CMD_REMOTE_CHDIR)+1);
                

                if(mq_send(mq2,(char *)&sender1,sizeof(sender1),0)==-1){
                    perror("Could not write the cd to the server!");
                    exit(1);
                }

                

                memset(read_buffer,0, 1024);
                memset(sender1.command,0, sizeof(COMMAND_LENGTH));
                memset(sender1.payload,0, sizeof(PAYLOAD_LENGTH));
                break;
            }

            //dir
            if (strncmp(read_buffer,CMD_REMOTE_DIR,strlen(CMD_REMOTE_DIR))==0){
                
                count=1;
                sprintf(sender1.command, "%s", CMD_REMOTE_DIR);

                if(mq_send(mq2,(char *)&sender1,sizeof(sender1),0)==-1){
                    perror("Could not write the pmd to the server!");
                    exit(1);
                }
                memset(read_buffer,0, 1024);
                memset(sender1.command,0, sizeof(COMMAND_LENGTH));
                break;
                
            }

            //put 
            if ( strncmp ( read_buffer, CMD_PUT " ", strlen(CMD_PUT) + 1 ) == 0 ){

                read_buffer[strlen(read_buffer) - 1] = '\0';
                snprintf ( buf3, strlen(read_buffer), "%s", read_buffer+4 ); // 存文件名字

                sprintf(sender1.payload, "%s", buf3); // 送put命令
                sprintf(sender1.command, "%s", CMD_PUT);
                
                file = open ( buf3, O_RDONLY);
                if (file<0)
                {
                    printf("\tNo such file\n");
                    continue;
                }

                if(mq_send(mq2,(char *)&sender1,sizeof(sender1),0)==-1){
                    perror("Could not write the pmd to the server!");
                    exit(1);
                }

                for (;;) {
                   
                    memset( sender2.payload, 0, sizeof(PAYLOAD_LENGTH) );
                 
                    t=read(file, sender2.payload, sizeof(PAYLOAD_LENGTH) );
                    sender2.num_bytes=t;
                    
                    if ( t > 0 ) {
                      
                   
                        t1=mq_send(mq2,(char *)&sender2,sizeof(sender2),0);
                       
                  
                        } //if
               
                    else {
                    
                    sender2.message_type= MESSAGE_TYPE_SEND_END;
  
                    mq_send(mq2,(char *)&sender2,sizeof(sender2),0);
                    
                    
                   
                   printf("\tPut file completely\n");
                    break;
                }  
            
            } // for
            close ( file);
            memset(read_buffer,0, 1024);
            
            continue;

            }


             //get
             if ( strncmp ( read_buffer, CMD_GET, strlen(CMD_GET) ) == 0 )
             {

                read_buffer[strlen(read_buffer) - 1] = '\0';
                snprintf ( buf3, strlen(read_buffer), "%s", read_buffer+4 );

                sprintf(sender1.payload, "%s", buf3); // 送get命令
                sprintf(sender1.command, "%s", CMD_GET);

                if(mq_send(mq2,(char *)&sender1,sizeof(sender1),0)==-1){
                    perror("Could not write the get to the server!");
                    exit(1);
                }

                memset(&sender1,'\0',sizeof(sender1));


               




               if((mq_receive(mq,(char*)&sender1,sizeof(sender1),NULL)>0)&&(sender1.message_type==MESSAGE_TYPE_ERROR))

               {
                printf("No such file\n");
                memset(&sender1,'\0',sizeof(sender1));
                continue;


               }



                file2= open ( buf3, O_CREAT | O_WRONLY | O_TRUNC, SEND_FILE_PERMISSIONS); 

               


                while((mq_receive(mq,(char*)&sender1,sizeof(sender1),NULL)>0)&&(sender1.message_type!=MESSAGE_TYPE_SEND_END))
                {
                    //printf("aaaaaaa");
                    write(file2,sender1.payload,sender1.num_bytes);
                    memset(&sender1,'\0',sizeof(sender1));

                }//while

                    printf("\tGet file completely\n");
                
                close (file2);
                continue;

             }



             if (strncmp(read_buffer,"\n",1)==0){
                continue;
            }
            else
            {   printf("Client: Command not recognized!\n");
                continue;
            }

    
    }//2nd for

                if(count==1){
                    
                    sleep(1);
                    mq_getattr(mq,&attrdir);
                    while(attrdir.mq_curmsgs!=0)
                {   
                    mq_receive(mq,(char *) &receiver,sizeof(receiver),NULL);
                    printf("%s",receiver.payload);
                    mq_getattr(mq,&attrdir);            
                }
                count=0;
                }




            else{

                    
                    bytes_read=mq_receive(mq,(char *) &receiver,sizeof(receiver),NULL);
                if (bytes_read < 0){
                    
                    perror("Could not receive data from server!");
                    //fprintf(stderr, "bytes_read: %d\n", bytes_read);
                    exit(1);
                    }

                if (receiver.message_type==MESSAGE_TYPE_ERROR)
                {
                    printf("\tNo such file directory\n");
                    memset(read_buffer,0, 1024);
                    memset(receiver.command,0, sizeof(COMMAND_LENGTH));
                    memset(receiver.payload,0, sizeof(PAYLOAD_LENGTH));
                    continue;
                }

                    printf("%s\n","The working directory for the client-server is:");
                    printf("\t%s\n",receiver.payload);

                 }

}//1st for
   
    //return 0;
}

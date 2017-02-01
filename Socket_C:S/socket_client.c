/*************************************************************
** Program: socket_client.c
** Author: Kai Shi
** Date: 1/24/2017
** Description: client/server processes that will do something 
                along the lines of an client/server pair 
                of programs through Socket.

** Input: ./fifo_client or ./fifo_client (ip address) or (port num) -> commands s.t. cd dir pwd etc.
** Output: Executing commands and get results
** Version: 1.0
**************************************************************/

#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <sys/ioctl.h>
#include <net/if.h>
#include <arpa/inet.h>
#include <string.h>
#include <fcntl.h>
#include <errno.h>
#include "socket_hdr.h"

int main(int argc, char * argv[]){

	int opt, port;
    int isport,ispri;
    char* ip;

    int sfd;
    int file1;
    int file2;
    
    char read_buffer[MAXLINE];
    char buffer[MAXLINE];
    char buf2[MAXLINE];
    char sockRead[MAXLINE];
    char sockWrite[MAXLINE];
    char clientReaddir[MAXLINE];
    
    ssize_t isNull;
    
    char *r;
    struct sockaddr_in servaddr;    

    
    isport=0;
    ispri=0;

    while((opt = getopt(argc, argv, "p:i:h")) != -1){
        switch(opt){
            case 'h':
                printf("%s\n","h");
                break;
            case 'p':
                port = atoi(optarg);
                printf("%d\n",port);
                isport=1;
                break;
            case 'i':
                ip = optarg;
                printf("%s\n",ip);
                ispri=1;
                break;
            default:
                break;
        }
        
    }


    if (isport==0) {
        port=10086;
        printf("%d\n",port);
    }
    
    
    
    if((sfd=socket(AF_INET,SOCK_STREAM,0))<0){
    	perror("failed to create socket");
        exit(1);
    }


    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(port);

    if (ispri==1){
    	inet_pton(AF_INET, ip, &servaddr.sin_addr.s_addr);
    }

    else{
        servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
        ip = inet_ntoa(servaddr.sin_addr);
    }

    if (connect(sfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0) {
        perror("could not connect");
        exit(EXIT_FAILURE);
    }

    while (1){
    	memset(read_buffer,0, MAXLINE);
        printf (PROMPT);
        fgets(read_buffer, sizeof(read_buffer),stdin);
        
        //lpwd
        if (strncmp (read_buffer, CMD_LOCAL_PWD, strlen(CMD_LOCAL_PWD)) == 0){
            printf("%s\n","The working directory for the client is:");
            getcwd(buffer, MAXLINE);
            printf ( "\t%s\n",buffer);
            memset(buffer,0, MAXLINE);
            memset(read_buffer,0, MAXLINE);
            continue;
        }


        //lhome
        else if (strncmp(read_buffer,CMD_LOCAL_HOME,strlen(CMD_LOCAL_HOME)) == 0){
            r=getenv("HOME");
            chdir(r);
            printf("%s\n","The working directory for the client is:");
            printf ("\t%s\n",r);
            memset(read_buffer,0, MAXLINE);
            continue;
        }


        //lcd   
        else if (strncmp(read_buffer,CMD_LOCAL_CHDIR,strlen(CMD_LOCAL_CHDIR)) == 0){
            snprintf (buffer, (strlen(read_buffer))-(strlen(CMD_LOCAL_CHDIR)), "%s", read_buffer+(strlen(CMD_LOCAL_CHDIR)));
            if (chdir(buffer) != -1 ){
                getcwd ( buffer, MAXLINE );
                printf("The working directory for the server is:\n");
                printf ( "\t%s\n",buffer);
            }
            else{
                printf("\tThere is no such directory!\n");
            }
            memset(buffer,0, MAXLINE);
            memset(read_buffer,0, MAXLINE);
            continue;
        }


        //ldir
        else if (strncmp (read_buffer, CMD_LOCAL_DIR, strlen(CMD_LOCAL_DIR)) == 0){
            
            if(pclose(popen(CMD_LS_POPEN,"w"))==-1){
                perror("Could not close the client pclose(ls) FIFO!");
                exit(1);
            }
            memset(read_buffer,0, MAXLINE);
            continue;
        }
        
        
        //help
        else if (strncmp(read_buffer,CMD_HELP,strlen(CMD_HELP)) == 0){
            printf("Available client commands:\n"
                   "  help      : print this help text\n"
                   "  exit      : exit the client, causing the client server to also exit\n"
                   "  cd <dir>  : change to directory <dir> on the server side\n"
                   "  lcd <dir> : change to directory dir> on the client side\n"
                   "  dir       : show a `ls -lFABh --group-directories-first` of the server side\n"
                   "  ldir      : show a `ls -lFABh --group-directories-first` on the client side\n"
                   "  home      : change to home directory on the server side\n"
                   "  lhome     : change to home directory on the client side\n"
                   "  pwd       : show the current directory from the server side\n"
                   "  lpwd      : show the current directory on the client side\n"
                   "  get <file>: send <file> from server to client\n"
                   "  put <file>: send <file> from client to server\n");
            
            printf("  You are connected to a server at IPv4 address %s\n",ip);
            printf("  You are connected over port %d\n",port);
            memset(read_buffer,0, MAXLINE);
            continue;
        }




        //pwd
        else if (strncmp(read_buffer,CMD_REMOTE_PWD,strlen(CMD_REMOTE_PWD))==0&&(strlen(read_buffer)-1==strlen(CMD_REMOTE_PWD))){
            
            
            read_buffer[strlen(CMD_REMOTE_PWD)]='\0';
            write(sfd,read_buffer,strlen(read_buffer));
            
            
            if (read(sfd, sockRead, sizeof(sockRead)) == 0){
                perror("socket is closed");
                exit(1);
            }
            
            printf("The working directory for the server is:\n");
            printf("\t%s\n",sockRead);
            memset(read_buffer,0, MAXLINE);
            memset(sockRead,0, MAXLINE);
            continue;
        }

        //home
        else if(strncmp(read_buffer,CMD_REMOTE_HOME,strlen(CMD_REMOTE_HOME))==0&&(strlen(read_buffer)-1==strlen(CMD_REMOTE_HOME))){
            
            read_buffer[strlen(CMD_REMOTE_HOME)]='\0';
            write(sfd,read_buffer,strlen(read_buffer));
            
            if (read(sfd, sockRead, sizeof(sockRead)) == 0) {
                perror("socket is closed");
                exit(1);
            }
            printf("The working directory for the server is:\n");
            printf("\t%s\n",sockRead);
            
            memset(read_buffer,0, MAXLINE);
            memset(sockRead,0, MAXLINE);
            
            continue;
        }
        
       
          
        //cd 
        else if (strncmp(read_buffer,CMD_REMOTE_CHDIR,strlen(CMD_REMOTE_CHDIR))==0){
            
            memcpy(sockWrite, read_buffer,sizeof(read_buffer));
            sockWrite[strlen(read_buffer)]='\0';
            
            
            write(sfd,sockWrite,strlen(sockWrite)-1);
            
            if (read(sfd, sockRead, sizeof(sockRead)) == 0){
                perror("socket is closed");
                exit(1);
            }
            
            printf("The working directory for the server is:\n");
            printf("\t%s\n",sockRead);
            
            memset(read_buffer,0, MAXLINE);
            memset(sockRead,0, MAXLINE);
            memset(sockWrite,0, MAXLINE);
            continue;    
        }

        
        //dir
        else if (strncmp(read_buffer,CMD_REMOTE_DIR,strlen(CMD_REMOTE_DIR))==0&&(strlen(read_buffer)-1==strlen(CMD_REMOTE_DIR))){
            
            printf("%s",read_buffer);

            read_buffer[strlen(CMD_REMOTE_PWD)]='\0';
            
            write(sfd,read_buffer,strlen(read_buffer));
            
            while (read(sfd, clientReaddir, sizeof(clientReaddir))>0 && clientReaddir[0]!=EOT_CHAR){
               
                printf("%s",clientReaddir);
                memset(clientReaddir,0, MAXLINE);
            }
     
            
            memset(read_buffer,0, MAXLINE);
            memset(buffer,0, MAXLINE);
            memset(clientReaddir,0, MAXLINE);
            continue;
        }
        
        
        //put
        else if (strncmp(read_buffer,CMD_PUT_TO_SERVER,strlen(CMD_PUT_TO_SERVER))==0){
            
            read_buffer[strlen(read_buffer)]='\0';

            snprintf (sockWrite, strlen(read_buffer)-strlen(CMD_PUT_TO_SERVER), "%s", read_buffer+strlen(CMD_PUT_TO_SERVER));
           
            printf("Please wait...\n");
            
            file1=open(sockWrite,O_RDONLY);
            
            if(file1<0){
                perror("failed to open file");
                close(file1);
                memset(read_buffer,0, MAXLINE);
                memset(sockWrite,0, MAXLINE);
                continue;
            }
            
            write(sfd,read_buffer,strlen(read_buffer)-1);
            memset(read_buffer,0, MAXLINE);
            
            while (1) {
                isNull=read(file1,sockWrite,sizeof(sockWrite));
                if (isNull>0) {
                    write(sfd,sockWrite,strlen(sockWrite));
                    memset(sockWrite,0, MAXLINE);
                }
                else if(isNull==0)
                {
                    sprintf (sockWrite, "%c", EOT_CHAR);
                    sleep(5);
                    write(sfd,sockWrite,strlen(sockWrite));
                    break;
                }
                
            }
            memset(read_buffer,0, MAXLINE);
            memset(sockWrite,0, MAXLINE);
            printf("Put complete\n");
            continue;
            
        }
        
        
        //get
        else if (strncmp(read_buffer,CMD_GET_FROM_SERVER,strlen(CMD_GET_FROM_SERVER))==0){
            
            read_buffer[strlen(read_buffer)]='\0';
            snprintf (sockWrite, strlen(read_buffer)-strlen(CMD_GET_FROM_SERVER), "%s", read_buffer+strlen(CMD_PUT_TO_SERVER));
            
            printf("Please wait...\n");
            write(sfd,read_buffer,strlen(read_buffer)-1);

            file2= open(sockWrite,O_CREAT | O_WRONLY, SEND_FILE_PERMISSIONS);
            strcpy (buf2,  RETURN_ERROR);
            
            memset(sockRead,0,MAXLINE);

            while ((read(sfd, sockRead, sizeof(sockRead)))>0&&(sockRead[0]!=EOT_CHAR)) {
               
                if(strncmp(sockRead,RETURN_ERROR,strlen(RETURN_ERROR))==0){
                    printf("No such file\n");
                    remove(sockWrite);
                    memset(sockRead,0,MAXLINE);
                    break;
                }

                write(file2,sockRead,sizeof(sockRead));
                memset(sockRead,0,sizeof(sockRead));
            }  
            memset(read_buffer,0, MAXLINE);
            memset(sockRead,0, MAXLINE);
            memset(sockWrite,0,MAXLINE);
            memset(buf2,0,MAXLINE);
            
            printf("Get complete\n");
            continue;   
        }

        else if (strncmp(read_buffer,CMD_EXIT,strlen(CMD_EXIT)) == 0){
            
            read_buffer[strlen(CMD_EXIT)]='\0';
            write(sfd,read_buffer,strlen(read_buffer));
            printf("%s\n", read_buffer);
            
            printf ("\n\tArnold says, Hasta la vista.\n\n");
            memset(read_buffer,0, MAXLINE);
            close(sfd);
            exit(0);
        }

        else if ((strncmp(read_buffer,"\n",1)==0)){
            continue;
        }

        else{
            printf("Client: Command not recognized.\n"
                   "\tThis is my surprised face.  :-()\n");
            continue;
        }

    }

    return 0;


}
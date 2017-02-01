/*************************************************************
** Program: fifo_client.c
** Author: Kai Shi
** Date: 1/21/2017
** Description: client/server processes that will do something 
				along the lines of an client/server pair 
				of programs through FIFO (aka. named pipe).

** Input: ./fifo_client (->) commands s.t. cd dir pwd etc.
** Output: Executing commands and get results
** Version: 1.0
**************************************************************/

#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <signal.h>
#include <inttypes.h>
#include <dirent.h>
#include "fifo.h"


char FifoclientReader_name[PATH_MAX]={'\0'};
char FifoclientWriter_name[PATH_MAX]={'\0'};
char Fifoserver_name[PATH_MAX]={'\0'};

int server_fd=0;
int client_rfd=0;
int client_wfd=0;

char read_buffer[2048]={'\0'};
char write_buffer[2048]={'\0'};

char buffer[2048]={'\0'};
char *r;

ssize_t dataSize_read;
int count =0;


void exitFunc(void)
{
    unlink(FifoclientReader_name);
    unlink(FifoclientWriter_name);
}

void sigHandler(int sig)
{
    exit(EXIT_SUCCESS);
}

int main(int argc, const char * argv[]){

	CREATE_CLIENT_READER_NAME( FifoclientReader_name , (int) getpid() );
    CREATE_CLIENT_WRITER_NAME( FifoclientWriter_name , (int) getpid() );

	if(mkfifo(FifoclientReader_name, FIFO_PERMISSIONS)==-1  && errno!=EEXIST ){
        perror("Fail to creat FIFO_1");
        exit(EXIT_FAILURE);
    }
    
    if(mkfifo(FifoclientWriter_name, FIFO_PERMISSIONS)== -1  && errno!=EEXIST){
        perror("Fail to creat FIFO_2");
        exit(EXIT_FAILURE);
    }

    CREATE_SERVER_FIFO_NAME(Fifoserver_name);

    if((server_fd=open(Fifoserver_name,O_WRONLY|O_NONBLOCK))==-1){
    	perror("Fail to open server FIFO");
        exit(EXIT_FAILURE);
    }


    sprintf(read_buffer,"%d\n",(int) getpid());
    write(server_fd,read_buffer,strlen(read_buffer));
    
    if((client_wfd = open ( FifoclientWriter_name, O_WRONLY ))==-1){
    	perror ("Failed to open Client Writer FIFO");
        exit(EXIT_FAILURE);
    } 

    (void) atexit(exitFunc);
    (void) signal(SIGINT, sigHandler);

    for(;;){

    	for(;;){

    		printf("%s", CLIENT_PROMPT);
    		fflush(stdout);
    		if(fgets(read_buffer,sizeof(read_buffer),stdin) == NULL)
    			break;


    		//ldir
    		if (strncmp (read_buffer, CMD_LOCAL_DIR, strlen(CMD_LOCAL_DIR)) == 0){
            	if(pclose(popen(CMD_LS_POPEN,"w"))==-1){
                	perror("Could not close the client pclose(ls) FIFO!");
                	exit(1);
            	}	
            	memset(read_buffer,0, sizeof(read_buffer));
            	continue;
        	}

        	//lpwd
        	else if (strncmp (read_buffer, CMD_LOCAL_PWD, strlen(CMD_LOCAL_PWD)) == 0){
            	printf("%s\n","The working directory for the client is:");
            	getcwd(buffer, 2048);
            	printf ( "\t%s\n",buffer);
            	memset(buffer,0, sizeof(buffer));
            	memset(read_buffer,0, sizeof(read_buffer));
            	continue;
        	}


        	//lcd
        	else if (strncmp(read_buffer,CMD_LOCAL_CHDIR,strlen(CMD_LOCAL_CHDIR)) == 0){
	            snprintf (buffer, (strlen(read_buffer))-(strlen(CMD_LOCAL_CHDIR)), "%s", read_buffer+(strlen(CMD_LOCAL_CHDIR)));
	            if(chdir(buffer) != -1){
	                getcwd ( buffer, 2048 );
	                printf("The working directory for the server is:\n");
	                printf ( "\t%s\n",buffer);
	            }
	            else{
	                printf("\tThere is no such directory!\n");
	            }
	            memset(buffer,0, sizeof(buffer));
	            memset(read_buffer,0, sizeof(read_buffer));
	            continue;
        	}

        	//lhome
        	else if(strncmp(read_buffer,CMD_LOCAL_HOME,strlen(CMD_LOCAL_HOME)) == 0){
	            r=getenv("HOME");
	            chdir(r);
	            printf("%s\n","The working directory for the client is:");
	            printf ("\t%s\n",r);
	            memset(read_buffer,0, sizeof(read_buffer));
	            continue;
	        }

            //help
	        else if(strncmp(read_buffer,CMD_HELP,strlen(CMD_HELP))== 0){
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

            //pwd
	        else if (strncmp(read_buffer,CMD_REMOTE_PWD,strlen(CMD_REMOTE_PWD))==0&&(strlen(read_buffer)-1==strlen(CMD_REMOTE_PWD))){
                if(write(client_wfd,CMD_REMOTE_PWD,sizeof(CMD_REMOTE_PWD))==-1){
                    perror("Could not write the pmd to the server!");
                    exit(EXIT_FAILURE);
                }
                break;	                   
	        }

	        //home
	        else if(strncmp(read_buffer,CMD_REMOTE_HOME,strlen(CMD_REMOTE_HOME))==0&&(strlen(read_buffer)-1==strlen(CMD_REMOTE_HOME))){
                if(write(client_wfd,CMD_REMOTE_HOME,sizeof(CMD_REMOTE_HOME))==-1){
                    perror("Could not write the home to the server!");
                    exit(EXIT_FAILURE);
                }
                break;         
	        }
	               
	        //cd 
	        else if (strncmp(read_buffer,CMD_REMOTE_CHDIR,strlen(CMD_REMOTE_CHDIR))==0){
                if(write(client_wfd,read_buffer,strlen(read_buffer)-1)==-1){
                    perror("Could not write the dir to the server!");
                    exit(EXIT_FAILURE);
                }
                break;    
	        }

	        
	        //dir
	        else if (strncmp(read_buffer,CMD_REMOTE_DIR,strlen(CMD_REMOTE_DIR))==0&&(strlen(read_buffer)-1==strlen(CMD_REMOTE_DIR))){
                count=1;
                if(write(client_wfd,CMD_REMOTE_DIR,sizeof(CMD_REMOTE_DIR))==-1){
                    perror("Could not write the dir to the server!");
                    exit(EXIT_FAILURE);
                }
                break;   
	        }

            //exit
            else if (strncmp(read_buffer,CMD_EXIT,strlen(CMD_EXIT))== 0 ){
                if(write(client_wfd,read_buffer,4)==-1){
                    perror("Could not write the exit to the server!");
                    exit(EXIT_FAILURE);
                }
                printf ("client exiting\n");
                exit(EXIT_SUCCESS);

            }

            else if (strncmp(read_buffer,"\n",1)==0){
                continue;
            }

            else{   
                printf("Client: Command not recognized!\n");
                continue;
            }

    	}

        if(count==1){
            
            client_rfd = open ( FifoclientReader_name, O_RDONLY );
            if (client_rfd == -1) {
                perror ("Failed to open Client Reader FIFO");
                exit(EXIT_FAILURE);
            }
            
            dataSize_read=read(client_rfd,write_buffer,sizeof(write_buffer)-1);
            printf("%s",write_buffer);
            while(dataSize_read!=0)
            {
                memset(write_buffer, 0, 1024);
                dataSize_read=read(client_rfd,write_buffer,sizeof(write_buffer)-1);
                printf("%s",write_buffer);
                
            }
            close(client_rfd);
            count=0;
            
        }
        
        
        
        else{
            
            client_rfd = open ( FifoclientReader_name, O_RDONLY );
            if (client_rfd == -1) {
                perror ("Failed to open Client Reader FIFO");
                exit(EXIT_FAILURE);
            }
            dataSize_read=read(client_rfd,write_buffer,sizeof(write_buffer)-1);
            write_buffer[dataSize_read]='\0';
            printf("\t%s\n",write_buffer);
            close(client_rfd);
        }

    } //first for end



}
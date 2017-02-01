/*************************************************************
** Program: fifo_server.c
** Author: Kai Shi
** Date: 1/21/2017
** Description: client/server processes that will do something 
				along the lines of an client/server pair 
				of programs through FIFO (aka. named pipe).

** Input: ./fifo_server
** Output: Wating for clients' connection requests.
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

char Fifoserver_name[PATH_MAX]={'\0'};
int isClient;
int server_fd;
int dummyFd;
pid_t client_pid = 0;

char pid_buf[2048];

char FifoclientReader_name[10000];
char FifoclientWriter_name[10000];

int client_wfd;
int client_rfd;

char read_buf[2048]={'\0'};

FILE *file_stream;

char tmp_dir[2048];
char tmp_pwd[2048];
char tmp_cd[2048];

char *r;

void at_exit(void)
{
    unlink(Fifoserver_name);
}

void sigSimple(int sig)
{
    killpg(0, SIGINT);
    exit(EXIT_SUCCESS);
}

int main(int argc, const char * argv[]) {

	CREATE_SERVER_FIFO_NAME(Fifoserver_name);

	if (mkfifo(Fifoserver_name,FIFO_PERMISSIONS)==-1 && errno != EEXIST){
        perror("Server FIFO create failed");
        exit(EXIT_FAILURE);
    }

    (void) atexit(at_exit);
    (void) signal(SIGINT, sigSimple);
    (void) signal(SIGCHLD, SIG_IGN);

    for(;;){
    	server_fd = open(Fifoserver_name,O_RDONLY);
    	if (server_fd==-1){
            perror("Could not open server read end of the FIFO!");
            exit(EXIT_FAILURE);
        }
        dummyFd=open(Fifoserver_name,O_WRONLY);
        if (dummyFd==-1){
            perror("Could not open server write end of the FIFO!");
            exit(1);
        }
        read(server_fd,pid_buf,2048);
        close(server_fd);
        close(dummyFd);

        client_pid=atoi(pid_buf);

        switch(fork()) {
            case -1:
                perror ("Fork failed");
                exit(EXIT_FAILURE); 

            case 0:
                goto CLIENT_SERVER; 

            default:
                break;
        }
    } //for end


CLIENT_SERVER:

    
    CREATE_CLIENT_READER_NAME(FifoclientReader_name, (int)client_pid);
    
    CREATE_CLIENT_WRITER_NAME(FifoclientWriter_name, (int)client_pid);

    client_wfd = open ( FifoclientWriter_name, O_RDONLY);
    if (client_wfd == -1) {
        perror ("Client Reader FIFO failed to open");
        exit(EXIT_FAILURE);
    }

    for(;;){

    	
        memset(read_buf, 0, 2048);
        memset(tmp_dir, 0, 2048);
        memset(tmp_pwd, 0, 2048);
        memset(tmp_cd, 0, 2048);
        read(client_wfd,read_buf,2048);

    	//dir
    	if (strncmp(read_buf,CMD_REMOTE_DIR,strlen(CMD_REMOTE_DIR)) == 0){
            file_stream=popen(CMD_LS_POPEN, "r");
            client_rfd = open(FifoclientReader_name,O_WRONLY);
            if(client_rfd == -1){
                perror ("Client Reader FIFO failed to open");
                exit(EXIT_FAILURE);
            }
            if(file_stream!=NULL){
                while(fgets(tmp_dir,sizeof(tmp_dir),file_stream)){
                    write(client_rfd, tmp_dir, (int)strnlen(tmp_dir, 2048));}
            }
            close(client_rfd);
    	} 

    	//pwd
    	else if (strncmp(read_buf,CMD_REMOTE_PWD, strlen(CMD_REMOTE_PWD)) == 0){
            getcwd (tmp_pwd, 1024 );
            
            client_rfd = open(FifoclientReader_name,O_WRONLY);
            if (client_rfd == -1) {
                perror("Client Reader FIFO failed to open");
                exit(EXIT_FAILURE);
            }
            write(client_rfd, tmp_pwd, strlen(tmp_pwd));
            close(client_rfd);
    	}

    	//cd
    	else if (strncmp ( read_buf, CMD_REMOTE_CHDIR, strlen(CMD_REMOTE_CHDIR) ) == 0){
            snprintf ( tmp_cd, strlen(read_buf)-2, "%s", read_buf+3);
            if(chdir(tmp_cd)==-1){
                perror("Failed to change directory");
            }
            
            client_rfd = open(FifoclientReader_name,O_WRONLY);
            if (client_rfd == -1) {
                perror ("Client Reader FIFO failed to open");
                exit(EXIT_FAILURE);
            }
            memset(tmp_cd, 0, 2048);
            getcwd(tmp_cd, 2048);
            write(client_rfd, tmp_cd, strlen(tmp_cd));
            close(client_rfd);    
    	}

    	//home
    	else if (strncmp ( read_buf, CMD_REMOTE_HOME, strlen(CMD_REMOTE_HOME)) == 0){
            r=getenv("HOME");
            chdir(r);  
            client_rfd = open(FifoclientReader_name, O_WRONLY);
            if (client_rfd == -1) {
                perror ("Client Reader FIFO failed to open");
                exit(EXIT_FAILURE);
            }
            write(client_rfd, r, strlen(r));
            close(client_rfd);
    	}

    	//exit
    	else if (strncmp ( read_buf, CMD_EXIT, strlen(CMD_EXIT)) == 0){
            exit(EXIT_SUCCESS);
            
    	}


    }  //for end

}




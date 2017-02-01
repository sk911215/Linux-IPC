/*************************************************************
** Program: socket_server.c
** Author: Kai Shi
** Date: 1/24/2017
** Description: client/server processes that will do something 
                along the lines of an client/server pair 
                of programs through Socket.

** Input: ./socket_server / ./socket_server (ip address) or (port num)
** Output: Waiting for clients' connection requests.
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


int main(int argc, char * argv[]) {

    int i,t;
	int listenfd;
    int maxfd;
    int nclients;
    char *r;
    
    int judge;
    int nready; 
    int connfd;
    int sockfd;
    char read_buf[MAXLINE];
    char buf1[MAXLINE];
    char buf2[MAXLINE];
    char buf3[MAXLINE];
    char Sockwrite[MAXLINE];

    struct sockaddr_in servaddr;
    struct sockaddr_in cliaddr;
	
	int client[MAX_CLIENTS];
    
    fd_set rset;
	fd_set allset;
    socklen_t clilen;

    int file1;
    int file2;

    FILE *stream;


    char client_dir[5][MAXLINE];
	char server_dir[MAXLINE];


    getcwd(server_dir, MAXLINE );
     

	int opt, port;
    int isPort =0;

	while((opt = getopt(argc, argv, "p:")) != -1){
        switch(opt){
            case 'p':
                port = atoi(optarg);
                printf("%d\n",port);
                isPort=1;
                break;
            default:
                break;
        }
        
    }

    if (isPort==0) {
        port=10086;
        printf("%d\n",port);
    }


    if((listenfd = socket(AF_INET, SOCK_STREAM, 0)) < 0){
        perror("failed to create socket");
        exit(1);
    }

    memset(&servaddr, '\0', sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(port);

    

    if(bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr)) != 0){
        perror("failed to bind socket");
        exit(1);
    }

    if (listen(listenfd, LISTENQ) == -1) {
        perror("error : cannot listen at the given socket!\n");
        exit(1);
    }

    maxfd = listenfd; 
    nclients = 0;

    for (i = 0 ; i < MAX_CLIENTS ; i++) {
        client[i] = -1;// A value of -1 indicates available entry
    }
    
    // Clean out the set of all file descriptors.
    FD_ZERO(&allset);

    // Add the listen file desc into the list of all descriptors.
    FD_SET(listenfd, &allset);

    for(;;){

    	rset=allset;
    	nready = select(maxfd + 1, &rset, NULL, NULL, NULL);

    	if (nready == -1) {
            perror("select returned failure");
            exit(EXIT_FAILURE);
        }

// Check the listenfd first.  If it is ready, then it is
// a new client connection request.
/*****************************************************************************/

        if (FD_ISSET(listenfd, &rset)){

        	// A new client connection...  Woooo  Whoooo!!!
            nclients ++;
            printf("server: a new client connection request\n");
            clilen = sizeof(cliaddr);
            connfd = accept(listenfd, (struct sockaddr *) &cliaddr, &clilen);

            for (i = 0 ; i < MAX_CLIENTS ; i++) {
                if (client[i] < 0) {
                    client[i] = connfd;
                    getcwd(client_dir[i], MAXLINE );
                    break;
                }
            }

            if (i == MAX_CLIENTS) {
                printf("!!! TOO MANY CLIENTS  !!!!\n");
                exit(EXIT_FAILURE);
            }

            FD_SET(connfd, &allset);

            if (connfd > maxfd) {
                maxfd = connfd;
            }
            
            
            if (--nready <= 0) {
                continue;
            }

        } //if (FD_ISSET(listenfd, &rset))

/*****************************************************************************/
// Check file desc to see if there is ready data.
         for (i = 0 ; i <= MAX_CLIENTS ; i++){
         	sockfd = client[i];
            if (sockfd < 0) {
                continue;
            }

            // This checks to see if this file desc is one of the
            // ready ones.
            if (FD_ISSET(sockfd, &rset)){
                memset(read_buf,0,MAXLINE);
                read(sockfd, read_buf, sizeof(read_buf));
                printf(">%s<\n",read_buf);

                //pwd
                if (strncmp(read_buf,CMD_REMOTE_PWD,strlen(CMD_REMOTE_PWD))==0){
                    if(chdir(client_dir[i]) != -1 ) {
                        getcwd(client_dir[i], MAXLINE);
                    }
                    getcwd(Sockwrite,MAXLINE);
                    write(sockfd, Sockwrite, sizeof(Sockwrite));

                    if(chdir(server_dir) != -1 ){
                        getcwd(server_dir, MAXLINE);
                    }
                    memset(read_buf,0, MAXLINE);
                    memset(Sockwrite,0, MAXLINE);
                }

                //home
                else if (strncmp(read_buf,CMD_REMOTE_HOME,strlen(CMD_REMOTE_HOME))==0){
                    if (chdir(client_dir[i]) != -1 ){
                        getcwd( client_dir[i], MAXLINE);
                    }
                    r=getenv("HOME");
                    chdir(getenv("HOME"));
                    getcwd( client_dir[i], MAXLINE);
                    sprintf(Sockwrite, "%s", r);
                    write(sockfd, Sockwrite, sizeof(Sockwrite));
                    if (chdir (server_dir) != -1 ) {
                        getcwd(server_dir, MAXLINE);
                    }
                    memset(read_buf,0, MAXLINE);
                    memset(Sockwrite, 0, MAXLINE);
                }

                //cd
                else if (strncmp(read_buf,CMD_REMOTE_CHDIR,strlen(CMD_REMOTE_CHDIR))==0){           
                    if (chdir(client_dir[i]) != -1 ) { 
                        getcwd( client_dir[i], MAXLINE);
                    }

                    snprintf (Sockwrite, strlen(read_buf)-strlen(CMD_REMOTE_CHDIR)+1, "%s", read_buf+strlen(CMD_REMOTE_CHDIR));
                    
                    judge=chdir(Sockwrite);

                    if(judge<0){
                        memset(Sockwrite,0, MAXLINE);
                        sprintf(Sockwrite, "%s", RETURN_ERROR);
                        write(sockfd, Sockwrite, sizeof(Sockwrite));
                    }
                    else if(judge==0){
                        memset(Sockwrite,0, MAXLINE);
                        getcwd(Sockwrite,MAXLINE);
                        
                        write(sockfd, Sockwrite, sizeof(Sockwrite));
                        getcwd(client_dir[i],MAXLINE);
                    }


                    if (chdir ( server_dir ) != -1 ) {
                        getcwd( server_dir, MAXLINE);
                    }
                
                    memset(read_buf,0, MAXLINE);
                    memset(Sockwrite,0, MAXLINE);
                }

                //dir
                else if(strncmp(read_buf, CMD_REMOTE_DIR, strlen(CMD_REMOTE_DIR)) == 0){
                    if (chdir(client_dir[i]) != -1 ) {
                        getcwd( client_dir[i], MAXLINE);
                    }

                    stream = popen (CMD_LS_POPEN, "r" );
                    
                    if (stream != NULL ){
                        while(fgets(Sockwrite,MAXLINE,stream)){
                            printf("%s",Sockwrite);
                            write(sockfd, Sockwrite, sizeof(Sockwrite));
                            memset(Sockwrite,0, sizeof(Sockwrite));
                        }
                        
                    }
                   
                    sprintf (Sockwrite, "%c", EOT_CHAR);
                    printf("%c",Sockwrite[0]);
                    write(sockfd, Sockwrite, sizeof(Sockwrite));
  
                    if(pclose(stream)==-1){
                        perror("Could not close the client pclose(dir)!");
                       
                        exit(1);
                    }
                    
                    
                    memset(read_buf,0, MAXLINE);
                    memset(Sockwrite,0, MAXLINE);
                }

                //put
                else if (strncmp(read_buf,CMD_PUT_TO_SERVER,strlen(CMD_PUT_TO_SERVER))==0){
                    if(chdir(client_dir[i]) != -1 ){
                        getcwd( client_dir[i], MAXLINE);
                    }
                    read_buf[strlen(read_buf)]='\0';
                    snprintf (Sockwrite, strlen(read_buf)-strlen(CMD_PUT_TO_SERVER)+2, "%s", read_buf+strlen(CMD_PUT_TO_SERVER));
                    //printf("%s\n",Sockwrite);
                    
                    file1=open(Sockwrite,O_CREAT | O_WRONLY, SEND_FILE_PERMISSIONS);
                    memset(Sockwrite,0, MAXLINE);
              
                    while((read(sockfd, Sockwrite, sizeof(Sockwrite))!=0)&&(Sockwrite[0]!=EOT_CHAR)){
                        write(file1,Sockwrite,sizeof(Sockwrite));
                        memset(Sockwrite,0, MAXLINE);
                    }   
                    close(file1);
                    memset(read_buf, 0, MAXLINE);
                    t=0;
                    break;
                    
                }

                //get
                else if (strncmp(read_buf,CMD_GET_FROM_SERVER,strlen(CMD_GET_FROM_SERVER))==0){
                    if(chdir(client_dir[i] ) != -1 ){
                        getcwd( client_dir[i], MAXLINE);
                    }
                    read_buf[strlen(read_buf)]='\0';
                    snprintf(Sockwrite, strlen(read_buf)-strlen(CMD_GET_FROM_SERVER)+2, "%s", read_buf+strlen(CMD_GET_FROM_SERVER));

                    file2 = open (Sockwrite, O_RDONLY);
                    memset(Sockwrite,0, MAXLINE);
                    if (file2<0){
                        strcpy (buf2,  RETURN_ERROR);
                        write(sockfd, buf2, strlen(buf2));
                        memset(buf2, 0, MAXLINE);
                        continue;
                    }

                    for(;;){
                        memset(Sockwrite, 0, MAXLINE);
                        t=read(file2, buf2, MAXLINE );
                        if(t>0){
                            write(sockfd, buf2, strlen(buf2));
                            memset(buf2, 0, MAXLINE);   
                        }
                        else{     
                            sprintf (buf3, "%c", EOT_CHAR);
                            sleep(5);
                            write(sockfd, buf3, strlen(buf3));
                            printf("    send file complete\n");
                            break;
                        }
                    }

                    memset(read_buf, 0, MAXLINE);
                    close(file2);     
                    t=0;

                }

                //exit
                else if (strncmp(read_buf,CMD_EXIT,strlen(CMD_EXIT))==0){
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                    printf("    server: client closed connection\n");
                    nclients --;
                }

                else{
                    close(sockfd);
                    FD_CLR(sockfd, &allset);
                    client[i] = -1;
                    printf("    server: client closed connection\n");
                    nclients --;    
                }

                if(--nready <= 0) {
                    break;
                }


            }

         }//for (i = 0 ; i <= MAX_CLIENTS ; i++)
         
/*****************************************************************************/
    } //for


    return 0;

}
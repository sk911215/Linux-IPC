/*************************************************************
** Program: myword_freq.c
** Author: Kai Shi
** Date: 1/21/2017
** Description:Program reads text from stdin or from a file 
			   named on the command line and generates output 
			   from a 6-stage filter of processes connected 
			   by pipe. 
** Input: myword_freq [filename]
** Output: Each words' frequences and sorted by frequence
** Version: 1.0
**************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <errno.h>

int main(int agrc, char* argv[]) {

	pid_t pid1=-1;
	int pipe_1[2];

	pid_t pid2=-1;
	int pipe_2[2];

	pid_t pid3=-1;
	int pipe_3[2];

	pid_t pid4=-1;
	int pipe_4[2];

	pid_t pid5=-1;
	int pipe_5[2];

	if (argv[1] != NULL)
		freopen(argv[1], "r", stdin);

	pipe(pipe_1);
	{
		pid1=fork();
		switch(pid1){
			case -1:
				perror("failed creating fork for pipe_1");
				return(EXIT_FAILURE);
				break;
			case 0:
				if(dup2(pipe_1[1],STDOUT_FILENO)<0){
					perror("failed dup2 pipe_1[1] and stdout");
                    return(EXIT_FAILURE);
				}
				close(pipe_1[0]);
				execlp("sed", "sed", "s/[^a-zA-Z]/ /g", (char *) NULL);
				break;
			default:
				if (dup2(pipe_1[0], STDIN_FILENO) < 0) {
                    perror("failed dup2 pipe_1[1] and stdin");
                    return(EXIT_FAILURE);
                }
                close(pipe_1[1]);
                break;
		}
	}

	pipe(pipe_2);
	{
		pid2=fork();
		switch(pid2){
			case -1:
				perror("failed creating fork for pipe_2");
				return(EXIT_FAILURE);
				break;
			case 0:
				if(dup2(pipe_2[1],STDOUT_FILENO)<0){
					perror("failed dup2 pipe_2[1] and stdin");
                    return(EXIT_FAILURE);
				}

				close(pipe_2[0]);
				execlp("tr", "tr", "[A-Z]","[a-z]",(char *) NULL);
				break;
			default:
				if (dup2(pipe_2[0], STDIN_FILENO) < 0) {
                    perror("failed dup2 pipe_2[0] and stdin");
                    return(EXIT_FAILURE);
                }
                close(pipe_2[1]);
                break;
		}
	}

	pipe(pipe_3);
	{
		pid3=fork();
		switch(pid3){
			case -1:
				perror("failed creating fork for pipe_3");
				return(EXIT_FAILURE);
				break;
			case 0:
				if(dup2(pipe_3[1],STDOUT_FILENO)<0){
					perror("failed dup2 pipe_3[1] and stdin");
                    return(EXIT_FAILURE);
				}
				close(pipe_3[0]);
				execlp("awk", "awk", "{ for(i = 1; i <= NF; i++) { print $i;} }",(char *) NULL);
				break;
			default:
				if (dup2(pipe_3[0], STDIN_FILENO) < 0) {
                    perror("failed dup2 pipe_3[0] and stdin");
                    return(EXIT_FAILURE);
                }
                close(pipe_3[1]);
                break;
		}
	}

	pipe(pipe_4);
	{
		pid4=fork();
		switch(pid4){
			case -1:
				perror("failed creating fork for pipe_4");
				return(EXIT_FAILURE);
				break;
			case 0:
				if(dup2(pipe_4[1],STDOUT_FILENO)<0){
					perror("failed dup2 pipe_4[1] and stdin");
                    return(EXIT_FAILURE);
				}
				close(pipe_4[0]);
				execlp("sort", "sort",(char *) NULL);
				break;
			default:
				if (dup2(pipe_4[0], STDIN_FILENO) < 0) {
                    perror("failed dup2 pipe_4[0] and stdin");
                    return(EXIT_FAILURE);
                }
                close(pipe_4[1]);
                break;
		}
	}

	pipe(pipe_5);
	{
		pid5=fork();
		switch(pid5){
			case -1:
				perror("failed creating fork for pipe_5");
				return(EXIT_FAILURE);
				break;
			case 0:
				if(dup2(pipe_5[1],STDOUT_FILENO)<0){
					perror("failed dup2 pipe_5[1] and stdin");
                    return(EXIT_FAILURE);
				}
				close(pipe_5[0]);
				execlp("uniq", "uniq","-c", (char *) NULL);
				break;
			default:
				if (dup2(pipe_5[0], STDIN_FILENO) < 0) {
                    perror("failed dup2 pipe_5[0] and stdin");
                    return(EXIT_FAILURE);
                }
                close(pipe_5[1]);
                execlp("sort", "sort","-n","-r", (char *) NULL);
                break;
		}
	}

	return(EXIT_SUCCESS);
}

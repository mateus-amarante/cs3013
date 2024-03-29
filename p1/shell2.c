/*
 * Mateus Amarante Araujo <mamarantearaujo@wpi.edu>
 */

//HEADERS
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

//implements wait function
#include <sys/types.h>
#include <sys/wait.h>

#include <sys/time.h>
#include <sys/resource.h>

//custom library
#include "auxfnc.h"
#include "bgprocess.h"

//#include <mcheck.h>// for memory leakage tracking


#define TRUE  1
#define FALSE 0

//MAIN
int main(int argc, char **argv) {
	
	//mtrace();//to check memory leakage problems
	
	//Initialize prev_rusage with 0 values
	struct rusage usage;
	int status = 0;//return from wait function
	int background = FALSE;//flag that indicate if a command is supposed to run on background or not
	
	char str[129];//input string
	char* cmd_args[32]; //vector of strings (arguments for the shell)
	struct timeval init, end; //checkpoint to measure wall-clock time
	
	bgprocessLL bgpLL = init_bgprocessLL();//store the background processes (LL: linked list)
	
	
	while(1) {
		printf(">");//prompt character
		
		//Get input
		if(fgets(str,129,stdin)==NULL){//EOF or error (to be able to pipe input files)
			wait4bgprocess(&bgpLL);
			break;
		}

		//parse commands in str to cmd_args (array of string) to be used in exec functions
		int n_args = args_from_str(str, cmd_args);
		
		//check if cmd_args has built-in commands
		int builtin_effect = treat_builtin_cmds(cmd_args, n_args, &bgpLL);
		
		if(builtin_effect==CONTINUE) continue;
		else if (builtin_effect==BREAK) break;
		//else: continue normally
		
		//Check if the process is supposed to run in background
		if(strcmp(cmd_args[n_args-1],"&") == 0) {
			free(cmd_args[n_args-1]);//transparent to the rest
			cmd_args[n_args-1]=NULL;//necesary condition for exec functions
			n_args--;
			background = TRUE;
		}else
			background = FALSE;
		
		gettimeofday(&init,NULL);//get time right before creating the child
		pid_t pid = fork(); //create a new process
		
		//CHILD
		if (pid == 0) { 
			
			if(execvp(cmd_args[0], cmd_args) == -1){ //error with de command
				perror(NULL);//print error message
				//free_args(cmd_args);//free memory allocaded in args_from_cmdline function
				exit(EXIT_FAILURE);
			}

		}else if (pid > 0) { //PARENT
			if(background == FALSE) {//false
				while(1) {
					
					int pid_done = wait3(&status, 0, &usage);//wait for any children
					gettimeofday(&end, NULL);//get end time
					
					if(pid_done!=pid) {//it's not the calling non-background child (a background process has finished)
						
						bgprocess bgp = remove_bgprocess(&bgpLL, pid_done);//get the information about it

						print_bgprocess(bgp, "completed");//show which one has finished
						print_report(diff_time(bgp.init_time,end), usage, status);//print running status
						free_bgprocess_name(&bgp);//to avoid memory leak
					}else {//it's not the calling non-background child (pid_done==pid)
						print_report(diff_time(init,end), usage, status);//print running status
						break;
					}
				}
			}else {//It is a background process
				bgprocess bgp = init_bgprocess(pid, init, cmd_args[0],0,NULL);//bgp null initialization
				
				add2bgprocessLL(&bgpLL, bgp);//add the information to the linked list
				
				free_bgprocess_name(&bgp);//avoid memory leaks
				
				print_bgprocess(*bgpLL.first, "");//print the information about the background process
			}
			
			free_args(cmd_args);//free memory allocaded in args_from_str function
		
		}else { //error
			perror(NULL);//error in fork
			free_args(cmd_args);//free memory allocaded in args_from_str function
			exit(EXIT_FAILURE);
		}
		
		//Check for completed background processes and print their status
		check_background_processes(&bgpLL, WNOHANG);
		
	}
	
	//free_args(cmd_args);//free memory allocaded in args_from_str function
	return 0;
}

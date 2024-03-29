#include "bgprocess.h"
#include "auxfnc.h"

#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>

#include <sys/time.h>
#include <sys/resource.h>
#include <sys/types.h>
#include <sys/wait.h>

/*Create a bgprocess instance
 * 
 * It allocates memory for name dinamically. Then, you need to call free_bgprocess_name
 *
 * @see bgprocessLL
 * @see free_bgprocess_name
 */
 //TODO: make it simpler, without dynamic allocation
bgprocess init_bgprocess(pid_t pid, struct timeval init_time, char* name, int number, bgprocess* next) {
	bgprocess bgp;
	
	//assign values
	bgp.pid = pid;
	bgp.init_time = init_time;
	
	//allocate space for name and copy it
	if(name!=NULL) {
		bgp.name = (char*) malloc ((strlen(name)+1)*sizeof(char));
		strcpy(bgp.name, name);
	}else
		bgp.name = NULL;
	
	bgp.next = next;// for linked list (used in bgprocessLL)
	
	bgp.number = number;//used in bgprocessLL
	
	return bgp;
}

/* Starts empty bgprocess (zero and NULL values)
 */
bgprocess init_null_bgprocess() {
	struct timeval t;
	t.tv_sec = 0; t.tv_usec = 0;
	return init_bgprocess(0, t, NULL, 0, NULL);
}

/*Copy bgprocess from "other" to the return value
 * 
 * remember to call free_bgprocess_name
 * 
 * @param other: copy from
 * 
 * @return output copy
 */
bgprocess copy_bgprocess(bgprocess other) {
	return init_bgprocess(other.pid, other.init_time, other.name, other.number, other.next);
}

/* Same as copy_bgprocess but the copy is dynamically allocated and returns the pointer
 * 
 *  the return pointer must be freed by free_bgprocess function
 */
bgprocess* dynamic_copy_bgprocess(bgprocess other) {
	bgprocess* b = (bgprocess*)malloc(sizeof(bgprocess));
	
	//free_bgprocess_name(b);//FIXME: is it necessary? I don't think so
	*b = copy_bgprocess(other);
	
	return b;
}

/* Free dynamically allocated bgprocess (by dynamic_copy_bgprocess, for example)
 */
void free_bgprocess(bgprocess* bgp) {
	free_bgprocess_name(bgp);
	if(bgp != NULL) {
		free(bgp);
	}
}

/* Free name pointer of bgprocess variable
 * 
 * TODO: make "name" static (simpler)
 */
void free_bgprocess_name(bgprocess* bgp) {
	if(bgp->name != NULL) {
		free(bgp->name);
	}
}

/*Initialize bgprocessLL variable with zero and NULL values
 */
bgprocessLL init_bgprocessLL() {
	bgprocessLL bg;
	
	bg.first = NULL;
	bg.size = 0;
	
	return bg;
}

/* Add bgprocess "bgp" into a bgprocessLL "bgpLL"
 * 
 * Add bgp on the beginning of the list and pops from the beggining as well, like a stack
 * */
int add2bgprocessLL(bgprocessLL* bgpLL, bgprocess bgp) {
	
	if(bgpLL->first == NULL){//empty
		//assign bgp to first
		bgpLL->first = dynamic_copy_bgprocess(bgp);
		bgpLL->first->next = NULL;
	}else {//it already has at least one element
		bgprocess* old_first = bgpLL->first;
		bgpLL->first = dynamic_copy_bgprocess(bgp);
		bgpLL->first->next = old_first;
	}
	
	bgpLL->size++;
	
	bgpLL->first->number = bgpLL->size;//number in th stack
	
	return 0;
}

/* Remove bgprocess "bgp" from a bgprocessLL "bgpLL" based on the pid
 * 
 * Returns NULL when it doesn't find it
 * */
bgprocess remove_bgprocess(bgprocessLL* bgpLL, pid_t pid) {
	
	bgprocess* node = bgpLL->first;//kind a iterator
	
	if(node->pid == pid) {//If it is the first one
		bgprocess* new_first = bgpLL->first->next;
		
		bgprocess b = copy_bgprocess(*bgpLL->first); //get the first element
		free_bgprocess(bgpLL->first);
		
		bgpLL->first = new_first;//reset first with the old bgpLL->first->next
		
		bgpLL->size--;
		
		return b;
	}else {
		
		while(node->next!=NULL) { //while it is in the list
			if (node->next->pid == pid) { //is it the next
				bgprocess* old_next = node->next;//temp
				
				bgprocess b = copy_bgprocess(*old_next);
				
				node->next = node->next->next;//reset node->next->nex
				
				free_bgprocess(old_next);
				bgpLL->size--;
				
				return b;
			}
			node = node->next;
		}
	}
	
	return *node;//NULL
}

/* Print bgprocess content + custom complement (TODO)
 * 
 * [number] pid name complement
 */
void print_bgprocess(bgprocess proc, const char* complement) {
	printf("[%d] %d %s %s\n", proc.number, proc.pid, proc.name, (complement!=NULL)?complement:"");
}

/* Print the contents in the linkedlist (in the oposite order. Stack) TODO: make it a queue
 */
void print_bgprocessLL(bgprocessLL bgpLL) {
	
	bgprocess* node = bgpLL.first;
	
	while(node!=NULL) {
		print_bgprocess(*node, "");
		node = node->next;
	}
}

/* "Wait" for background processes in bgpLL
 * 
 * It returns imediatally if no one is done
 * It also print the report of the finished bgprocess 
 */
void check_background_processes(bgprocessLL * bgpLL, int wait_option) {
	int status = 0;
	struct rusage usage;
	struct timeval end;
	
	while(bgpLL->size > 0) {
		int pid_done = wait3(&status, wait_option, &usage); // It returns imediatally if no one is done (returns 0 when it happens)
		gettimeofday(&end, NULL);//get end time
		
		if(pid_done <= 0) break;// //TODO: treat -1 condition
		else {
			bgprocess bgp = remove_bgprocess(bgpLL, pid_done);//remove from LL
			print_bgprocess(bgp, " completed");//print the name
			print_report(diff_time(bgp.init_time,end), usage, status);//print the report
			free_bgprocess_name(&bgp);//free name
		}
	}
}

/* Wait all wait4bgprocess in bgpLL finish
 */
void wait4bgprocess(bgprocessLL *bgpLL) {
	if(bgpLL->size>0) {
		printf("Waiting for:\n");
		print_bgprocessLL(*bgpLL);
		check_background_processes(bgpLL, 0);
	}
}

/* Take care of special built-in programs
 * 
 * @param cmd_args: vector of string taken form args_from_str fucntion
 * @param n_args: number of arguments in cmd_args
 * @param bgpLL: pointer to the current background process LinkedList
 * 
 * @return result (3 possible results)
 * 
 * NO_BUILTIN:	it is not a built-in function request
 * CONTINUE:	simply request to ask for the next command
 * BREAK:		stop the program
 * 
 */
int treat_builtin_cmds(char** cmd_args, int n_args, bgprocessLL *bgpLL) {
	//SPECIAL CASES (TODO: put special cases in a function)
	if(n_args==0) {
		check_background_processes(bgpLL, WNOHANG);
		free_args(cmd_args);
		return CONTINUE;
	}//no argument
	if(strcmp(cmd_args[0],"exit") == 0 && n_args == 1) {
		wait4bgprocess(bgpLL);
		free_args(cmd_args);
		return BREAK;
	}//exit command
	if(strcmp(cmd_args[0],"cd") == 0) {//change directory built in command
		check_background_processes(bgpLL, WNOHANG);
		if(n_args==1)chdir(".");//default value (in this case, the current directory)
		else chdir(cmd_args[1]);
		free_args(cmd_args);
		return CONTINUE;
	}
	if(strcmp(cmd_args[0],"jobs") == 0 && n_args == 1) {//change directory built in command
		check_background_processes(bgpLL, WNOHANG);
		print_bgprocessLL(*bgpLL);
		free_args(cmd_args);
		return CONTINUE;
	}
	
	return NO_BUILTIN;
}

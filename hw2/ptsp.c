
/* TSP Problem using MultiProcessing */
/* To build the code, I have to use process creation, join, pipe, signal concepts properly. */

#include <stdio.h>
#include <stdlib.h>  // for using malloc function
#include <signal.h>
#include <unistd.h> // this header is for using execl function.
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h> 
#include <sys/wait.h> // for wait funtion to prevent terminating children process 

#define MAXSIZE 50

int **route = {0,} ; // I need to allocate width and height dynamically using malloc. 
int path[MAXSIZE] ;
int covered[MAXSIZE] ;
long tot_weight = 0L ;
unsigned long long count = 0ULL ; // At 'count', children will count how many routes they covered.
int pipes_p[12][2] ; // parent' pipes[12][2]
int pipes_c[12][2] ; // children' pipes[12][2]
int lines = -1 ; // line count from tsp file. 
long min = -1L ; // Do I have to use malloc to initialize with -1 to give child array?

void
handler(int sig){			/* signal Handler */
	int i;
	if (sig == SIGINT){
		// printf(" The best solution is %ld ( )\n", min );
		// printf(" And the number of covered routes are %llu \n", count );
		// then terminate children processes
		for(i = 0 ; i < lines ; i++) {	free(route[i]) ;  }
		free(route) ; 

		exit(0);

	}

	//else if (sig == SIGCHLD){	}
}

void
_travel(int idx) { //professor's code
	int i ;

	if(idx == lines) {
		tot_weight += route[path[lines-1]][path[0]] ;
		if(min == -1L || min > tot_weight) {
			min = tot_weight ; 
			
			printf("%ld (", tot_weight) ;
			for (i = 0 ; i < lines ; i++)
				printf("%d ", path[i]) ;
			printf("%d)\n", path[0]) ;

		}
		tot_weight -= route[path[lines-1]][path[0]] ;
	}
	else{
		for (i = 0 ; i < lines ; i++) {
			if (covered[i] == 0){
				path[idx] = i ;
				covered[i] = 1 ;
				tot_weight += route[path[idx-1]][i] ;
				_travel(idx+1) ;
				tot_weight -= route[path[idx-1]][i] ;
				covered[i] = 0 ;
			}
		}
	}
}

void
travel(int start) { // professor's code
	path[0] = start ; 
	covered[start] = 1 ;
	_travel(1) ;
	covered[start] = 0 ;
}

void
parent_proc(int idx){
	char buf[256] ;
	char* str = "Please let me check. I'm parent\n" ;
	//signal(SIGCHLD, handler) ;
	//read(pipes_c[id][0], &tmp ,sizeof(tmp) ) ;
	close(pipes_c[idx][1]) ;
	close(pipes_p[idx][0]) ;
	read(pipes_c[idx][0], buf, strlen(buf)) ;
	
	write(pipes_p[idx][1],str, strlen(str) ) ;
	sleep(3) ;
	printf("min num is %ld", min) ;
	buf[0] = 0x0 ;
	close(pipes_c[idx][0]) ;
	close(pipes_p[idx][1]) ;
}

void
child_proc(int idx){
	char buf[256] ;
	char* str = "test from child\n" ; 
	travel(idx) ;
	close(pipes_c[idx][0]) ;
	close(pipes_p[idx][1]) ;
	
	write(pipes_c[idx][1], str , strlen(str)) ;
	sleep(2) ;
	read(pipes_p[idx][0], buf, strlen(buf)) ;
	
	buf[0] = 0x0 ;
	exit(0);
}

int
main(int argc, char** argv){
	// argv[0] = exe command , argv[1] = tsp file instance , argv[2] = number of children 

	pid_t child[12] ; // Maximum 12 child processes should be available.
	pid_t child_pid ;
	int i = 0, j = 0; // variables for loops or something
	int exit_code ;
	int t = 0 ; 
	char input[1024] ;

	signal(SIGINT, handler);
	if(argc != 3){

		printf("Wrong Input! Usage : %s <tsp_instance> <number of children> \n", argv[0]);
		exit(0);
	}

	printf("starting point. Master process : %d \n ", getpid()); //delete this later
	FILE * fp = fopen(argv[1], "r");

	/* To get a length of file instance */
	while(!feof(fp)){
		fgets(input, sizeof(input), fp) ;
		lines++ ; 
	}
	input[0] = 0x0 ; //clear the input buffer

	int **route = (int**) malloc(sizeof(int*) * lines) ;
	for(i = 0 ; i< lines ; i++){
		route[i] = (int*) malloc(sizeof(int) * lines) ;
		memset(route[i] , 0, sizeof(int) * lines) ;
	}

	/* THIS IS TO STORE THE DATA INTO THE MATRIX FROM A FILE*/
	for (i = 0 ; i < lines ; i++) {
		for (j = 0 ; j < lines ; j++) {
			fscanf(fp, "%d", &t) ;
			route[i][j] = t ;
		}
	}

	fclose(fp) ; 

	/* create bi-directional pipes to communicate parent and children */
	if (pipe(*pipes_p) != 0 || pipe(*pipes_c) !=0 ){
		perror("Create pipes Error! \n") ;
		exit(1) ;
	}
	
	/* creating Children Processes */
	for (i = 0 ; i < atoi(argv[2]) ; i++){
		if ( (child_pid =fork() ) < 0) { /* fork fail */
			perror (" Fork failed ! \n") ;
			exit(1) ;
		}
		else if ( child_pid == 0) { /* Children */
			printf("Parent : %d , child[%d] : %d\n", getppid(), i, getpid() /* child[i] */ ) ;
			// then I have to connect children with parent via pipe(write)
			//travel(i) ;
			child_proc(i) ;
			//signal(SIGCHLD, handler) ; // to give the solution and to be reforked
			//exit(0) ;	
		}
		else { /* master(parent) process */
			// using pipe read file operation, take the solution from children
			parent_proc(i) ;
			//waitpid(child[i], NULL, 0) ; 
			//wait(0x0) ;
			printf("Process %d spawned process %d\n", getpid(), child_pid) ;
		}
		wait(0x0) ;
	}
	
	//wait(&exit_code);
	

	// printf(" The best solution is %d \n", tot_weight );
	// printf(" And the number of covered routes are %llu \n", count );
	//wait(&exit_code) ;

	/* Deallocating the route table then terminate normally */
	for(i = 0 ; i < lines ; i++) {	free(route[i]) ;  }
	free(route) ;
	printf("Matrix has been deallocated. Bye! \n");
	exit(0) ;

}


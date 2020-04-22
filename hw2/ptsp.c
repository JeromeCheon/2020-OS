/* TSP Problem using MultiProcessing */
/* To build the code, I have to use process creation, join, pipe, signal concepts properly. */

#include <stdio.h>
#include <stdlib.h>  // for using malloc function
#include <signal.h>
#include <unistd.h> // this header is for using execl function.
#include <errno.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/wait.h> // for wait funtion to prevent terminating children process 

#define MAXSIZE 50

int **route = {0x0, }; // I need to allocate width and height dynamically using malloc. 
int path[MAXSIZE] ;
int used[MAXSIZE] ;
unsigned long long length = 0ULL ;
unsigned long long count ; // At 'count', children will count how many routes they covered.
int pipes[2] ; // read and write pipe
int lines = -1 ; // line count from tsp file. 

void
handler(int sig){
	int i;
	if (sig == SIGINT){
		// printf(" The best solution is %llu \n", length );
		// printf(" And the number of covered routes are %llu \n", count );
		for(i = 0 ; i < MAXSIZE ; i++) {	free(route[i]) ;  }
		free(route) ; 

		exit(0);

	}
}

int
main(int argc, char** argv){
	// argv[0] = exe command , argv[1] = tsp file instance , argv[2] = number of children 
	pid_t child[12] ; // Maximum 12 child processes should be available. 
	int i = 0, j = 0; // variables for loops or something
	int exit_code ;
	int t = 0 ; 
	char buf[1024] ;

	signal(SIGINT, handler);
	if(argc != 3){

		printf("Wrong Input! Usage : %s <tsp_instance> <number of children> \n", argv[0]);
		exit(0);
	}

	int **route = (int**) malloc(sizeof(int*) * MAXSIZE) ;
	for(i =0 ; i< MAXSIZE ; i++){
		route[i] = (int*) malloc(sizeof(int) * MAXSIZE) ;
		memset(route[i] , 0, sizeof(int) * MAXSIZE) ;
	}

	printf("starting point. Master process : %d \n ", getpid()); //delete this later
	FILE * fp = fopen(argv[1], "r");

	/* To get a length of file instance */
	while(!feof(fp)){
		fgets(buf, sizeof(buf), fp) ;
		lines++ ; 
	}

	/* THIS IS TO STORE THE DATA INTO THE MATRIX FROM A FILE*/
	for (i = 0 ; i < lines ; i++) {
		for (j = 0 ; j < lines ; j++) {
			fscanf(fp, "%d", &t) ;
			route[i][j] = t ;
		}
	}
	fclose(fp) ; 

	/* creating Children Processes */
	for (i = 0 ; i < atoi(argv[2]) ; i++){
	  	child[i] = fork();
		if (child[i] < 0) { /* fork fail */
			perror (" Fail forking ! \n") ;
			exit(1) ;
		}
		else if ( child[i] == 0) { /* Children */
			printf("Parent : %d , child[%d] : %d\n", getppid(), i, getpid() /* child[i] */ ) ;
			// then I have to connect children with parent via pipe(write)
			sleep(1) ;
			exit(0) ;
		}
		else { /* master(parent) process */
			pid_t term_pid ;
			int exit_code ;
			// using pipe read file operation, take the solution from children
			wait(0x0) ; 
			printf("Process %d spawned process %d\n", getpid(), child[i]) ;
			//printf("Process %d is finished with exit code %d \n", term_pid, exit_code) ;
		}
	}
	
	//wait(0x0);
	

	// printf(" The best solution is %d \n", );
	// printf(" And the number of covered routes are %llu \n", count );
	//wait(&exit_code) ;
	for(i = 0 ; i < MAXSIZE ; i++) {	free(route[i]) ;  }
	free(route) ;

	printf("Matrix has been deallocated. Bye! \n");
	
	exit(0) ;

}


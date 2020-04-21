		/* TSP Problem using MultiProcessing */
/* To build the code, I have to use process creation, join, pipe, signal concepts properly. */

#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h> // this header is for using execl function.
#include <errno.h>
#include <fcnfl.h> 
#include <string.h>
#include <sys/types.h> 
#include <sys/wait.h> // for wait funtion to prevent terminating children process 

#define MAXSIZE 50

int **route = {0x0, }; // I need to allocate width and height dynamically using malloc. 
unsigned long long count ; // At 'count', children will count how many routes they covered.

void
handler(int sig){
	int i;
	if (sig == SIGINT){
		// printf(" The best solution is %d \n", );
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
	int i=0, j=0, k=0; // variables for loops or something
	int t = 0 ; 
	if(argc != 3){
		perror("Wrong Input! Usage : %s <tsp_instance> <number of children> \n", argv[0]);
		exit(0);
	}

	signal(SIGINT, handler);
	int **route = (int**) malloc(sizeof(int*) * MAXSIZE) ;
	for(i =0 ; i< MAXSIZE ; i++){
		route[i] = (int*) malloc(sizeof(int) * MAXSIZE) ;
		memset(route[i] , NULL, sizeof(int) * MAXSIZE) ;
	}
 	// memset(route , NULL , sizeof(route)) ;  // might be able to be done like this.

	printf("starting point. Master process : %d \n ", getpid());
	FILE * fp = fopen(argv[1], "r");
	
	/* THIS IS TO STORE THE DATA INTO THE MATRIX FROM A FILE
	   for (i = 0 ; i < M_SIZE ; i++) {
		for (j = 0 ; j < M_SIZE ; j++) {
			fscanf(fp, "%d", &t) ;
			route[i][j] = t ;
		}
	}
	fclose(fp) ; */

	for (i = 1 ; i <= atoi(argv[2]) ; i++){
			child[i] = fork(); // create children processes

	}
	wait(0x0);
	//




}


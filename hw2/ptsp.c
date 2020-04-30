#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/wait.h>

#define MAXSIZE 50 // Maximum city number

int **route = {0,} ; // I need to allocate width and height dynamically using malloc. 
int path[MAXSIZE] ;
int covered[MAXSIZE] ;
int tot_weight = 0 ;
unsigned long long count = 0ULL ; // At 'count', children will count how many routes they covered.
int lines = -1 ; // line count from tsp file. 
long min = -1L ;
short terminate = 0 ; // Initialize as FALSE 
int *end_prefix ={ 0x0 } ;
int prefix_weight =0 ;

void
parent_handler(int sig){			/* Handler for master process */
	int i;
	if (sig == SIGINT){
		// for each slave pid, kill(INT, pid) ;
		terminate = 1 ;
		for(i = 0 ; i < lines ; i++) {	free(route[i]) ;  }
		free(route) ; 
		exit(0);

	}
}

void
child_handler(int sig){ /* Handler for children processes */
	int i ;
	if(sig == SIGINT){
		terminate = 1 ;
		exit(0) ;
	}
}
void
child_proc(int* prefix, int *p /* pipe file descripter */){
	signal(SIGINT, child_handler) ;
    close(p[0]) ;
    

}
void
init_prefix(int** route){
    /* initialize prefix from the input number of nodes */
    int i ;
	for(i=0 ; i < lines - 12 ; i++){
        path[i] = i ;
       // covered[i] = 1 ;
    }
}

void
next_prefix(){
    path[0] = 0 ;
    covered[0] = 1 ;
    _next_prefix(1) ;
    covered[0] = 0;
}

void
_next_prefix(int idx){ // prefix[0], prefix[1], ..., prefix[N-13]
    int i ;
    int last_prefix = lines -12 ;

    while (idx < last_prefix) {
        if(idx == last_prefix -1 ){ // when 0,1,2,3,4~15
            
            for(i = 0 ; i < lines ; i++){
                if (covered[i] == 0 && path[idx] < i) {
                    path[idx] = i ;
                    covered[i] = 1 ;
                    prefix_weight += route[path[idx-1]][i] ;
                    if(i == lines - 1){ // when 0,1,2,3,15
                        idx-- ;
                        path[idx]++ ;
                        covered[idx] =0 ;
                        break ;
                    }
                    idx++ ;
                    
                    prefix_weight -= route[path[idx-1]][i] ;
                    break ;
                }
            }
        }
        else{ // when 0,1,2,3,~
            for (int i =0 ; i < lines ; i++) {
                if (covered[i] == 0) {
                    path[idx] = i ;
                    covered[i] = 1 ;
                    prefix_weight += route[path[idx-1]][i] ;
                    idx++ ;
                    break ;
                }
            }
        }
    }
    prefix_weight -= route[path[idx-1]][i] ;
    covered[i] = 0 ;

}

void
spawning(int* prefix) {
	int pipes[2] ; // 0 : reader / 1 : writer
    pid_t child_pid ;
	/* create unnamed pipes to communicate parent and children */
	if (pipe(pipes) != 0 ){
		perror("Create pipes Error! \n") ;
		exit(1) ;
	}
	if((child_pid = fork()) == 0){ /* child */
		child_proc(prefix, pipes) ;
	}
	else{ /* parent */
		close(p[1]) ;
		current_child_info(child_pid, p[0]) ;
	}
}

void
current_child_info(pid_t pid, int *p){
	
}

int
main(int argc, char** argv){
	// argv[0] = exe command , argv[1] = tsp file instance , argv[2] = number of children 

	pid_t pid ;
	int i = 0, j = 0, t= 0; // variables for loops or something
	int exit_code, children =0 ;
	char f_input[1024] ;


	signal(SIGINT, parent_handler);
	if(argc != 3){
		printf("Wrong Input! Usage : %s <tsp_instance> <number of children> \n", argv[0]);
		exit(0);
	}

    /* To get a length of file instance */
    FILE * fp = fopen(argv[1], "r");
	while(!feof(fp)){
		fgets(f_input, sizeof(f_input), fp) ;
		lines++ ; 
	}
	f_input[0] = 0x0 ; //clear the input buffer

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

    init_prefix(route) ;

	/* creating Children Processes */
	while(path != end_prefix){
        if ( children == argv[2] ) {
            pid = wait() ;
            children-- ;
//            read_result_and update_best(pid) ; 
        }
        if (terminate == 0) {
            spawning(path) ;
            children++ ;
            next(path) ;
        }
        else{ // when terminate is 1(True)
            if (children == 0){
                /* Deallocating the route table then terminate normally */
                for(i = 0 ; i < lines ; i++) {    free(route[i]) ;  }
                free(route) ;
                printf(" The best solution is %ld \n", tot_weight );
                printf(" And the number of covered routes are %llu \n", count );
                printf("Matrix has been deallocated. Bye! \n");
                exit(0) ;
            }
        }
    }
}


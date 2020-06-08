#include <stdio.h>
#include <string.h>  	// for string functions.
#include <unistd.h>		// read and close function
#include <errno.h>		// perror
#include <stdlib.h>
#include <fcntl.h>		// open system call
#include <sys/types.h>
#include <pthread.h>	// for using pthread API

#define TRUE 1		// indicators of cycle in graph
#define FALSE 0

unsigned long node[20] = { 0, } ; // A total nodes are 10 threads + 10 mutexes as an assumption
int visited[20] = { 0, } ; // for checking if there can be made any cycle 
int m[10][10] = { 0, } ; // to indicate if there is adjacent node or not

int idxOftid = 0 ;
int idxOfmutex = 10 ;
/********* To construct a lockgraph algorithm using resource allocation ********/
//		 threads : processes    ||		mutexes : resources


void			// function to create Process nodes
creatNode(unsigned long tid) {
	node[idxOftid] = tid ;
	idxOftid += 1 ;
}
void			// function to create Resource nodes 
creatMtres(pthread_mutex_t *lockV){
	node[idxOfmutex] = (unsigned long)lockV ;
	idxOfmutex += 1 ;
}

void
addEdge(unsigned long tid, pthread_mutex_t *lockV){
	int i, j ;
	for ( i = 0; i < idxOftid ; i++ ){ // checking if there is already existed
		if( node[i] == tid )
			break ;
	}
	if ( node[i] != tid ){		// unless create a new node of process(thread)
		creatNode(tid) ;
		i = idxOftid ;
	}
	for ( j = 10 ; j < idxOftid ; j++ ){ // checking if there is already existed
		if ( node[j] == (unsigned long)lockV )
			break ;	
	}
	if ( node[j] != (unsigned long)lockV ){	// unless create a new node of resource(mutex)
		creatMtres(lockV) ;
		j = idxOfmutex ;
	}
	m[i][j] = 1 ;	 // make an edge between tid and lockV
}

void			// reverse with addEdge. 
releaseLock(unsigned long tid, pthread_mutex_t *lockV){
	int i, j ;
	for ( i = 0; i < idxOftid ; i++ ){
		if( node[i] == tid )
			break ;
	}
	if ( node[i] != tid ){
		creatNode(tid) ;
		i = idxOftid ;
	}
	for ( j = 10 ; j < idxOftid ; j++ ){
		if ( node[j] == (unsigned long)lockV )
			break ;	
	}
	if ( node[j] != (unsigned long)lockV ){
		creatMtres(lockV) ;
		j = idxOfmutex ;
	}
	m[i][j] = 0 ;
	// then how can I remove thread Nx ???
}
/***** Should make a function which checks any cycles in the graph *****/

// void dfs(){}  // for traversal the graph in height order
// int cycle(){ return TRUE or FALSE }  // TRUE -> Deadlock detected 

int
main(int argc, char** argv){
	int fd ;
	unsigned long process ;		 // to store the identifier of thread
	pthread_mutex_t *resource ;  // to store the mutex variable addresses
	if ((fd = open(".ddtrace", O_RDONLY | O_SYNC)) == -1 ){ // open named pipe
		perror("fail to open the pipe. \n") ;
		exit(1) ;
	}

	while(1) {
		int len ;
		char buf[256] ;
		if ((len = read(fd, buf , sizeof(buf)) ) == -1)
			break ;
		if (len > 0){
			sscanf(buf, "%lu %p", &process , &resource) ;
			
			addEdge(process, resource) ;
			printf("thread[%lu] made an edge with mutex(%p)\n", process, resource) ;
			buf[0] = 0x0 ;
		}
	}

	close(fd) ;
	return 0 ;
}

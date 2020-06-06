#include <stdio.h>
#include <string.h>  	// for string functions.
#include <unistd.h>		// open and close function
#include <errno.h>		// perror
#include <stdlib.h>
#include <fcntl.h>		// O_RDONLY
#include <sys/types.h>
#include <pthread.h>	// for using pthread API

typedef struct {
	pthread_t tid ;
	int weight ;
	pthread_mutex_t* ptr ;
} mutexInfo ;

typedef struct _Vertex{
	mutexInfo* node ;
	struct _Vertex* next ;
} Vertex ;

typedef struct _lockgraph{
	Vertex *order[10] ;
} lockgraph ;

lockgraph* l = 0x0 ;

Vertex*
newVertex(mutexInfo* t) {
	Vertex* new = (Vertex*)malloc(sizeof(Vertex)) ;
	new->node = t ;
	new->next = NULL ;
	return new ;
}

void
addEdge(lockgraph* l, mutexInfo* from, mutexInfo* index){
	int i = 0 ;
	Vertex* new = newVertex(from) ;
	new->node = l->order[from->weight]->node ;
	new->next = l->order[from->weight] ;
	l->order[from->weight]->next = new ;
}

void
graphInit(lockgraph* l, int amount){
	int i = 0 ; 
	//l->order = (Vertex*)malloc( sizeof(Vertex) * amount ) ;
	for( i = 0 ; i < amount ; i++){
		l->order[i] = NULL ;
	}

}

int
main(int argc, char** argv){
	int fd ;
	int i = 0;
	if ((fd = open(".ddtrace", O_RDONLY | O_SYNC)) == -1 ){ // open named pipe
		perror("fail to open the pipe. \n") ;
		exit(1) ;
	}
	printf("test before making lockgraph") ;	
	l = (lockgraph*)malloc(sizeof(lockgraph)) ;
	graphInit(l, 10) ;

	while(1) {
		int len ;
		if ((len = read(fd, (struct mutexInfo*)l->order[i]->node , sizeof(mutexInfo)) ) == -1)
			break ;
		if (len > 0){
			printf("read thread[%lu] the mutex address (%p)\n", (unsigned long)(l->order[i]->node->tid), l->order[i]->node->ptr ) ;	
		}
		i++ ;
	}
	/*fp = popen(".ddtrace", "r") ;
	if (fp == 0x0 )
		return 1 ;

	while(1) {
		// lock graph algorithm. to detect any deadlocks in target programs
		char s[128] ;
		int len ;
		// read data as a string 
		if ((len = read(fp, s, 128)) == -1)
			break ;
		//sscanf(s, "%llu", &tmp) ; 
		if (len > 0)
			printf("%s\n", s) ;

	}*/
	close(fd) ;
	free(l) ;
	return 0 ;
}

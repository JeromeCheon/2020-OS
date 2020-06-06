#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 	// for replacing Pthread API
#include <dlfcn.h> 	// for using shared library symbols.
#include <string.h>
#include <errno.h>   // for error handling 
#include <execinfo.h>
#include <unistd.h>		// open and close function
#include <fcntl.h>  	// O_WRONLY
#include <sys/types.h>
// replace the original function to redefined function of pthread API
// those are 'pthread_mutex_lock' and 'pthread_mutex_unlock'

typedef struct {
	pthread_t tid ; //  to contain caller's thread ID
	int weight ;
	pthread_mutex_t* ptr ;  //  to contain the pointer address of mutex variable
} mutexInfo ;

mutexInfo* mi = 0x0 ;
int count = 1 ;

int
pthread_mutex_lock(pthread_mutex_t *mutex){
	int (*lockp)(pthread_mutex_t *) = NULL ; // function pointer
	mi = (mutexInfo*)malloc(sizeof(mutexInfo)) ;
	mi->tid = pthread_self() ;
	mi->ptr = mutex ;
	mi->weight = count++ ; // for giving some temperary weight of edge

	char * error ;
	int fd, i ;

	lockp = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;
	if ((error = dlerror()) != 0x0)
		exit(1) ;
	int ptr = lockp(mutex) ; // always be 0 if it's successful 

	//fwrite vs fputs . mutex value has to be sent via writing in the pipe. 
	char buf[128] ;
	snprintf( buf, 128, "thread[%lu] acquired pthread_mutex_lock(%p). \n", (unsigned long)(mi->tid), mi->ptr ) ; 
	fputs(buf, stderr) ;
	buf[0] = 0x0 ;

	if ( (fd = open(".ddtrace", O_WRONLY | O_SYNC )) == -1 ){ // write only
		perror("fail to open the pipe. \n") ;
		exit(1) ;
	}
	// just send the pointer address of mutex 
	//while(1){	 // this is useless
		write(fd, (struct mutexInfo*)&mi, sizeof(mi)) ;
		
	//}
	if( close(fd) == -1 ){ // if fails to close the pipe
		fprintf(stderr, "something happened that interrupted closing pipe. \n") ;
	}

	free(mi) ;
	//pthread_mutex_unlock(mutex) ; // it might be necessary to give a next lock
	return ptr ;
}


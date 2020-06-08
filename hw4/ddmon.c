#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h> 	// for replacing Pthread API
#include <dlfcn.h> 	// for using shared library symbols.
#include <string.h>
#include <errno.h>   // for error handling 
#include <execinfo.h>
#include <unistd.h>		// write and close function
#include <fcntl.h>		// open system call
#include <sys/types.h>
// replace the original function to redefined function of pthread API
// those are 'pthread_mutex_lock' and 'pthread_mutex_unlock'

typedef struct {
	pthread_t tid ; //  to contain caller's thread ID
	pthread_mutex_t* ptr ;  //  to contain the pointer address of mutex variable
} mutexInfo ;

mutexInfo* mi = 0x0 ;

int
pthread_mutex_lock(pthread_mutex_t *mutex){
	int (*lockp)(pthread_mutex_t *) = NULL ; // function pointer
	mi = (mutexInfo*)malloc(sizeof(mutexInfo)) ;
	mi->tid = pthread_self() ;
	mi->ptr = mutex ;
	char * error ;
	int fd ;

	lockp = dlsym(RTLD_NEXT, "pthread_mutex_lock") ;
	if ((error = dlerror()) != 0x0)
		exit(1) ;
	int ptr = lockp(mutex) ; // always be 0 if it's successful 

	char buf[128] ;
	snprintf( buf, 128, "thread[%lu] acquired lock value(%p). \n", (unsigned long)(mi->tid), mi->ptr ) ; 
	fputs(buf, stderr) ;
	buf[0] = 0x0 ;

	if ( (fd = open(".ddtrace", O_WRONLY | O_SYNC )) == -1 ){ // write only
		perror("fail to open the pipe. \n") ;
		exit(1) ;
	}
	// just send the structure  of mutex 
	sprintf(buf, "%lu %p", (unsigned long)(mi->tid), mi->ptr) ;
		if( write(fd, buf, sizeof(buf)) == -1 )
				fprintf(stderr, "fail to write \n") ;
		
	if( close(fd) == -1 ){ // if fails to close the pipe
		fprintf(stderr, "something happened that interrupted closing pipe. \n") ;
	}

	free(mi) ;
	return ptr ;
}

int
pthread_mutex_unlock(pthread_mutex_t *mutex){
	int (*unlockp)(pthread_mutex_t *) = NULL ; // function pointer
	mi = (mutexInfo*)malloc(sizeof(mutexInfo)) ;
	mi->tid = pthread_self() ;
	mi->ptr = mutex ;
	char * error ;
	int fd ;

	unlockp = dlsym(RTLD_NEXT, "pthread_mutex_unlock") ;
	if ((error = dlerror()) != 0x0)
		exit(1) ;
	int ptr = unlockp(mutex) ; // always be 0 if it's successful 

	char buf[128] ;
	snprintf( buf, 128, "thread[%lu] released lock value(%p). \n", (unsigned long)(mi->tid), mi->ptr ) ; 
	fputs(buf, stderr) ;
	buf[0] = 0x0 ;

/*	if ( (fd = open(".ddtrace", O_WRONLY | O_SYNC )) == -1 ){ // write only
		perror("fail to open the pipe. \n") ;
		exit(1) ;
	}
	
	sprintf(buf, "%lu %p", (unsigned long)(mi->tid), mi->ptr) ;
		if( write(fd, buf, sizeof(buf)) == -1 )
				fprintf(stderr, "fail to write \n") ;
		
	if( close(fd) == -1 ){ // if fails to close the pipe
		fprintf(stderr, "something happened that interrupted closing pipe. \n") ;
	}
*/
	free(mi) ;
	return ptr ;
}

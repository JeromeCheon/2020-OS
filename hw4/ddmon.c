#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <dlfcn.h>
#include <string.h>
#include <execinfo.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>

// replace the original function to redefined function of pthread API
// those are 'pthread_mutex_lock' and 'pthread_mutex_unlock'


int
pthread_mutex_lock(pthread_mutex_t *mutex){
	static __thread int n_locks = 0 ;
	//n_locks += 1 ;
	void *(*lockp)(pthread_mutex_t *t) ;  		// function pointer
	char * error ;
	FILE* fd ;

	lockp =dlsym(RTLD_NEXT, "pthread_mutex_lock") ;
	if ((error = dlerror()) != 0x0)
		exit(1) ;
	char* ptr = lockp(&t) ;

	// make FIFO named pipe	
	if (mkfifo(".ddtrace", 0666)){
		// errno : integer variable. it contains some error information occured by a function of previous library. 
		if(errno != EEXIST){ // The named file doesn't exist
			perror("fail to make fifo: \n") ;
			exit(1) ;
		}
	}
	fd = popen(".ddtrace", "w") ; // write only 
	if ( fp == 0x0 ){
		perror("fail to open the pipe. \n") ;
		exit(1) ;
	}

	/*if ( n_locks == 1 ){
		int i ;
		void * arr[10] ;
	}*/

	//n_locks -= 1 ;
	pclose(fd) ;
	return 0 ;
}

int
pthread_mutex_unlock(pthread_mutex_t *mutex){
	static __thread int n_unlocks = 0 ;
	//n_unlocks += 1 ;
	void *(*unlockp)(pthread_mutex_t *t) ; 		// function pointer
	char * error ;
	FILE* fd ;
	unlockp =dlsym(RTLD_NEXT, "pthread_mutex_unlock") ;
	if ((error = dlerror()) != 0x0)
		exit(1) ;
	char* ptr = unlockp(&t) ;

	// make FIFO named pipe	
	if (mkfifo(".ddtrace", 0666)){
		// errno : integer variable. it contains some error information occured by a function of previous library. 
		if(errno != EEXIST){ // The named file doesn't exist
			perror("fail to make fifo: ") ;
			exit(1) ;
		}
	}

	fd = popen(".ddtrace", "w") ; // write only 
	
	//if ( n_unlocks == 1 ){
		int i ;
		void * arr[10] ;
	//}

	//n_unlocks -= 1 ;
	pclose(fd) ;
	return 0 ;
}

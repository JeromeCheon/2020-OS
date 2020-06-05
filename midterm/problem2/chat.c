#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <string.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <errno.h>

/* TODO: you can freely edit this file */

void
handler(int sig){
	if(sig == SIGINT) {
		printf("Goodbye. \n") ;
		exit(0) ;
	}
}

void*
receiver(void* arg){
	char* fname ;
	fname = (char*)arg ;

	if(mkfifo(fname, 0666)){
		if (errno != EEXIST){
			perror("fail to open fifo: ") ;
			exit(1) ;
		}
	}

	int fd1 = open(r_fifo, O_RDONLY | O_NONBLOCK) ;
	while(1){
		char s[256] ;
		int len ;
		if( (len = read(fd1, s, 256) == -1)){
			printf("There are no processes opened. \n") ;
			exit(1) ;
		}
		if(len > 0)
			printf("%s\n", s) ;
		close(fd1) ;
		return 0; 
	}
}

void*
sender(void* arg) {
	char* fname ;
	fname = (char*)arg ;

	if(mkfifo(fname, 0666)){
		if (errno != EEXIST){
			perror("fail to open fifo: ") ;
			exit(1) ;
		}
	}

	int fd2 = open(w_fifo, O_WRONLY | O_NONBLOCK) ;
	while(1) {
		char s[256] ; 
		char newline ;
		int i = 0, j=0 ;
		
		while (i <256 && ((s[i] = getchar()) != EOF && s[i++] != '\n')) ;
		s[i-1] = 0x0 ;
		if(s[0] == 0x0)
			break ;
		for (i=0; i<256 ;){
			i += write(fd2, s+i, 256) ;
		}
	}
	close(fd2) ;
	return 0 ;
}

int
main (int argc, char ** argv)
{
	if (argc != 3) {
		fprintf(stderr, "Wrong number of arguments\n") ;
		exit(1) ;
	}
	signal(SIGINT, handler) ;

	pthread_t reader, writer ;

	char * r_fifo = argv[1] ;
	char * w_fifo = argv[2] ;

	/*TODO*/
	pthread_create(&reader, 0x0, (void*)receiver, (void*)r_fifo) ;
	pthread_create(&writer, 0x0, (void*)writer, (void*)w_fifo) ;
	

	pthread_join(&reader, 0x0) ;
	pthread_join(&writer, 0x0) ;

}

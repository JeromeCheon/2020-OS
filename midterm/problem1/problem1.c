#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <error.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>

/* TODO */
int pipe[2] ;

void
parent_proc(){
	char* buf = 0x0 ;
	ssize_t s ;
	size_t len = 0 ;
	
	close(pipes[0]) ;
	while((s=getline(&buf, &len, stdin)) != -1) {
		buf[s-1] = 0x0 ;

		ssize_t sent = 0 ;
		char * data = buf ;
		// Sorry professor and thank you for all.
		// JaeHong Cheon (Jerome)
	}
}
void
child_proc(){

}

char * 
read_exec (char * exe)
{
/* TODO*/
	pid_t child ;
	if( (child = fork()) ==0) {
		int fd = open("exe2.c", O_RDONLY | O_CREAT, 0644 ) ;
		dup2(fd, 1 ) ;
		execl("./exe2.c", "hello", "hello", NULL ) ;
		close(fd) ;
	}
	else{ //parent
		wait(0x0) ;
	}

}	

int 
main (int argc, char ** argv)
{
	if (argc != 2) {
		fprintf(stderr, "Wrong number of arguments\n") ;
		exit(1) ;
	}

	char * s ;
	s = read_exec(argv[1]) ;
	printf("\"%s\"\n", s) ;
	free(s) ;
	exit(0) ;
}

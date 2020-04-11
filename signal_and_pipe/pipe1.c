#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <sys/wait.h>

int pipes[2] ;

	void
parent_proc()
{
	char * buf = 0x0 ;
	ssize_t s ;
	size_t len = 0 ;

	close(pipes[0]) ;

	while ((s = getline(&buf, &len, stdin)) != -1) {
		buf[s - 1] = 0x0 ;

		ssize_t sent = 0 ;
		char * data = buf ;		

		while (sent < s) {
			sent += write(pipes[1], buf + sent, s - sent) ;
		}

		free(buf) ;
		buf = 0x0 ;
		len = 0 ;
	}
	close(pipes[1]) ;
}

	void
child_proc()
{
	char buf[32] ;
	ssize_t s ;
	// 자식 프로세스가 write pipe를 닫아. write pipe 는 pipes[1]
	close(pipes[1]) ;
	// read하는데 pipes[0]면 buf에 있는 내용을 읽어오겠다는 의미. reader의 역할
	while ((s = read(pipes[0], buf, 31)) > 0) {
		buf[s + 1] = 0x0 ;
		printf("> %s\n", buf) ;
	}
	exit(0) ;
}

	int
main()
{
	pid_t child_pid ;
	int exit_code ;

	if (pipe(pipes) != 0) {
		perror("Error") ;
		exit(1) ;
	}
	printf("%d %d\n", pipes[0], pipes[1]) ;

	child_pid = fork() ;
	if (child_pid == 0) {
		child_proc() ;
	}
	else {
		parent_proc() ;
	}
	wait(&exit_code) ;

	exit(0) ;
}

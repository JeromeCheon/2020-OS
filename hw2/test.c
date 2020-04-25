#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>

int main(void){
	pid_t pid ;
	int i = 0;
	int pare = 0;

	printf("start! \n ") ;

	for (i = 0 ; i < 5 ; i++){
		pid = fork() ;

		if(pid < 0){
			printf("error! \n" ) ;
			exit(1) ;
		}
		else if(pid == 0){
			pare = getppid() ;
			printf("Parent : %d , Child : %d\n", pare, getpid()) ;
			return 0;
		}
	}
}

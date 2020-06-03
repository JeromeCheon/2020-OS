#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <pthread.h>

typedef struct _Vertex{
	pthread_mutex_t node ;
	struct _Vertex* next ;
} Vertex ;

typedef struct _adjList{
	Vertex* head ;
} adjList ;

typedef struct _lockgraph{
	adjList* order ;
} lockgraph ;

Vertex*
newVertex(int index) {}

void
addEdge(lockgraph* l, int from, int index){}

lockgraph*
graphInit(){}

void
freeGraph(lockgraph* l){}

int
main(int args, char** argv){
	int fd = open(".ddtrace", 0_RDONLY | O_SYNC) ;

	while(1) {
		// lock graph algorithm. to detect any deadlocks in target programs
		char s[128] ;
		int len ;
		// read data as %
		if ((len = read(fd, s, 128)) == -1)
			break ;
		if (len > 0)
			printf("%s\n", s) ;

	}
	close(fd) ;

	return 0 ;
}

#include <stdio.h>
#include <stdlib.h>

int m[5][5] ;

int path[5] ;
int used[5] ;
int length = 0 ;
int min = -1 ;
int count ;
void _travel(int idx) {
	int i ;
	count ++ ;
	if (idx == 5) {
		length += m[path[4]][path[0]] ;
		if (min == -1 || min > length) {
			min = length ;

			printf("%d (", length) ;
			for (i = 0 ; i < 5 ; i++) 
				printf("%d ", path[i]) ;
			printf("%d)\n", path[0]) ;	
		}
		length -= m[path[4]][path[0]] ;
	}
	else {
		for (i = 0 ; i < 5 ; i++) {
			if (used[i] == 0) {
				path[idx] = i ;
				used[i] = 1 ;
				length += m[path[idx-1]][i] ;
				_travel(idx+1) ;
				length -= m[path[idx-1]][i] ;
				used[i] = 0 ;
			}
		}
	}

}

void travel(int start) {
	path[0] = start ;
	used[start] = 1 ;
	_travel(1) ;
	used[start] = 0 ;
}

int main() {
	int i, j, t ;

	FILE * fp = fopen("table5.txt", "r") ;

	for (i = 0 ; i < 5 ; i++) {
		for (j = 0 ; j < 5 ; j++) {
			fscanf(fp, "%d", &t) ;
			m[i][j] = t ;
		}
	}
	fclose(fp) ;

	for (i = 0  ; i < 5 ; i++) 
		travel(i) ;
	printf("min way: %d\n", min) ;

	printf("Values which is stored at the end : (") ;
	for (i = 0 ; i < 5 ; i++) 
		printf("%d ", path[i]) ;
	printf("%d)\n", path[0]) ;

	printf(" route count number : %d \n", count) ;
}

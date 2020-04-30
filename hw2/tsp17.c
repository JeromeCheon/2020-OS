#include <stdio.h>
#include <stdlib.h>

int m[17][17] ;

int path[17] ;
int used[17] ;
int length = 0 ;
int min = -1 ;

void _travel(int idx) {
	int i ;
	//idx는 1부터 시작해서 재귀적으로 돈다.  
	if (idx == 17) {
		length += m[path[16]][path[0]] ; // travel 함수에선 항상 idx를 0을 주는데. 
		if (min == -1 || min > length) {
			min = length ;
			// 중복을 제외하고, 첫번째 인덱스를 기준으로 해서 출력이 될것.
			printf("%d (", length) ; 
			for (i = 0 ; i < 17 ; i++) 
				printf("%d ", path[i]) ;
			printf("%d)\n", path[0]) ;	
		}
		length -= m[path[16]][path[0]] ;
	}
	else {
		for (i = 0 ; i < 17 ; i++) {
			if (used[i] == 0) {
				path[idx] = i ;
				used[i] = 1 ;
				length += m[path[idx-1]][i] ; 
				_travel(idx+1) ; //여기서 재귀적으로 들어감. idx를 늘려줌
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

	FILE * fp = fopen("gr17.tsp", "r") ;

	for (i = 0 ; i < 17 ; i++) {
		for (j = 0 ; j < 17 ; j++) {
			fscanf(fp, "%d", &t) ;
			m[i][j] = t ;
		}
	}
	fclose(fp) ;

	for (i = 0  ; i < 17 ; i++) 
		travel(i) ;
}

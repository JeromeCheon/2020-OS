#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h> 
#include <sys/wait.h>

#define TRUE 1
#define FALSE 0
#define MAXSIZE 50 // Maximum city number

int **route = {0,} ; // I need to allocate width and height dynamically using malloc. 
//int path[MAXSIZE] ;
int covered[MAXSIZE] ;
int tot_weight = 0 ;
unsigned long long count = 0ULL ; // At 'count', children will count how many routes they covered.
int lines = -1 ; // line count from tsp file. 
long min = -1L ;
short terminate = FALSE ; // Initialize as FALSE 

/***************** Signal Handler *******************/
/***************** Prefix functions *******************/
void
swap(int* x, int* y){
	int temp ;
	temp = *x ;
	*x = *y ;
	*y = temp ;
}
void
next_prefix(int n, int k, int depth, int *path){
	int i ;
	int prefix[lines-12] ;
	if( depth == k /* lines-12  k */ ){ // 이게 마지막 배열 부분이면 
		//printf("For test in next_prefix: ") ;
		if ( path[0] == 0 ){
		for (i = 0; i < k ; i++ ){
			//prefix[i] = path[i] ; // 0, 1, 2 ~ 0, 1, 3 ~ ~
			printf(" %d ", path[i] ) ;		
		}
		printf("\n") ;
		}
		tot_weight++ ;
		return ;
		//return prefix ;  그리고 나서 바뀐 프리픽스 출력. 
	}
	for ( i = depth ; i < n /* N */; i++){
		// 재귀로 인해  for문 중첩 효과로 숫자를 변화해감. 
		// 근데 여기서 covered를 어떻게 조건 넣고 break 하지..?
		// 또 자식 프로세스 끝나고 다시 이 함수 next_prefix(0)를 호출했을 때 앞에서 구한 것 다 건너뛸 수 있나..? 
			swap(&path[i], &path[depth]) ;
			
			next_prefix(n, k, depth+1, path) ;
			swap(&path[i], &path[depth]) ;
	}
	//printf("depth != lines-12, then prefix:%d \n", ) ;
	//return prefix ;
}

int
main(int argc, char** argv){
	// argv[0] = exe command , argv[1] = tsp file instance , argv[2] = number of children 

	pid_t pid ;
	int i = 0, j = 0, t= 0; // variables for loops or something
	int *prefix ; // prefix는 0부터 N-13까지의 인덱스를 가진 배열
	int children =0 ;
	char f_input[1024] ;

	//signal(SIGINT, parent_handler);
	if(argc != 3){
		printf("Wrong Input! Usage : %s <tsp_instance> <number of children> \n", argv[0]);
		exit(0);
	}

    /* To get a length of file instance */
    FILE * fp = fopen(argv[1], "r");
	while(!feof(fp)){
		fgets(f_input, sizeof(f_input), fp) ;
		lines++ ; 
	}
	f_input[0] = 0x0 ; // clear the input buffer
	int *path  = (int*)calloc(lines, sizeof(int)) ;
	route = (int**) malloc(sizeof(int*) * lines) ;
	for(i = 0 ; i< lines ; i++){
		route[i] = (int*) malloc(sizeof(int) * lines) ;
		memset(route[i] , 0, sizeof(int) * lines) ;
	}

		printf("Inializing path : " ) ;
	/* THIS IS TO STORE THE DATA INTO THE MATRIX FROM A FILE*/
	for (i = 0 ; i < lines ; i++) {
		for (j = 0 ; j < lines ; j++) {
			fscanf(fp, "%d", &t) ;
			route[i][j] = t ;
		}
		path[i] = i ;
		printf( " %d ", path[i] ) ;
	}
	printf("\n") ;

	fclose(fp) ; 
//	prefix = (int)calloc(lines-12, sizeof(int*)) ;
	next_prefix(lines, lines-12, 0, path) ;
//	free(prefix) ;
	printf("count : %d \n", tot_weight) ;
	 for(i = 0 ; i < lines ; i++) {    free(route[i]) ;  }
                free(route) ;

	free(path) ;
	exit(0) ;
    //prefix = init_prefix() ; // 그럼 이제 [0 1 2 ~ ] 초기화 됐을 것.

}


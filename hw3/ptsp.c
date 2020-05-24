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
int path[MAXSIZE] ;
int covered[MAXSIZE] ;
int tot_weight = 0 ;
unsigned long long count = 0ULL ; // At 'count', children will count how many routes they covered.
int lines = -1 ; // line count from tsp file. 
long min = -1L ;
short terminate = FALSE ; // Initialize as FALSE 

/***************** Signal Handler *******************/
void
parent_handler(int sig){			/* Handler for master process */
	int i;
	if (sig == SIGINT){
		// 만약에 현재 pid가 root의 pid가 아니면 kill();
		// for each slave pid, kill(INT, pid) ;
		terminate = TRUE ;
		for(i = 0 ; i < lines ; i++) {	free(route[i]) ;  }
		free(route) ; 
		exit(0);

	}
}

void
child_handler(int sig){ /* Handler for children processes */
	int i ;
	if(sig == SIGINT){
		terminate = TRUE ;
		exit(0) ;
	}
}

/***************** Prefix functions *******************/
void
swap(int* x, int* y){
	int temp ;
	temp = *x ;
	*x = *y ;
	*y = temp ;
}
int *
next_prefix(int depth){
	int i ;
	int prefix[lines-12] ;
	if( depth == lines-12 /* k */ ){ // 이게 마지막 배열 부분이면 
		for (i = 0; i < depth ; i++ ){
			prefix[i] = path[i] ; // 0, 1, 2 ~ 0, 1, 3 ~ ~
		}
		return prefix ; // 그리고 나서 바뀐 프리픽스 출력. 
	}
	for ( i = depth ; i < lines /* N */; i++){
		// 재귀로 인해  for문 중첩 효과로 숫자를 변화해감. 
		// 근데 여기서 covered를 어떻게 조건 넣고 break 하지..?
		// 또 자식 프로세스 끝나고 다시 이 함수 next_prefix(0)를 호출했을 때 앞에서 구한 것 다 건너뛸 수 있나..? 
			swap(&path[i], &path[depth]) ;
			
			next_prefix(depth+1) ;
			swap(&path[i], &path[depth]) ;
	}
	return prefix ;
}

int *
init_prefix(){
	/* initialize prefix from the input number of nodes */
	int i ;
	int prefix[lines-12] ;
	for ( i = 0 ; i < lines - 12 ; i++){
		prefix[i] = i ;
		covered[i] = 1 ;
	}
	// [0, 1, 2, 3, 4]
	return prefix; 
}

int * 
end_prefix(){
	/* Getting the last prefix combination */
	int i ;
	int prefix[lines-12] ;
	for ( i = 0 ; i < lines - 12 ; i++){
		prefix[i] = lines - 1 - i ;
		covered[i] = 1 ;
	}
	// [16, 15, 14, 13, 12]
	return prefix; 
}


/***************** children's task allocation  *******************/

void 		 
spawning(int* prefix){ 	// start_a_new_slave with an assigned new prefix 

	int pipes[2] ; // 0: reader / 1: writer
	pid_t child_pid ;
	// create unnamed pipes to communicate parent and children 
	if ( pipe(pipes) != 0){ // error handling of pipe creation
		perror("Create pipes Error! \n" ) ;
		exit(1) ;
	}
	if ((child_pid = fork()) == 0) { /* child */
		//child_proc(prefix, pipes) ;
	}
	else{ /* parent */
		close(pipes[1]) ;
		//	current_child_info(child_pid, pipes[0]) ;
	}
}
void
child_proc(int* prefix, int* pipes){
	int best, n_routes ;
	signal(SIGINT, child_handler) ;
	close(pipes[0]) ; // 읽는 것은 필요없고 여기선 쓰기만 할 것.
	// best, n_routes = travel(prefix) ;
	// best랑 n_routes를 쓰기 위해 write함수 사용.
	// write(pipes[1], , ) ; 
}

void
child_info(pid_t pid, int* pipes){

}
void
result_and_best(pid_t pid) {

}

void
travel(int* prefix){
	path[0] = start ;
	covered[start] = 1 ;
	_travel(1) ;
	covered[start] = 0 ;
}
void
_travel(int idx){
	int i ;
	if (idx == 17 ){
		length += route[path[16]][path[0]] ;
		if (min == -1 || min > length) {
			min = length ;
			// 중복을 제외하고, 첫번째 인덱스를 기준으로 해서 출력이 될것.
		}
		length -= route[path[16]][path[0]] ;
	}
	else {
		for ( i = 0 ; i < 17 ; i++) {
			if (covered[i] == 0) {
				path[idx] = i ;
				covered[i] = 1 ;
				length += route[path[idx-1]][i] ;
				_travel(idx+1) ; // 재귀적으로 들어가는 부분. idx를 늘리면서 간다.
				length -= route[path[idx-1]][i] ;
				covered[i] = 0 ;
			}
		}
	}
}

/***************** Master(main) Process *******************/
int
main(int argc, char** argv){
	// argv[0] = exe command , argv[1] = tsp file instance , argv[2] = number of children 

	pid_t pid ;
	int i = 0, j = 0, t= 0; // variables for loops or something
	int *prefix ; // prefix는 0부터 N-13까지의 인덱스를 가진 배열
	int children =0 ;
	char f_input[1024] ;


	signal(SIGINT, parent_handler);
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

	**route = (int**) malloc(sizeof(int*) * lines) ;
	for(i = 0 ; i< lines ; i++){
		route[i] = (int*) malloc(sizeof(int) * lines) ;
		memset(route[i] , 0, sizeof(int) * lines) ;
	}

	/* THIS IS TO STORE THE DATA INTO THE MATRIX FROM A FILE*/
	for (i = 0 ; i < lines ; i++) {
		for (j = 0 ; j < lines ; j++) {
			fscanf(fp, "%d", &t) ;
			route[i][j] = t ;
		}
		path[i] = i ;
	}
	fclose(fp) ; 
	prefix = (int)calloc(lines-12, sizeof(int*)) ;
    //prefix = init_prefix() ; // 그럼 이제 [0 1 2 ~ ] 초기화 됐을 것.
	
	/* creating Children Processes */
	while( prefix != end_prefix() ){ // prefix가 마지막 시티들 수의 조합이 될 때까지
        if ( children == argv[2] ) {
            pid = wait(0x0) ;
            children-- ;
//            read_result_and_update_best(pid) ; 
        }
        if ( terminate == FALSE ) {
            spawning(prefix) ;
            children++ ;
            prefix = next_prefix(0) ;
        }
        else{ // when terminate is 1(True)
            if ( children == 0 ){
                /* Deallocating the route table then terminate normally */
                for(i = 0 ; i < lines ; i++) {    free(route[i]) ;  }
                free(route) ;
                printf(" The best solution is %ld \n", tot_weight );
                printf(" And the number of covered routes are %llu \n", count );
                printf(" Matrix has been deallocated. Bye! \n");
                exit(0) ;
            }
        }
    }
}


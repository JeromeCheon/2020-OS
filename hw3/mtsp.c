#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <sys/types.h>
#include <pthread.h>

#define TRUE 1
#define FALSE 0

int **route = {0,} ; // I need to allocate width and height dynamically using malloc.
int *path = {0, } ;
int *covered = {0, };
long tot_length = 0L ;
int n_slaves = 0 ;  // global variable for a number of slave threads

int slave_counter = 0 ;
unsigned long long count = 0ULL ; // At 'count', children will count how many routes they covered.
int lines = -1 ; // line count from tsp file.
long min = -1L ;
short terminate = FALSE ; // Initialize as FALSE

typedef struct {
	pthread_t cons_id ;
	long long num_subtask ;
	unsigned long long route_subtask ;
} threads_info ;  // structure to get TID from consumer threads


threads_info ti[8] ; // make 8 structure array to contain values.

typedef struct {
	pthread_cond_t enqueue_cv ;
	pthread_cond_t dequeue_cv ;
	pthread_mutex_t lock ;
	void (**fp)(int) ; // instead of elem, replace as function pointer
	int capacity ;
	int num ;
	int front ;
	int rear ;
} bounded_buffer ;

bounded_buffer * buf = 0x0 ;

/********************* Signal Handler *******************/
void
signal_handler(int sig){
	int i ;
	if (sig == SIGINT) {
		// necessary to add some features
		terminate = TRUE ;
		printf("The best solution : %ld, and covered_routes : %llu \n", min, count ) ;

		for ( i = 0; i < lines ; i++) { free(route[i]) ; }
		free(route) ;
		free(buf) ;
		free(path) ;
		free(covered) ;
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

void
add(int* s, int** arr){
	int i ;
	int newsize = *s + 1 ; // increase the size of array 
	int* temp = (int*)malloc(sizeof(int) * newsize) ;

	// copy the value from arr to temp
	for (i = 0 ; i < sizeof(s) ; i++){
		temp[i] = (*arr)[i] ;
	}
	free(*arr) ;
	*arr = temp ;
	*s += 1 ;
}
void
bounded_buffer_enqueue(bounded_buffer * buf, void (*fp)(int) ){
	pthread_mutex_lock(&(buf->lock)) ;
		while (buf->num == buf->capacity)
			pthread_cond_wait(&(buf->enqueue_cv), &(buf->lock)) ;

		buf->fp[buf->rear] = fp ;
		buf->rear = (buf->rear +1) % buf->capacity ;
		buf->num +=1 ;

		pthread_cond_signal(&(buf->dequeue_cv)) ;
	pthread_mutex_unlock(&(buf->lock)) ;

}
void getPrefix(int start) ;
void
subtasking(int depth){ // such as permutation nPk
	int i ;

	if (terminate == TRUE) // get signalling while processing
		return ;

	if( depth == lines-11 /* k */ ){ // if it's the last element of prefix arry
		covered[lines-12] = 1 ;
		if (slave_counter == n_slaves ){
			// it needs some waiting...
			//pthread_cond_wait(&(buf->queue_cv), &(buf->lock)) ;
			slave_counter -= 1 ;
			// then store the informations and result into somewhere from finished thread
		}
		if ( ( slave_counter < n_slaves) && (!terminate) ){
			slave_counter += 1 ;
			// make threads and allocate some tasks
			// then allocate the subtasks
			 bounded_buffer_enqueue( buf, getPrefix ) ;
			 exit(1) ;
		}
		count++ ;
		return ;
	}
	for ( i = depth ; i < lines /* N */; i++){
		swap(&path[i], &path[depth]) ;
		covered[path[i]] = 1 ;   	 
		subtasking( depth+1 ) ;
		covered[path[i]] = 0 ;
		swap(&path[i], &path[depth]) ;
	}
}

void
explore(int* prefix) {
	int i ;
	int prefixSize = (sizeof(prefix)) / (sizeof(prefix[0])) ;
	// when prefixsize is same with lines
	if ( prefixSize == lines ){
		tot_length += route[path[lines-1]][path[0]] ;
		if (min == -1 || min > tot_length)
			min = tot_length ;
		tot_length -= route[path[lines-1]][path[0]] ;
	}
	else{ 			
		for (i = 0 ; i < lines ; i++) {
			if (covered[i] == 0) {
				add(&prefixSize, &prefix) ; // function to expand a size of the original array
				prefix[prefixSize] = path[prefixSize] = i ;
				covered[i] = 1 ;
				tot_length += route[path[prefixSize-1]][i] ;
				explore(prefix) ;
				tot_length -= route[path[prefixSize-1]][i] ;
				covered[i] = 0 ;
			}
		}
	}
}

void 
getPrefix(int start) { // lines-11 should be typed. 
	int i ;
	int prefix[start] ;
	for ( i = 0 ; i < start ; i++) { // to initialize prefix before exploring
		prefix[i] = path[i] ;
		covered[path[i]] = 1 ;
		tot_length += route[path[i]][i] ;
	}
	explore(prefix) ;
}
/***************** Bounded buffer part *******************/
void
bounded_buffer_init(bounded_buffer * buf, int capacity){
	int i ;
	pthread_cond_init(&(buf->enqueue_cv), 0x0) ;
	pthread_cond_init(&(buf->dequeue_cv), 0x0) ;
	pthread_mutex_init(&(buf->lock), 0x0) ; // it's necessary to initialize
	buf->capacity = capacity ; 
	buf->fp = NULL ;
	buf->num = 0;
	buf->front = 0 ;
	buf->rear = 0;
}

void *
bounded_buffer_dequeue(bounded_buffer * buf){
	void (*r)(int) = 0x0 ;
	pthread_mutex_lock(&(buf->lock)) ;
		while (buf->num == 0)
			pthread_cond_wait(&(buf->dequeue_cv), &(buf->lock)) ;

		r = buf->fp[buf->front] ;
		buf->front = (buf->front +1) % buf->capacity ;
		buf->num -= 1 ;
		pthread_cond_signal(&(buf->enqueue_cv)) ;
	pthread_mutex_unlock(&(buf->lock)) ;
	return (*r) ;
}


/********************* Producer  ********************/
void *
producer(void * ptr){
	//void (*fp)(int) = NULL ; // function pointer and transfer through bounded_buffer

	pthread_t tid ;
	int i ;

	tid = pthread_self() ;
	printf("producer : %ld is running \n", (unsigned long) tid) ;
	subtasking(0) ;

	//bounded_buffer_enqueue(buf, subtasking) ;
	//free(fp) ;
	return 0x0 ;
}

/********************* children threads a.k.a consumer *******************/
void *
consumer(void * ptr){
	pthread_t tid ;
	void (*fp)(int) = NULL ;
	int i ;
	
	for (i = 0; i < n_slaves ; i++){
		if (ti[i].cons_id == 0){
			ti[i].cons_id = tid = pthread_self() ;	
		}

	}

	tid = pthread_self() ; // to get a thread id
	printf("cons : %ld is running \n", (unsigned long) tid) ;
	for(i = 0; i < n_slaves ; i++) { // for each slaves
		printf("to do dequeue \n") ;
		fp = bounded_buffer_dequeue(buf) ;
		if(fp != 0x0 ){
			printf("[%ld] go for prefix\n", (unsigned long) tid) ;
			//getPrefix(lines-11) ;	 getprefix -then-> explore
			fp(lines-11) ;
		}
		free(fp) ;
	}
	printf("done in cons [%ld] \n", (unsigned long) tid) ;
	return 0x0;
}


/********************* Main Thread *******************/
void stat() ;
void threads() ;
void num_N(int new) ;

int
main(int argc, char** argv){
	// argv[0] = exe command , argv[1] = tsp file instance , argv[2] = number of children threads
	if(argc != 3){
		printf("Wrong Input! Usage : %s <tsp_instance> <number of children> \n", argv[0]);
		exit(0);
	}
	pthread_t main_id ;
	pthread_t prod ;
	pthread_t cons[8] ;
	main_id = pthread_self() ; // get main's thread id
	int cmd = 0, newNum ; // variables for user interface command.
	int i = 0, j = 0, t= 0; // variables for loops or something
	char f_input[256] ;
	signal(SIGINT, signal_handler) ;
	sscanf(argv[2], "%d", &n_slaves) ; // string to int
	//ti = (threads_info) calloc( n_slaves, sizeof(threads_info) ) ;
	buf = malloc(sizeof(bounded_buffer)) ;
	
	bounded_buffer_init(buf, n_slaves) ; 
	pthread_create(&(prod), 0x0, producer, 0x0) ; // construct one producer thread
	for (i = 0 ; i < n_slaves ; i++)
		pthread_create(&(cons[i]), 0x0, consumer, 0x0) ;  // construct initial consumer threads

	/* To get a length of file instance */
	FILE * fp = fopen(argv[1], "r");
	while(!feof(fp)){
		fgets(f_input, sizeof(f_input), fp) ;
		lines++ ;
	}
	f_input[0] = 0x0 ; // clear the input buffer
	path = (int*) calloc(lines, sizeof(int)) ;
	covered = (int*) malloc(sizeof(int) *lines) ;
	route = (int**) malloc(sizeof(int*) * lines) ;
	for(i = 0 ; i< lines ; i++){
		route[i] = (int*) malloc(sizeof(int) * lines) ;
		memset(route[i] , 0, sizeof(int) * lines) ;
	}

	/* THIS IS TO STORE THE DATA INTO THE MATRIX FROM A FILE */
	for (i = 0 ; i < lines ; i++) {
		for (j = 0 ; j < lines ; j++) {
			fscanf(fp, "%d", &t) ;
			route[i][j] = t ;
		}
		path[i] = i ;
	}
	fclose(fp) ;

	printf("Hi. This is interactive User interface. \n") ;
	printf("You can also control using 3 instructions : 1.'stat', 2.'threads', and 3.'number N'\n") ;
	while(!terminate){ // main thread : User Interface
		printf("please input the command numbers : 1. stat / 2. threads / 3. number_N \n") ;
		scanf("%d",&cmd) ;
		switch(cmd){
			case 1: // stat function
				 stat() ;
				break ;
			case 2: // threads function
				 //threads() ;
				break ;
			case 3: // number_N fucntion 
				printf("Please type the number you want to change: ") ;
				scanf("%d", &newNum ) ;
				if ( newNum != n_slaves )
					//num_N(newNum) ;
				break ;
			default:
				printf("Wrong instruction number! Please re enter the valid number. \n") ;

		}

	}

	//pthread_join(prod, 0x0) ;
	//pthread_join(cons, 0x0) ;

	for(i = 0 ; i < lines ; i++) {    free(route[i]) ;  }
	free(route) ;
	free(buf) ;
	free(covered) ;
	free(path) ;
	exit(0) ;

}

/****************** other requirements : stat, Threads, and num_N  ********************/

void // to print out the best results up to the moment
stat() {
	printf("You raised the stat function via user interface command line. \n") ;
	// bring values from the global variables
	printf("So far, the best solution is %ld \n", min ) ;
	printf("and the number of checked routes is %llu .\n", count ) ;
}

void
threads(){
	int i ;
	printf("You raised the stat function via user interface command line. \n") ;
	for ( i = 0; i < n_slaves ; i++) {
		printf("cons: [%ld] has done %lld subtasks and %llu routes checked.\n", (unsigned long) ti[i].cons_id, ti[i].num_subtask, ti[i].route_subtask ) ;
	}
}

void
num_N(int new) {
	int i ;
	if ( new > n_slaves ){	
		for (i = n_slaves ; i < new ; i++)
			pthread_create(&(ti[i].cons_id), 0x0, consumer, 0x0) ;  // construct initial consumer threads
	}
	else{ // new < n_slaves
		for ( i = n_slaves-1 ; i > new ; i--)
		{	pthread_cancel(ti[i].cons_id) ; }
	}
	n_slaves = new ;

}


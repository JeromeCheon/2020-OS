#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include "smalloc.h" 

int totalpages = 0 ;


sm_container_t sm_head = {
	0,
	&sm_head, 
	&sm_head,
	0 
} ;

static 
void * 
_data (sm_container_ptr e)
{
	return ((void *) e) + sizeof(sm_container_t) ;
}

static 
void 
sm_container_split (sm_container_ptr hole, size_t size)
{
	sm_container_ptr remainder = (sm_container_ptr) (_data(hole) + size) ;

	remainder->dsize = hole->dsize - size - sizeof(sm_container_t) ;	
	remainder->status = Unused ;
	remainder->next = hole->next ;
	remainder->prev = hole ;
	hole->dsize = size ;
	hole->next->prev = remainder ;
	hole->next = remainder ;
}

static 
void * 
retain_more_memory (int size)
{
	sm_container_ptr hole ;
	int pagesize = getpagesize() ;
	int n_pages = 0 ;

	n_pages = (sizeof(sm_container_t) + size + sizeof(sm_container_t)) / pagesize  + 1 ;
	hole = (sm_container_ptr) sbrk(n_pages * pagesize) ;
	totalpages += (n_pages * pagesize) ;

	if (hole == 0x0)
		return 0x0 ;
	
	hole->status = Unused ;
	hole->dsize = n_pages * getpagesize() - sizeof(sm_container_t) ;
	return hole ;
}

void * 
smalloc (size_t size) 
{
	sm_container_ptr hole = 0x0, itr = 0x0 ;
	size_t least = 1000000 ; // this is for comparison to find the profit hole
	sm_container_ptr leasthole = 0x0 ;
	// here, the way I see it, this iteration traversal all of linked list nodes. 
	// so just let it be only traversal without breaking the loop.
	for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next) {
		if (itr->status == Busy)
			continue ;
		if ( ((size == itr->dsize) || (size + sizeof(sm_container_t) < itr->dsize)) && (least>=itr->dsize) ) {
			least = itr->dsize ;
			leasthole = itr ;
			//hole = itr ; later give this to found one
			//break ; 
		}
	}
	hole = leasthole ;
	
	if (hole == 0x0) {
		hole = retain_more_memory(size) ;
		if (hole == 0x0)
			return 0x0 ;
		hole->next = &sm_head ;
		hole->prev = sm_head.prev ;
		(sm_head.prev)->next = hole ;
		sm_head.prev = hole ;
	}
	if (size < hole->dsize) 
		sm_container_split(hole, size) ;
	hole->status = Busy ;
	return _data(hole) ;
}

void 
sfree (void * p)
{
	sm_container_ptr itr ;
	for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next) {
		if (p == _data(itr)) {
			itr->status = Unused ;
			//break ;
			//instead of breaking right away, find the adjacent 'unused' container
			if( (itr->prev != 0x0) && (itr->prev->status) == Unused ){ // left side of the target pointer
				itr->prev->dsize += itr->dsize ;
				itr->prev->next = itr->next ;
				itr->prev = itr->next->prev;
			}

			if( (itr->next != 0x0) && (itr->next->status) == Unused ){ // right side of the target pointer
				itr->next->dsize += itr->dsize ;
				itr->next->prev = itr->prev ;
				itr->next = itr->prev->next ;
			
			}
			//free(p) ;
			break ;
		}
	}
}

void 
print_sm_containers ()
{
	sm_container_ptr itr ;
	int i ;

	printf("==================== sm_containers ====================\n") ;
	for (itr = sm_head.next, i = 0 ; itr != &sm_head ; itr = itr->next, i++) {
		printf("%3d:%p:%s:", i, _data(itr), itr->status == Unused ? "Unused" : "  Busy") ;
		printf("%8d:", (int) itr->dsize) ;

		int j ;
		char * s = (char *) _data(itr) ;
		for (j = 0 ; j < (itr->dsize >= 8 ? 8 : itr->dsize) ; j++) 
			printf("%02x ", s[j]) ;
		printf("\n") ;
	}
	printf("\n") ;

}

void
print_mem_uses (){

	sm_container_ptr itr ;	
	size_t allocatedAll = 0 ;
	for (itr = sm_head.next ; itr != &sm_head ; itr = itr->next) {
		if (itr->status == Busy)
			allocatedAll += itr->dsize ;
	}
	fprintf(stderr, "=========== Amount of memory retained by smalloc =============\n") ;
	fprintf(stderr, "memory retained by smalloc so far: FROM  ") ;
	fprintf(stderr, "%p -------- %p \n", _data(sm_head.next), _data(itr->prev) ) ;
	fprintf(stderr, "Total amount of pages is %d \n\n", totalpages ) ;

	fprintf(stderr, "=========== Amount of memory allocated by smalloc =============\n") ;
	fprintf(stderr, "memory allocated by smalloc at this moment: %d \n\n", (int)allocatedAll ) ;

	fprintf(stderr, "=========== Amount of free(unused) memory by smalloc =============\n") ;
	fprintf(stderr, "memory retained by smalloc, but not allocated : %d \n\n", totalpages - (int)allocatedAll ) ;

}

void *
srealloc(void* p, size_t nsize) {
	// couldn't implement in time due to lack of the time individually.. 
}

void 
sshrink(){
	// couldn't implement in time due to lack of the time individually.. 
}

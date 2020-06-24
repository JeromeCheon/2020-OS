#include <stdio.h>
#include "smalloc.h"

int 
main()
{
	void *p1, *p2, *p3, *p4, *p5, *p6 ;

	print_sm_containers() ;

	p1 = smalloc(700) ; 
	printf("smalloc(700):%p\n", p1) ; 
	print_sm_containers() ;

	p2 = smalloc(1500) ; 
	printf("smalloc(1500):%p\n", p2) ; 
	print_sm_containers() ;

	sfree(p1) ; 
	printf("sfree(%p)\n", p1) ; 
	print_sm_containers() ;

	p3 = smalloc(1200) ; 
	printf("smalloc(1200):%p\n", p3) ; 
	print_sm_containers() ;

	p4 = smalloc(200) ; 
	printf("smalloc(200):%p\n", p4) ; 
	print_sm_containers() ;

	sfree(p2) ; 
	printf("sfree(%p)\n", p2) ; 
	print_sm_containers() ;
	
	p5 = smalloc(900) ; 
	printf("smalloc(900):%p\n", p5) ; 
	print_sm_containers() ;

	p6 = smalloc(100) ; 
	printf("smalloc(100):%p\n", p6) ; 
	print_sm_containers() ;
	// task 3 
	print_mem_uses() ;
}

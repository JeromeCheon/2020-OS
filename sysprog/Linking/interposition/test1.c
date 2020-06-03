#include <stdio.h>
#include <stdlib.h>
#include <time.h>

int 
main ()
{
	int i ;

	srand(time(0x0)) ;
	// random seed a time value. will have a different pattern. 
	for (i = 0 ; i < 10 ; i++) {
		printf("%d\n", rand()) ;
	}
	// 그렇게 해서 ./test1 을 실행하면 무작위의 랜덤숫자가 10번 출력돼 
	// 근데 만약 이런 간단한 프로그램이 아닌, 좀 더 복잡한 프로그램을 이 random number generater 를 통해 구축하고 싶다면, your program has an error. 디버깅 목적으로, 랜덤 넘버가 생성되면 그것은 less reproducibility 를 가질 거야. 
	// so you don't wanna touch anything for test number one but you want to control the rand functions behavior. 그 때 runtime interpositioning library를 통해 할 수 있어. 

	// 이제 myrand.c 를 살펴보자.
}

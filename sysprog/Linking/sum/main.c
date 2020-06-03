#include <stdio.h>


// 근데 여기서 sum declaration을 주석 처리를 하면 '원칙상'  gcc는 컴파일을 허용하지 않아. 
// 그래서 이 선언은 essential information for translating into instruction level or assembly level
// before a function call, it pushes things on the stack. Type information. 이게 주어져야. 
// without this information, compiler can't generate assembly code for this func call. 
// we can have a function name as a symbol. No having definition is alright. 
// it will be resolved by the linking phase. 
int sum (int * a, int n) ;
// 여기 sum은 생성만 되어 있고 정의는 안돼있어. 


int array[2] = {1, 2} ;
// 이렇게 메인 함수가 있고 sum이라는 함수를 val 변수와 도킹하고 있지. 
int main ()
{
	int val = sum(array, 2) ;
	// 1st param : pointer, 2nd : num of indices.
	printf("%d\n", val) ;

	return 0 ;
}

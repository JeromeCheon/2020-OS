#include <stdio.h>

int main(){
	int d ;
	while (1){
		d = getchar();

		int i = 0;
		for(i =0; i<1000000 ; i++){
			d +=i;
		}
		printf("%d \n", d);
	}
}

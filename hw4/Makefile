all :
	gcc -pthread -o target1 tar1_abba.c
	gcc -pthread -o target2 tar2_dinning.c
	gcc -shared fPIC -o ddmon.so ddmon.c -ldl
	gcc -o ddcheck ddcheck.c


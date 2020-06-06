all :
	gcc -pthread -o target1 tar1_abba.c -O0 -g
	gcc -pthread -o target2 tar2_dinning.c -O0 -g
	gcc -shared -fPIC -o ddmon.so ddmon.c -ldl
	gcc -o ddchck ddchck.c

clean:
	rm target1
	rm target2
	rm ddmon.so
	rm ddchck

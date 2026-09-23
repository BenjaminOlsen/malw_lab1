
lib1.so: lib1.c
	gcc -fPIC -shared -o lib1.so lib1.c -ldl

clean:
	rm -rf lib1.so

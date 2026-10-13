
lib1.so: lib1.c
	gcc -fPIC -shared -o lib1.so lib1.c -ldl

lib2.so: lib2.c
	gcc -fPIC -shared -o lib2.so lib2.c -ldl

.PHONY: all clean

clean:
	rm -f lib1.so lib2.so

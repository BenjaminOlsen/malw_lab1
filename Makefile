all: lib1.so lib2.so lib3.so

lib1.so: lib1.c
	gcc -fPIC -shared -o lib1.so lib1.c -ldl

lib2.so: lib2.c
	gcc -fPIC -shared -o lib2.so lib2.c -ldl

lib3.so: lib3.c
	gcc -fPIC -shared -o lib3.so lib3.c -ldl

.PHONY: all clean

clean:
	rm -f lib1.so lib2.so lib3.so

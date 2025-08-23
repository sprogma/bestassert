gcc -O0 -g -c a.c -o a.o
gcc -O0 -g -c myassert2.c -o myassert2.o
gcc -O0 -g a.o myassert2.o -o a2.exe

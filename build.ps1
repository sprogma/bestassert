$F = @("-Wall", "-Wextra")
gcc $F -O0 -g -c a.c -o a.o
gcc $F -O0 -g -c myassert.c -o myassert.o
gcc $F -O0 -g a.o myassert.o -o ($IsWindows ?"a.exe":"a")


if ($IsWindows)
{
    gcc -g example/a.c "bestassert.dll" -o test1.exe "-Iinclude"
    gcc -g example/b.c "bestassert.dll" -o test2.exe "-Iinclude"
}
else
{
    gcc -g example/a.c -o test1 "-Iinclude" "-L." "-lbestassert"
    gcc -g example/b.c -o test2 "-Iinclude" "-L." "-lbestassert"
}

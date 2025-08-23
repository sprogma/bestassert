
if ($IsWindows)
{
    gcc -g example/a.c "bestassert.dll" -o test.exe "-Iinclude"
}
else
{
    gcc -g example/a.c -o test "-Iinclude" "-L." "-lbestassert"
}

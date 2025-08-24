
if ($IsWindows)
{
    Write-Host -- gcc -g example/a.c "bestassert.dll" -o test1.exe "-Iinclude"
    gcc -g example/a.c "bestassert.dll" -o test1.exe "-Iinclude"
    Write-Host -- gcc -g example/b.c "bestassert.dll" -o test2.exe "-Iinclude"
    gcc -g example/b.c "bestassert.dll" -o test2.exe "-Iinclude"

}
else
{
    Write-Host -- gcc -g example/a.c -o test1 "-Iinclude" "-L." "-lbestassert"
    gcc -g example/a.c -o test1 "-Iinclude" "-L." "-lbestassert"
    Write-Host -- gcc -g example/b.c -o test2 "-Iinclude" "-L." "-lbestassert"
    gcc -g example/b.c -o test2 "-Iinclude" "-L." "-lbestassert"
}

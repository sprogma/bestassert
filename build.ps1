
# build game :)
gcc game/game.c -o game 2>$null

# build assert
$F = @("-Wall", "-Wextra")
$L = @()
$F += @("-Iinclude")

if (!$IsWindows)
{
    $F += @("-fPIC", "-fpic")
    $L += @("-fPIC", "-fpic")
}
else
{
    $F += @("-DLIBRARY_EXPORT")
}

$platform = ($IsWindows ?"win":"lin")

gcc $F -O0 -g -c any/bestassert.c -o any/bestassert.o
gcc $F -O0 -g -c $platform/bestassert.c -o $platform/bestassert.o
gcc $L $platform/bestassert.o any/bestassert.o -shared -o "$($IsWindows ?'':'lib')bestassert.$($IsWindows ?'dll':'so')"

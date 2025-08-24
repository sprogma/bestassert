param(
    [string[]]$CompilerFlags
)

# build game :)
gcc game/game.c -o game 2>$null

# build assert
$build = @()
$F = @("-Wall", "-Wextra") + $CompilerFlags
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
$build += "any/bestassert.o"

gcc $F -O0 -g -c $platform/bestassert.c -o $platform/bestassert.o
$build += "$platform/bestassert.o"

if (Test-Path $platform/signals.c)
{
    gcc $F -O0 -g -c $platform/signals.c -o $platform/signals.o
    $build += "$platform/signals.o"
}

gcc $L $build -shared -o "$($IsWindows ?'':'lib')bestassert.$($IsWindows ?'dll':'so')"

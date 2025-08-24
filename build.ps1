param(
    [string[]]$CompilerFlags
)

# build game :)
gcc game/game.c -o bestassert_game$($IsWindows ?'.exe':'') 2>$null

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



Write-Host -- gcc $F -O0 -g -c any/bestassert.c -o any/bestassert.o
gcc $F -O0 -g -c any/bestassert.c -o any/bestassert.o

$build += "any/bestassert.o"

Write-Host -- gcc $F -O0 -g -c $platform/bestassert.c -o $platform/bestassert.o
gcc $F -O0 -g -c $platform/bestassert.c -o $platform/bestassert.o

$build += "$platform/bestassert.o"

Write-Host -- gcc $F -O0 -g -c $platform/signals.c -o $platform/signals.o
gcc $F -O0 -g -c $platform/signals.c -o $platform/signals.o

$build += "$platform/signals.o"


Write-Host -- gcc $L $build -shared -o "$($IsWindows ?'':'lib')bestassert.$($IsWindows ?'dll':'so')"
gcc $L $build -shared "-Wl,--out-implib,libbestassert.lib" -o "$($IsWindows ?'':'lib')bestassert.$($IsWindows ?'dll':'so')"

# if (!$IsWindows)
# {
#     Write-Host -- ar rcs libmyfuncs.a foo.o bar.o
#     ar rcs libbestassert.a $build
# }

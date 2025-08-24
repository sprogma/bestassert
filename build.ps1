param(
    [string[]]$CompilerFlags
)


function Run-Command
{
    Write-Host ($MyInvocation.UnboundArguments)
    $cmd = @($MyInvocation.UnboundArguments[1..$MyInvocation.UnboundArguments.Count])
    &$MyInvocation.UnboundArguments[0] @cmd
}


# build game :)
Run-Command -- gcc -O0 game/game.c -o bestassert_game$($IsWindows ?'.exe':'') 2>$null

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



Run-Command -- gcc $F -O0 -g -c any/bestassert.c -o any/bestassert.o

$build += "any/bestassert.o"

Run-Command -- gcc $F -O0 -g -c $platform/bestassert.c -o $platform/bestassert.o

$build += "$platform/bestassert.o"

Run-Command -- gcc $F -O0 -g -c $platform/signals.c -o $platform/signals.o

$build += "$platform/signals.o"

Run-Command -- gcc $L $build -shared "-Wl,--out-implib,libbestassert.lib" -o "$($IsWindows ?'':'lib')bestassert.$($IsWindows ?'dll':'so')"

# if (!$IsWindows)
# {
#     Run-Command -- ar rcs libbestassert.a $build
# }

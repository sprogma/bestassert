if ($IsWindows)
{
    ./build.ps1 -CompilerFlags "-DCREATE_NEW_CONSOLE_FOR_GDB", "-DADD_SIGNAL_HANDLERS"
}
else
{
    ./build.ps1 -CompilerFlags "-DCREATE_NEW_CONSOLE_FOR_GDB", "-DADD_SIGNAL_HANDLERS", "-DKILL_GDB"
}

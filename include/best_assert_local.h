#ifndef BEST_ASSERT_LOCAL
#define BEST_ASSERT_LOCAL

#include "stdlib.h"


#ifdef WIN32
    #include "conio.h"
    #include "windows.h"
    
    extern PROCESS_INFORMATION gdb;
    extern HANDLE hChildStdoutRd;
    extern HANDLE hChildStdinWr;
    
#else
    #include "unistd.h"
    #include "spawn.h"

    extern pid_t gdb_pid;
    extern int child_stdin_pipe[2];
    extern int child_stdout_pipe[2];
#endif

void bestassert_send_text(const char *);
int bestassert_gdbchar();
void bestassert_bestspinlock();

#endif

#include "windows.h"
#include "conio.h"
	
#include "errno.h"
#include "stdio.h"
#include "fcntl.h"
#include "signal.h"
#include "ctype.h"
#include "string.h"

#include "best_assert.h"
#include "best_assert_local.h"



PROCESS_INFORMATION gdb = {};
HANDLE hChildStdoutRd = NULL;
HANDLE hChildStdinWr = NULL;


void bestassert_bestspinlock(struct gdb_instance *gdb)
{
    (void)gdb;
    while (!IsDebuggerPresent())
    {
        Sleep(50);
    }
}



void bestassert_send_text(struct gdb_instance *gdb, const char *commands)
{
    DWORD written = 0;
    EXITIF(!WriteFile(gdb->hChildStdinWr, commands, strlen(commands), &written, NULL),
            "WriteFile to stdin failed: %lu", GetLastError());
}


void bestassert_run_gdb(struct gdb_instance *gdb, int user_friendly)
{   
    int pid_to_attach = GetCurrentProcessId();
    
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    HANDLE hChildStdinRd = NULL;
    EXITIF(!CreatePipe(&hChildStdinRd, &gdb->hChildStdinWr, &sa, 0),
           "CreatePipe error.");

    HANDLE hChildStdoutWr = NULL;
    EXITIF(!CreatePipe(&gdb->hChildStdoutRd, &hChildStdoutWr, &sa, 0),
           "CreatePipe error.");

    SetHandleInformation(gdb->hChildStdinWr, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(gdb->hChildStdoutRd, HANDLE_FLAG_INHERIT, 0);

    char app[1024];
    DWORD flags = 0;
    BOOL inerhit_handles = FALSE;
    STARTUPINFO si = {};
    si.cb = sizeof(si);
    if (!user_friendly)
    {
        snprintf(app, sizeof(app), "gdb attach %d -ex cont --silent -i=mi", pid_to_attach);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = hChildStdinRd;
        si.hStdOutput = hChildStdoutWr;
        si.hStdError = hChildStdoutWr;
        inerhit_handles = TRUE;
    }
    else
    {
        snprintf(app, sizeof(app), "gdb attach %d -ex cont", pid_to_attach);
        #ifdef CREATE_NEW_CONSOLE_FOR_GDB
            flags |= CREATE_NEW_CONSOLE;
        #endif
    }
    
    EXITIF(!CreateProcess(NULL, app, NULL, NULL, inerhit_handles,
                        flags, NULL, NULL, &si, &gdb->gdb),
           "CreateProcess failed: %lu", GetLastError());
    
    CloseHandle(hChildStdinRd);
    CloseHandle(hChildStdoutWr);
}



int bestassert_gdbchar(struct gdb_instance *gdb)
{
    DWORD readBytes;
    DWORD availableBytes = 0;
    EXITIF(!PeekNamedPipe(gdb->hChildStdoutRd, NULL, 0, NULL, &availableBytes, NULL),
           "PeerNamedPipe error!?");
    if (availableBytes == 0)
    {
        return -1;
    }
    CHAR buffer[1] = {};
    BOOL ok = ReadFile(gdb->hChildStdoutRd, buffer, 1, &readBytes, NULL);
    EXITIF(!ok || readBytes == 0,
           "Error: read EOF or channel is broken.");
    return buffer[0];
}



void bestassert_wait_to_close(struct gdb_instance *gdb)
{
    #ifdef KILL_GDB
        printf("Bad windows kill gdb :)\n");
        TerminateProcess(gdb->gdb.hProcess, 0);
        printf("gdb closed. Best Assert section ends.\n\n");
    #endif
    gdb->gdb = (PROCESS_INFORMATION){};
    gdb->hChildStdoutRd = NULL;
    gdb->hChildStdinWr = NULL;
}

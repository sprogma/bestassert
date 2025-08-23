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
    



 void bestassert_bestspinlock()
{
    while (!IsDebuggerPresent())
    {
        Sleep(50);
    }
}


void bestassert_send_text(const char *commands)
{
    DWORD written = 0;
    if (!WriteFile(hChildStdinWr, commands, strlen(commands), &written, NULL)) 
    {
        printf("WriteFile to stdin failed: %lu\n", GetLastError());
    }
}


void bestassert_run_gdb(int user_friendly)
{   
    int pid_to_attach = GetCurrentProcessId();
    
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    HANDLE hChildStdinRd = NULL;
    if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &sa, 0))
    {
        printf("CreatePipe error. exiting...\n");
        exit(4);
    }

    HANDLE hChildStdoutWr = NULL;
    if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &sa, 0)) 
    {
        printf("CreatePipe error. exiting...\n");
        exit(4);
    }

    SetHandleInformation(hChildStdinWr, HANDLE_FLAG_INHERIT, 0);
    SetHandleInformation(hChildStdoutRd, HANDLE_FLAG_INHERIT, 0);

    char app[1024];
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
    }
    
    if (!CreateProcess(NULL, app, NULL, NULL, inerhit_handles,
                        0, NULL, NULL, &si, &gdb))
    {
        printf("CreateProcess failed: %lu\n", GetLastError());
        exit(4);
    }
    
    CloseHandle(hChildStdinRd);
    CloseHandle(hChildStdoutWr);
}


int bestassert_gdbchar()
{
    DWORD readBytes;
    DWORD availableBytes = 0;
    if (!PeekNamedPipe(hChildStdoutRd, NULL, 0, NULL, &availableBytes, NULL))
    {
        printf("PeekNamedPipe error!?\n");
        exit(4);
    }
    if (availableBytes == 0)
    {
        return -1;
    }
    CHAR buffer[1] = {};
    BOOL ok = ReadFile(hChildStdoutRd, buffer, 1, &readBytes, NULL);
    if (!ok || readBytes == 0)
    {
        printf("Error: read EOF or channel is broken.\n");
        exit(4);
    }
    return buffer[0];
}


void bestassert_wait_to_close()
{
    printf("Bad windows kill gdb :)\n");
    TerminateProcess(gdb.hProcess, 0);
    printf("gdb closed. Best Assert section ends.\n\n");
    gdb = (PROCESS_INFORMATION){};
    hChildStdoutRd = NULL;
    hChildStdinWr = NULL;
}

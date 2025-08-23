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



void bestassert_run_gdb(int pid_to_attach)
{   
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

    STARTUPINFO si = {};
    si.cb = sizeof(si);
    if (!pid_to_attach)
    {
        char app[] = "gdb --silent -i=mi";
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = hChildStdinRd;
        si.hStdOutput = hChildStdoutWr;
        si.hStdError = hChildStdoutWr;
        if (!CreateProcess(NULL, app, NULL, NULL, TRUE,
                            0, NULL, NULL, &si, &gdb))
        {
            printf("CreateProcess failed: %lu\n", GetLastError());
            exit(4);
        }
    }
    else
    {
        char app[1024];
        sprintf(app, "gdb attach %d -ex cont", pid_to_attach);
        if (!CreateProcess(NULL, app, NULL, NULL, FALSE,
                            CREATE_NEW_CONSOLE, NULL, NULL, &si, &gdb))
        {
            printf("CreateProcess failed: %lu\n", GetLastError());
            exit(4);
        }
    }
    
    CloseHandle(hChildStdinRd);
    CloseHandle(hChildStdoutWr);
}

void bestassert_attach()
{
    char input[1024] = {};
    sprintf(input, "attach %ld\n"
                   "continue\n", GetCurrentProcessId());
    bestassert_send_text(input);
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
        return -1;
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



#include "stdio.h"
#define UNICODE
#include "windows.h"
#include "fcntl.h"
#include "io.h"
#include "signal.h"


void bestassert_run_gdb(const char *expr, int line, const char *file)
{
    fprintf(stderr, "\n\n-----------------------------------------"
                      "\n    Oh no! best assertation occured!\n");
    fprintf(stderr, "AT: %s:%d: expression (%s) == false\n", file, line, expr);
    
    PROCESS_INFORMATION gdb = {};
    HANDLE hChildStdinWr = NULL;
    HANDLE hChildStdoutRd = NULL;
    HANDLE myStdout = GetStdHandle(STD_OUTPUT_HANDLE);
    HANDLE myStderr = GetStdHandle(STD_ERROR_HANDLE);
    
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

    HANDLE hChildStdinRd = NULL;
    if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &sa, 0))
    {
        printf("CreatePipe error. exiting...\n");
        exit(4);
    }

    SetHandleInformation(hChildStdinWr, HANDLE_FLAG_INHERIT, 0);

    HANDLE hChildStdoutWr = NULL;
    if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &sa, 0)) 
    {
        printf("CreatePipe error. exiting...\n");
        exit(4);
    }

    SetHandleInformation(hChildStdoutRd, HANDLE_FLAG_INHERIT, 0);

    STARTUPINFO si = {};
    si.cb = sizeof(si);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdInput = hChildStdinRd;
    si.hStdOutput = myStdout;// hChildStdoutWr;
    si.hStdError = myStderr;// hChildStdoutWr;

    wchar_t app[] = L"gdb";
    if (!CreateProcess(NULL, app, NULL, NULL, TRUE,
                        0, NULL, NULL, &si, &gdb))
    {
        printf("CreateProcess failed: %lu\n", GetLastError());
        CloseHandle(hChildStdinRd); 
        CloseHandle(hChildStdinWr);
        // CloseHandle(hChildStdoutRd); 
        // CloseHandle(hChildStdoutWr);
        exit(4);
    }
    
    CloseHandle(hChildStdinRd);
    CloseHandle(hChildStdoutWr);

        
    char input[1024] = {};
    sprintf(input, "attach %ld\n"
                   // "info threads\n"
                   // "info sources\n"
                   // "info locals\n"
                   "continue\n"
                   "shell echo --- BEST ASSERT INFORMATION FLOW ---\n"
                   "shell echo ------------------------------------\n"
                   "shell echo ------------ STACK TRACE -----------\n"
                   "bt 99\n"
                   "shell echo ------------------------------------\n"
                   "shell echo -------------- LOCALS --------------\n"
                   "info locals\n"
                   "shell echo ---  EXITING FROM BEST ASSERT :( ---\n"
                   "q\n", GetCurrentProcessId());
    
    DWORD written = 0;
    if (!WriteFile(hChildStdinWr, input, (DWORD)strlen(input), &written, NULL)) {
        printf("WriteFile to stdin failed: %lu\n", GetLastError());
    }

    /* wait for connection... */
    printf("\n\n\nwaiting... [data collecting. Buy premium to bypass speed limit :)]\n");
    Sleep(3000);
}

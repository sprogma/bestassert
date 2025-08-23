#include "stdio.h"
#define UNICODE
#include "windows.h"
#include "fcntl.h"
#include "io.h"
#include "signal.h"



int myassert(int expr)
{
    if (!expr) // !gdb.hProcess && hChildStdinWr == NULL)
    {
        PROCESS_INFORMATION gdb = {};
        HANDLE hChildStdinWr = NULL;
        HANDLE hChildStdoutRd = NULL;
        HANDLE myStdout = GetStdHandle(STD_OUTPUT_HANDLE);
        HANDLE myStderr = GetStdHandle(STD_ERROR_HANDLE);
        printf("%p %p\n", myStdout, myStderr);
        
        SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };

        HANDLE hChildStdinRd = NULL;
        if (!CreatePipe(&hChildStdinRd, &hChildStdinWr, &sa, 0)) 
            return 1;

        SetHandleInformation(hChildStdinWr, HANDLE_FLAG_INHERIT, 0);

        HANDLE hChildStdoutWr = NULL;
        if (!CreatePipe(&hChildStdoutRd, &hChildStdoutWr, &sa, 0)) 
            return 1;

        SetHandleInformation(hChildStdoutRd, HANDLE_FLAG_INHERIT, 0);

        STARTUPINFO si = {};
        si.cb = sizeof(si);
        si.dwFlags = STARTF_USESTDHANDLES;
        si.hStdInput = hChildStdinRd;
        si.hStdOutput = myStderr;// hChildStdoutWr;
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
                       "info threads\n"
                       "info sources\n"
                       "info locals\n"
                       "continue\n"
                       "bt 10\n"
                       "info locals\n", GetCurrentProcessId());
        
        DWORD written = 0;
        if (!WriteFile(hChildStdinWr, input, (DWORD)strlen(input), &written, NULL)) {
            printf("WriteFile to stdin failed: %lu\n", GetLastError());
        }

        /* wait for connection... */
        Sleep(1000);

        printf("waiting...\n");
        __asm__ __volatile__("int $3");
        return 0;
    }
}

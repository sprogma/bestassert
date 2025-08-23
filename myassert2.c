#include "stdio.h"
#define UNICODE
#include "windows.h"
#include "fcntl.h"
#include "io.h"

PROCESS_INFORMATION gdb = {};
HANDLE hChildStdinWr = NULL;
HANDLE hChildStdoutRd = NULL;

void sync_gdb()
{
    DWORD readBytes;
    for (;;) {
        DWORD avail = 0;
        if (!PeekNamedPipe(hChildStdoutRd, NULL, 0, NULL, &avail, NULL))
        {
            printf("Error!?\n");
        }
        if (avail > 0)
        {
            CHAR buffer[4096] = {};
            if (avail > sizeof(buffer) - 1)
            {
                avail = sizeof(buffer) - 1;
            }
            BOOL ok = ReadFile(hChildStdoutRd, buffer, avail, &readBytes, NULL);
            if (!ok || readBytes == 0) break;
            buffer[readBytes] = '\0';
            printf("?%s", buffer);
        }
        else
        {
            printf(".");
            return;
        }
    }
}

int myassert(int expr)
{
    if (!expr) // !gdb.hProcess && hChildStdinWr == NULL)
    {
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
        si.hStdOutput = hChildStdoutWr;
        si.hStdError = hChildStdoutWr;

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
        sprintf(input, "attach %ld\ninfo sources\nbreak myassert.c:98\nc\n", gdb.dwProcessId);
        DWORD written = 0;
        if (!WriteFile(hChildStdinWr, input, (DWORD)strlen(input), &written, NULL)) {
            printf("WriteFile to stdin failed: %lu\n", GetLastError());
        }

        Sleep(1000);

        while (1)
        {
            sync_gdb();
            Sleep(128);
        }

    }
//     if (!expr)
//     {
//         printf("assert!");
//         
//         char input[1024] = {};
//         sprintf(input, "info locals\n");
//         DWORD written = 0;
//         if (!WriteFile(hChildStdinWr, input, (DWORD)strlen(input), &written, NULL)) {
//             wprintf(L"WriteFile to stdin failed: %u\n", GetLastError());
//         }
// 
//         Sleep(1000);
// 
//         sync_gdb();
// 
//         while (1)
//         {
//             sync_gdb();
//             Sleep(128);
//         }
//     }
    (void)expr;
}

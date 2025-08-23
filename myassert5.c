#include "stdio.h"
#include "windows.h"
#include "fcntl.h"
#include "conio.h"
#include "io.h"
#include "signal.h"

#include "my_assert.h"


#define PREMIUM




PROCESS_INFORMATION gdb = {};
HANDLE hChildStdoutRd = NULL;
HANDLE hChildStdinWr = NULL;



static void bestassert_bestspinlock()
{
    while (!IsDebuggerPresent())
    {
        Sleep(50);
    }
}



static void bestassert_send_text(const char *commands)
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
    if (!pid_to_attach)
    {
        char app[] = "gdb --silent -i=mi";
        si.hStdInput = hChildStdinRd;
        si.hStdOutput = hChildStdoutWr;
        si.hStdError = hChildStdoutWr;
        if (!CreateProcess(NULL, app, NULL, NULL, TRUE,
                            0, NULL, NULL, &si, &gdb))
        {
            printf("CreateProcess failed: %lu\n", GetLastError());
            CloseHandle(hChildStdinRd); 
            CloseHandle(hChildStdinWr);
            exit(4);
        }
    }
    else
    {
        char app[1024];
        sprintf(app, "gdb attach %d -ex cont", pid_to_attach);
        si.dwFlags = 0;
        if (!CreateProcess(NULL, app, NULL, NULL, FALSE,
                            CREATE_NEW_CONSOLE, NULL, NULL, &si, &gdb))
        {
            printf("CreateProcess failed: %lu\n", GetLastError());
            CloseHandle(hChildStdinRd); 
            CloseHandle(hChildStdinWr);
            exit(4);
        }
    }

    
    
    CloseHandle(hChildStdinRd);
}

void bestassert_attach()
{
    char input[1024] = {};
    sprintf(input, "attach %ld\n"
                   "continue\n", GetCurrentProcessId());
    bestassert_send_text(input);
}
    
void bestassert_request()
{
    {
        char input[1024] = {};
        sprintf(input, "bt 99\n"
                       "info locals\n"
                       "continue\n");
        bestassert_send_text(input);
    }

    /* wait for connection... */
    #ifndef PREMIUM
        printf("\n\n\nwaiting... [data collecting. Buy premium to bypass speed limit :)]\n");
        Sleep(3000);
    #else
        bestassert_bestspinlock();
    #endif
}



static int bestassert_gdbchar()
{
    DWORD readBytes;
    DWORD avail = 0;
    if (!PeekNamedPipe(hChildStdoutRd, NULL, 0, NULL, &avail, NULL))
    {
        printf("Error!?\n");
        exit(4);
    }
    if (avail > 0)
    {
        CHAR buffer[1] = {};
        BOOL ok = ReadFile(hChildStdoutRd, buffer, 1, &readBytes, NULL);
        if (!ok || readBytes == 0)
        {
            return -1;
        }
        return buffer[0];
    }
    else
    {
        return -1;
    }
}


int bestassert_update()
{
    /* read while there is no &bt */
    const char *text = "&\"bt";
    int next_id = 0;
    while (text[next_id] != 0)
    {
        int ch = bestassert_gdbchar();
        if (ch != -1)
        {
            if (ch == text[next_id])
            {
                next_id++;
            }
            else
            {
                next_id = 0;
            }
        }
    }

    const char * sections[] = {
        "stack trace",
        "local variables",
    };

    /* start parsing */
    for (int sd = 0; sd < (int)(sizeof(sections)/sizeof(*sections)); ++sd)
    {
        while (bestassert_gdbchar() != '\n');
        printf("------------------------------------------------------------\n");
        printf("                         %s\n", sections[sd]);
        /* parse stack trace */
        while (1)
        {
            int type = bestassert_gdbchar();
            if (type == '&')
            {
                break;
            }
            if (type == '~')
            {
                char line[1024] = "py -c print('";
                int ch = 0, id = strlen(line);
                while (ch != '\n' && id + 2 < (int)sizeof(line))
                {
                    ch = line[id++] = bestassert_gdbchar();
                }
                id -= 2;
                line[id++] = '\'';
                line[id++] = ',';
                line[id++] = 'e';
                line[id++] = 'n';
                line[id++] = 'd';
                line[id++] = '=';
                line[id++] = '\'';
                line[id++] = '\"';
                line[id++] = '\"';
                line[id++] = '\'';
                line[id++] = ')';
                line[id++] = 0;
                system(line);
            }
        }
    }
    printf("------------------------------------------------------------\n");
    /* show text up to the end */
    {
        int cnt = 0;
        while (1)
        {
            int c;
            c = bestassert_gdbchar();
            // putchar(c)
            cnt++;
            if (c == -1) 
            {
                break;
            }
        }
        printf("Read %d unknown symbols\n", cnt);
    }

    /* show prompt */
    int result = 0;
    do
    {
        printf("\nSelect action: [q - quit from program, c - continue without actions, g - fall in gdb] >");
        result = _getch();
        if (isalpha(result))
        {
            printf("presed %c", result);
        }
    }
    while (result != 'q' && result != 'c' && result != 'g' && result != 'Q' && result != 'C' && result != 'G');
    printf("\n");
    
    if (result == 'q' || result == 'Q')
    {
        printf("Selected exit.\n");
        exit(3);
    }

    return result == 'g' || result == 'G';
}


void bestassert_connect_gdb()
{
    printf("Run gdb... :)\n");
    /* set console gdb input and output handles on this process handles? */
    bestassert_close_gdb();
    __asm__ __volatile__ ("int $3");
    bestassert_wait_to_close();
    bestassert_run_gdb(GetCurrentProcessId());

    #ifndef PREMIUM
        printf("\n\n\nwaiting... [data collecting. Buy premium to bypass speed limit :)]\n");
        Sleep(3000);
    #else
        bestassert_bestspinlock();
    #endif
}

void bestassert_close_gdb()
{
    printf("Close gdb... :)\n");
    {
        char input[1024] = {};
        sprintf(input, "q\n");
        bestassert_send_text(input);
    }
}

void bestassert_wait_to_close()
{
    printf("wait for closing... [close gdb window]\n");
    WaitForSingleObject(gdb.hProcess, INFINITE);
    printf("gdb closed. Best Assert section ends.\n\n");
    gdb = (PROCESS_INFORMATION){};
    hChildStdoutRd = NULL;
    hChildStdinWr = NULL;
}



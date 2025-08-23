#ifdef WIN32
    #include "windows.h"
	#include "conio.h"
#else
    #include "unistd.h"
    #include "spawn.h"
#endif
#include "stdio.h"
#include "fcntl.h"
#include "signal.h"

#include "my_assert.h"




#ifdef WIN32
    PROCESS_INFORMATION gdb = {};
    HANDLE hChildStdoutRd = NULL;
    HANDLE hChildStdinWr = NULL;
#else
    pid_t gdb_pid;
    long gdb_stdout;
    long gdb_stdin;
#endif


static void bestassert_bestspinlock()
{
	#ifdef WIN32
        while (!IsDebuggerPresent())
        {
            Sleep(50);
        }
    #else
        /* no, premium doesn't helps */
        sleep(3);
	#endif
}



static void bestassert_send_text(const char *commands)
{
    #ifdef WIN32
        DWORD written = 0;
        if (!WriteFile(hChildStdinWr, commands, strlen(commands), &written, NULL)) 
        {
            printf("WriteFile to stdin failed: %lu\n", GetLastError());
        }
    #else
        #error TODO
    #endif
}



void bestassert_run_gdb(int pid_to_attach)
{   
    #ifdef WIN32
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
    #else
        int user_friendly = (pid_to_attach != 0);
        
        if (pid_to_attach == 0)
        {
            pid_to_attach = getpid();
        }
        
        char string[64] = {};
        sprintf("%d", pid_to_attach);
        /* spawn new gdb */
        if (user_friendly)
        {
            const char *argv[] = {
                "gdb", "attach", string, "-ex", "cont", "-i=mi", "--silent", NULL
            }
            posix_spawn(&gdb_pid, "gdb", ???, NULL, argv, NULL);
        }
        else
        {
            const char *argv[] = {
                "gdb", "attach", string, "-ex", "cont", "-i=mi", "--silent", NULL
            }
            posix_spawn(&gdb_pid, "gdb", ???, NULL, argv, NULL);
        }
    #endif
}

void bestassert_attach()
{
    #ifdef WIN32
        char input[1024] = {};
        sprintf(input, "attach %ld\n"
                       "continue\n", GetCurrentProcessId());
        bestassert_send_text(input);
    #endif
}
    
void bestassert_request()
{
    char input[1024] = {};
    sprintf(input, "bt 99\n"
                   "info locals\n"
                   "continue\n");
    bestassert_send_text(input);

    /* wait for connection... */
    printf("waiting...\n");
    bestassert_bestspinlock();
}



static int bestassert_gdbchar()
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
        printf("\nSelect action:\n"
                                 "p - play game\n"
                                 "q - quit from program\n"
                                 "c - continue without actions\n"
                                 "g - fall in gdb\n"
                                 ">");
        result = _getch();
        if (isalpha(result))
        {
            printf("presed %c", result);
        }
        if (result == 'p' || result == 'P')
        {
            printf("\n");
            system("game.exe");
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
    printf("Starting gdb... [don't look at the logs about closing :)]\n");
    /* set console gdb input and output handles on this process handles? */
    if (gdb.hProcess)
    {
        bestassert_close_gdb();
        __asm__ __volatile__ ("int $3");
        bestassert_wait_to_close();
    }
    bestassert_run_gdb(GetCurrentProcessId());

    printf("waiting...\n");
    bestassert_bestspinlock();
    printf("Establish connection...\n");
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



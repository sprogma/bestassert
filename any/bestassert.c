#include "errno.h"
#include "stdio.h"
#include "fcntl.h"
#include "signal.h"
#include "ctype.h"
#include "string.h"

#include "best_assert.h"
#include "best_assert_local.h"


    
void bestassert_request()
{
    char input[2048] = {};

    /* read info file */
    sprintf(input, "bt 99\n"
                   "info locals\n");
    // for (int i = 0; i < n; ++i)
    // {
    //     sprintf(input + strlen(input), "continue\n");
    // }
    sprintf(input + strlen(input), "continue\n");
    
    bestassert_send_text(input);

    /* wait for connection... */
    printf("waiting...\n");
    bestassert_bestspinlock();
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
                #ifdef WIN32
                    char line[1024] = "py -c print('";
                #else
                    char line[1024] = "python3 -c print('";
                #endif
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
        #ifdef WIN32
            result = _getch();
        #else
            result = getchar();
        #endif
        if (isalpha(result))
        {
            printf("presed %c", result);
        }
        if (result == 'p' || result == 'P')
        {
            printf("\n");
            #ifdef WIN32
                system("game.exe");
            #else
                system("./game");
            #endif
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
    #ifdef WIN32
        if (gdb.hProcess)
    #else
        if (gdb_pid != 0)
    #endif
    {
        bestassert_close_gdb();
        __asm__ __volatile__ ("int $3");
        bestassert_wait_to_close();
    }
    
    printf("Running new instance...\n");
    
    #ifdef WIN32
        bestassert_run_gdb(GetCurrentProcessId());
    #else
        bestassert_run_gdb(getpid());
    #endif

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



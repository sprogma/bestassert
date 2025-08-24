#include "errno.h"
#include "stdio.h"
#include "fcntl.h"
#include "signal.h"
#include "ctype.h"
#include "string.h"

#include "best_assert.h"
#include "best_assert_local.h"




static void unquote(char *s)
{
    char *z = s;
    while (*z)
    {
        if (*z == '\\')
        {
            switch (*++z)
            {
                case '\\':
                    *s++ = '\\'; break;
                case 'n':
                    *s++ = '\n'; break;
                case 'r':
                    *s++ = '\r'; break;
                case 't':
                    *s++ = '\t'; break;
                case 'v':
                    *s++ = '\v'; break;
                case 'a':
                    *s++ = '\a'; break;
                case 'e':
                    *s++ = '\033'; break;
                case '\"':
                    *s++ = '\"'; break;
                case '\'':
                    *s++ = '\''; break;
                default:
                    s++;
            }
            z++;
        }
        else
        {
            *s++ = *z++;
        }
    }
    *s = 0;
}



    
void bestassert_request(struct gdb_instance *gdb)
{
    char input[2048] = {};

    /* read info file */
    sprintf(input, "bt 99\n"
                   "info locals\n");
    sprintf(input + strlen(input), "continue\n");
    
    bestassert_send_text(gdb, input);

    /* wait for connection... */
    printf("waiting...\n");
    bestassert_bestspinlock(gdb);
}


int bestassert_update(struct gdb_instance *gdb)
{
    /* read while there is no &bt */
    const char *text = "&\"bt";
    int next_id = 0;
    while (text[next_id] != 0)
    {
        int ch = bestassert_gdbchar(gdb);
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
        while (bestassert_gdbchar(gdb) != '\n');
        printf("------------------------------------------------------------\n");
        printf("                         %s\n", sections[sd]);
        /* parse stack trace */
        while (1)
        {
            int type = bestassert_gdbchar(gdb);
            if (type == '&')
            {
                break;
            }
            if (type == '~')
            {
                char line[1024];
                int ch = 0, id = 0;
                while (ch != '\n' && id + 2 < (int)sizeof(line))
                {
                    ch = line[id++] = bestassert_gdbchar(gdb);
                }
                /* remove last " */
                #ifdef WIN32
                    line[id - 3] = 0;
                #else
                    line[id - 2] = 0;
                #endif
                unquote(line + 1);
                puts(line + 1);
            }
        }
    }
    printf("------------------------------------------------------------\n");
    {
        int cnt = 0;
        while (1)
        {
            int c;
            c = bestassert_gdbchar(gdb);
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
                system("inner_game.exe");
            #else
                system("./inner_game");
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



void bestassert_reconnect_gdb(struct gdb_instance *gdb)
{
    printf("Starting gdb... [don't look at the logs about closing :)]\n");

    /* set console gdb input and output handles on this process handles? */
    #ifdef WIN32
        if (gdb->gdb.hProcess)
    #else
        if (gdb->gdb_pid != 0)
    #endif
    {
        bestassert_close_gdb(gdb);
        __asm__ __volatile__ ("int $3");
        bestassert_wait_to_close(gdb);
    }
    
    printf("Running new instance...\n");
    
    bestassert_run_gdb(gdb, 1);

    printf("waiting...\n");
    bestassert_bestspinlock(gdb);
    printf("Establish connection...\n");
}



void bestassert_close_gdb(struct gdb_instance *gdb)
{
    printf("Close gdb... :)\n");
    {
        char input[1024] = {};
        sprintf(input, "q\n");
        bestassert_send_text(gdb, input);
    }
}



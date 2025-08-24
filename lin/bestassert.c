#include "unistd.h"
#include "spawn.h"

#include "errno.h"
#include "stdio.h"
#include "fcntl.h"
#include "signal.h"
#include "ctype.h"
#include "string.h"

#include "best_assert.h"
#include "best_assert_local.h"



/* check /proc/self/status/TracerPid */
static int check_tracerpid() 
{
    FILE *f = fopen("/proc/self/status", "r");
    if (!f)
    {
        printf("Error: no access to file /proc/self/status\n");
        exit(4);
    }
    
    char line[1024];

    int tracer_pid = 0;
    
    while (fgets(line, sizeof(line), f)) 
    {
        if (strncmp(line, "TracerPid:", 10) == 0) 
        {
            char *p = line + 10;
            while (isblank(*p))
            {
                ++p;
            }
            tracer_pid = atoi(p);
            break;
        }
    }
    
    fclose(f);
    
    return tracer_pid != 0;
}


static int is_debugger_present_linux() 
{
    return check_tracerpid();
}



void bestassert_bestspinlock(struct gdb_instance *gdb)
{
    (void)gdb;
    while (!is_debugger_present_linux())
    {
        usleep(50 * 1000);
    }
}


static int make_nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        exit(4);
    }    
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}


void bestassert_send_text(struct gdb_instance *gdb, const char *commands)
{
    ssize_t nwrited = write(gdb->child_stdin_pipe[1], commands, strlen(commands));
    (void)nwrited;
}


void bestassert_run_gdb(struct gdb_instance *gdb, int user_friendly)
{   
    pid_t pid_to_attach = getpid();

    /* create pipes... */
    if (pipe(gdb->child_stdin_pipe) != 0) 
    {
        perror("pipe stdin");
        exit(4);
    }
    if (pipe(gdb->child_stdout_pipe) != 0) 
    {
        perror("pipe stdin");
        close(gdb->child_stdin_pipe[0]); close(gdb->child_stdin_pipe[1]);
        exit(4);
    }
    posix_spawn_file_actions_t fa;
    if (posix_spawn_file_actions_init(&fa) != 0) 
    {
        perror("posix_spawn_file_actions_init");
        exit(4);
    }

    
    char string[64] = {};
    sprintf(string, "%d", pid_to_attach);
    /* spawn new gdb */
    if (user_friendly)
    {
        char * const argv[] = {
            "gdb", "attach", string, "-ex", "cont", NULL
        };
        int err = posix_spawnp(&gdb->gdb_pid, "gdb", &fa, NULL, argv, NULL);
        if (err)
        {
            printf("posix_spawn error %d [%d].\n", err, errno);
            exit(4);
        }
        printf("Running...\n");
    }
    else
    {
        if (posix_spawn_file_actions_adddup2(&fa, gdb->child_stdin_pipe[0], STDIN_FILENO) != 0) 
        {
            perror("adddup2 stdin");
            exit(4);
        }
        if (posix_spawn_file_actions_adddup2(&fa, gdb->child_stdout_pipe[1], STDOUT_FILENO) != 0) 
        {
            perror("adddup2 stdout");
            exit(4);
        }
        if (posix_spawn_file_actions_adddup2(&fa, gdb->child_stdout_pipe[1], STDERR_FILENO) != 0) 
        {
            perror("adddup2 stderr");
            exit(4);
        }
        
        char * const argv[] = {
            "gdb", "attach", string, "-ex", "cont", "-i=mi", "--silent", NULL
        };
        int err = posix_spawnp(&gdb->gdb_pid, "gdb", &fa, NULL, argv, NULL);
        if (err)
        {
            printf("posix_spawn error %d [%d].\n", err, errno);
            exit(4);
        }
    }
    
    posix_spawn_file_actions_destroy(&fa);
    
    close(gdb->child_stdin_pipe[0]);
    close(gdb->child_stdout_pipe[1]);

    make_nonblock(gdb->child_stdout_pipe[0]);
}
    
int bestassert_gdbchar(struct gdb_instance *gdb)
{
    char c;
    int r = read(gdb->child_stdout_pipe[0], &c, 1);
    if (r == 1) 
    {
        return c;
    }
    else if (r == 0) 
    {
        printf("Error. EOF found.\n");
        exit(4);
    } 
    else 
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK) 
        {
            return -1;
        } 
        else 
        {
            printf("Some another error.\n");
            exit(4);
        }
    }
}

void bestassert_wait_to_close(struct gdb_instance *gdb)
{
    /* kill gdb :) */
    #ifdef KILL_GDB
        printf("Bad Penguin is killing gdb :)\n");
        if (kill(gdb->gdb_pid, SIGTERM) == -1)
        {
            printf("Warning: Kill error %d\n", errno);
        }
    #endif
    gdb->gdb_pid = 0;
    memset(gdb->child_stdout_pipe, 0, sizeof(gdb->child_stdout_pipe));
    memset(gdb->child_stdin_pipe, 0, sizeof(gdb->child_stdin_pipe));
}

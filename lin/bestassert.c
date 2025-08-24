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



void bestassert_bestspinlock(struct gdb_instance *gdb)
{
    (void)gdb;
    usleep(3000 * 1000);
}


static int make_nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    EXITIF(flags == -1, "cannot get fd flags");
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

    char string[64] = {};
    sprintf(string, "%d", pid_to_attach);

    /* create pipes... */
    if (!user_friendly)
    {
        int res = 0;
        
        res = pipe(gdb->child_stdin_pipe) != 0;
        EXITIF(res, "stdin creation failed");
        res = pipe(gdb->child_stdout_pipe) != 0;
        EXITIF(res, "stdout creation failed");
    }
    
    /* spawn new gdb */
    pid_t child_pid = fork();
    if (child_pid == 0)
    {
        if (user_friendly)
        {
            execlp("gdb", "gdb", "attach", string, "-ex", "cont", (char *)NULL);
        }
        else
        {
            dup2(gdb->child_stdin_pipe[0], 0);
            dup2(gdb->child_stdout_pipe[1], 1);
            dup2(gdb->child_stdout_pipe[1], 2);
            
            execlp("gdb", "gdb", "attach", string, "-ex", "cont", "-i=mi", "--silent", (char *)NULL);
        }
        exit(4);
    }

    gdb->gdb_pid = child_pid;

    if (!user_friendly)
    {
        close(gdb->child_stdin_pipe[0]);
        close(gdb->child_stdout_pipe[1]);
    }

    make_nonblock(gdb->child_stdout_pipe[0]);
}

    
int bestassert_gdbchar(struct gdb_instance *gdb)
{
    char c;
    int r = read(gdb->child_stdout_pipe[0], &c, 1);
    
    EXITIF(r == 0, "Error - read EOF in dialog with gdb");
    
    if (r == 1) 
    {
        return c;
    }
    else 
    {
        if (errno == EAGAIN || errno == EWOULDBLOCK) 
        {
            return -1;
        } 
        else 
        {
            EXITIF(1, "Some error in dialog with gdb");
        }
    }
    return -1;
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
    if (gdb->child_stdin_pipe[0])
    {
        close(gdb->child_stdin_pipe[0]);
    }
    if (gdb->child_stdout_pipe[1])
    {
       close(gdb->child_stdout_pipe[1]); 
    }
    gdb->gdb_pid = 0;
    memset(gdb->child_stdout_pipe, 0, sizeof(gdb->child_stdout_pipe));
    memset(gdb->child_stdin_pipe, 0, sizeof(gdb->child_stdin_pipe));
}

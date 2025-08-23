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




pid_t gdb_pid = 0;
int child_stdin_pipe[2] = {};
int child_stdout_pipe[2] = {};


int make_nonblock(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1)
    {
        exit(4);
    }    
    return fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}



void bestassert_bestspinlock()
{
    /* no, premium doesn't helps */
    sleep(3);
}



void bestassert_send_text(const char *commands)
{
    ssize_t nwrited = write(child_stdin_pipe[1], commands, strlen(commands));
    (void)nwrited;
}



void bestassert_run_gdb(int pid_to_attach)
{   
    int user_friendly = (pid_to_attach != 0);
    
    if (pid_to_attach == 0)
    {
        pid_to_attach = getpid();
    }

    /* create pipes... */
    if (pipe(child_stdin_pipe) != 0) 
    {
        perror("pipe stdin");
        exit(4);
    }
    if (pipe(child_stdout_pipe) != 0) 
    {
        perror("pipe stdin");
        close(child_stdin_pipe[0]); close(child_stdin_pipe[1]);
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
        int err = posix_spawnp(&gdb_pid, "gdb", &fa, NULL, argv, NULL);
        if (err)
        {
            printf("posix_spawn error %d [%d].\n", err, errno);
            exit(4);
        }
        printf("Running...\n");
    }
    else
    {
        if (posix_spawn_file_actions_adddup2(&fa, child_stdin_pipe[0], STDIN_FILENO) != 0) 
        {
            perror("adddup2 stdin");
            exit(4);
        }
        if (posix_spawn_file_actions_adddup2(&fa, child_stdout_pipe[1], STDOUT_FILENO) != 0) 
        {
            perror("adddup2 stdout");
            exit(4);
        }
        if (posix_spawn_file_actions_adddup2(&fa, child_stdout_pipe[1], STDERR_FILENO) != 0) 
        {
            perror("adddup2 stderr");
            exit(4);
        }
        
        char * const argv[] = {
            "gdb", "attach", string, "-ex", "cont", "-i=mi", "--silent", NULL
        };
        int err = posix_spawnp(&gdb_pid, "gdb", &fa, NULL, argv, NULL);
        if (err)
        {
            printf("posix_spawn error %d [%d].\n", err, errno);
            exit(4);
        }
    }
    
    posix_spawn_file_actions_destroy(&fa);
    
    close(child_stdin_pipe[0]);
    close(child_stdout_pipe[1]);

    make_nonblock(child_stdout_pipe[0]);
}

void bestassert_attach()
{
    /**/
}
    


int bestassert_gdbchar()
{
    char c;
    int r = read(child_stdout_pipe[0], &c, 1);
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

void bestassert_wait_to_close()
{
    /* kill gdb :) */
    printf("Bad Penguin is killing gdb :)\n");
    if (kill(gdb_pid, SIGTERM) == -1)
    {
        printf("Warning: Kill error %d\n", errno);
    }
    gdb_pid = 0;
    memset(child_stdout_pipe, 0, sizeof(child_stdout_pipe));
    memset(child_stdin_pipe, 0, sizeof(child_stdin_pipe));
}



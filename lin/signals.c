#include "stdio.h"
#include "time.h"
#include "unistd.h"
#include "signal.h"

#include "best_assert.h"
#include "best_assert_local.h"

#ifdef ADD_SIGNAL_HANDLERS
    static const char * signal_to_string(int signum)
    {
        #define ADD_SIGNAL(x) \
            case x: \
                return #x ;
        switch (signum)
        {
            ADD_SIGNAL(SIGINT)
            ADD_SIGNAL(SIGTERM)
            ADD_SIGNAL(SIGHUP)
            default:
                return "Unknown boring signal";
        }
    }

    static void universal_signal_handler_print_info(int signum, siginfo_t *info)
    {
        printf("--------------- Signal: ---------------\n");
        printf("Received signal: %d [%s]\n", signum, signal_to_string(signum));
        printf("number:                   %d\n", info->si_signo);
        printf("errno:                    %d\n", info->si_errno);
        printf("code:                     %d\n", info->si_code);
        printf("sender PID:               %d\n", info->si_pid);
        printf("sender user UID:          %d\n", info->si_uid);
        printf("status:                   %d\n", info->si_status);
        printf("user time consumed:       %lg s.\n", ((double)(info->si_utime)) / CLOCKS_PER_SEC);
        printf("system time consumed:     %lg s.\n", ((double)(info->si_stime)) / CLOCKS_PER_SEC);
        // printf("info value:               %d\n", info->si_value);
        printf("address [IP?]:            %p\n", info->si_addr);
    }

    static void universal_signal_handler(int signum, siginfo_t *info, void *context) 
    {
        (void)context;
        
        printf("Boring linux signal system catch signal\n");

        universal_signal_handler_print_info(signum, info);

        printf("For now, you can't run GDB from signam handler.\n");
        printf("Get real! Run Windows if you whant this.\n");
        // /* run gdb? */
        // /* ok, start new gdb session */
        // bestassert_run_gdb(1);
        // printf("waiting...\n");
        // bestassert_bestspinlock();
        // printf("Establish connection...\n");
        // __asm__ __volatile__ ("int $3");
        
        exit(0);
    }

    __attribute__((constructor))
    void signal_handler_init()
    {        
        static int not_enabled = 1;
        if (not_enabled)
        {
            printf("Are you sure you don't want to use Windows?\n");
            printf("Ok, boring linux signals enabled.\n");

            struct sigaction sa;
            sa.sa_sigaction = universal_signal_handler;
            sigfillset(&sa.sa_mask);    
            sa.sa_flags = SA_SIGINFO;

            sigaction(SIGINT, &sa, NULL);
            sigaction(SIGTERM, &sa, NULL);
            sigaction(SIGHUP, &sa, NULL);
            sigaction(SIGSEGV, &sa, NULL);
            
            not_enabled = 0;
        }
    }

    void signal_handler_init_function()
    {
        signal_handler_init();
    }
#endif

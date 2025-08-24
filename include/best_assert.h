#ifndef BEST_ASSERT
#define BEST_ASSERT

#include "stdlib.h"


#ifdef __cplusplus
extern "C" {
#endif


#ifdef WIN32
    #ifdef LIBRARY_EXPORT
    #    define EXPORT __declspec(dllexport)
    #else
    #    define EXPORT __declspec(dllimport)
    #endif
#else
    #define EXPORT
#endif



#ifdef WIN32
    #include "windows.h"
    struct gdb_instance
    {
        PROCESS_INFORMATION gdb;
        HANDLE hChildStdoutRd;
        HANDLE hChildStdinWr;
    };
#else
    #include "unistd.h"
    struct gdb_instance
    {
        pid_t gdb_pid;
        int child_stdin_pipe[2];
        int child_stdout_pipe[2];
    };
#endif

#define BESTASSERT(expr) \
    do \
    { \
        if (!(expr)) \
        { \
            fprintf(stderr, "\n\n-----------------------------------------" \
                            "\n    Oh no! best assertation occured!\n"); \
            fprintf(stderr, "AT: %s:%s:%d: expression (%s) == false\n", __FILE__, __FUNCTION__, __LINE__, #expr); \
            struct gdb_instance gdb; \
            bestassert_run_gdb(&gdb, 0); \
            bestassert_request(&gdb); \
            __asm__ __volatile__ ("int $3"); \
            int rungdb = bestassert_update(&gdb); \
            if (rungdb) \
            { \
                bestassert_reconnect_gdb(&gdb); \
                __asm__ __volatile__ ("int $3"); \
            } \
            else \
            { \
                bestassert_close_gdb(&gdb); \
                __asm__ __volatile__ ("int $3"); \
            } \
            bestassert_wait_to_close(&gdb); \
            printf("------------------------------------------\n"); \
            printf(" --------- BESTASSERT SECTION END --------\n"); \
            printf("------------------------------------------\n"); \
        }\
    } \
    while (0);


EXPORT void bestassert_run_gdb(struct gdb_instance *gdb, int user_friendly);
EXPORT void bestassert_request(struct gdb_instance *gdb);
EXPORT int bestassert_update(struct gdb_instance *gdb);
EXPORT void bestassert_reconnect_gdb(struct gdb_instance *gdb);
EXPORT void bestassert_close_gdb(struct gdb_instance *gdb);
EXPORT void bestassert_wait_to_close(struct gdb_instance *gdb);


#ifdef ADD_SIGNAL_HANDLERS
    EXPORT void signal_handler_init_function();
#endif

                        
#ifdef __cplusplus
}
#endif

#endif

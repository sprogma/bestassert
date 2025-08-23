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
	#define BESTASSERT(expr) \
	    do \
	    { \
	        if (!(expr)) \
	        { \
	            fprintf(stderr, "\n\n-----------------------------------------" \
	                            "\n    Oh no! best assertation occured!\n"); \
	            fprintf(stderr, "AT: %s:%s:%d: expression (%s) == false\n", __FILE__, __FUNCTION__, __LINE__, #expr); \
	            bestassert_run_gdb(0); \
	            bestassert_attach(); \
	            bestassert_request(); \
	            __asm__ __volatile__ ("int $3"); \
	            int rungdb = bestassert_update(); \
	            if (rungdb) \
	            { \
	                bestassert_connect_gdb(); \
	                __asm__ __volatile__ ("int $3"); \
	            } \
	            else \
	            { \
	                bestassert_close_gdb(); \
	                __asm__ __volatile__ ("int $3"); \
	            } \
	            bestassert_wait_to_close(); \
	        }\
	    } \
	    while (0);
#else
    #define EXPORT
	#define BESTASSERT(expr) \
	    do \
	    { \
	        if (!(expr)) \
	        { \
	            fprintf(stderr, "\n\n-----------------------------------------" \
	                            "\n    Oh no! best assertation occured!\n"); \
	            fprintf(stderr, "AT: %s:%s:%d: expression (%s) == false\n", __FILE__, __FUNCTION__, __LINE__, #expr); \
	            bestassert_run_gdb(0); \
	            bestassert_attach(); \
	            bestassert_request(); \
	            __asm__ __volatile__ ("int $3"); \
	            int rungdb = bestassert_update(); \
	            if (rungdb) \
	            { \
                    bestassert_connect_gdb(); \
                    __asm__ __volatile__ ("int $3"); \
	            } \
	            else \
	            { \
	                bestassert_close_gdb(); \
	                __asm__ __volatile__ ("int $3"); \
	            } \
	            bestassert_wait_to_close(); \
	        }\
	    } \
	    while (0);
#endif



#ifdef WIN32
    EXPORT void bestassert_run_gdb(int pid_to_attach);
#else
    EXPORT void bestassert_run_gdb(pid_t pid_to_attach);
#endif
EXPORT void bestassert_attach();
EXPORT void bestassert_request();
EXPORT int bestassert_update();
EXPORT void bestassert_connect_gdb();
EXPORT void bestassert_close_gdb();
EXPORT void bestassert_wait_to_close();

#ifdef __cplusplus
}
#endif

#endif

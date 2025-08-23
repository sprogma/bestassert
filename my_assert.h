#include "stdlib.h"


#ifdef WIN32
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
    void bestassert_run_gdb(int pid_to_attach);
#else
    void bestassert_run_gdb(pid_t pid_to_attach);
#endif
void bestassert_attach();
void bestassert_request();
int bestassert_update();
void bestassert_connect_gdb();
void bestassert_close_gdb();
void bestassert_wait_to_close();


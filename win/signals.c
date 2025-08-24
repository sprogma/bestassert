#include "stdio.h"

#include "best_assert.h"
#include "best_assert_local.h"


#ifndef STATUS_WX86_BREAKPOINT
#define STATUS_WX86_BREAKPOINT                   0x4000001F
#endif

#ifndef STATUS_POSSIBLE_DEADLOCK
#define STATUS_POSSIBLE_DEADLOCK                 0xC0000194
#endif

#ifndef DBG_THREAD_NAME
#define DBG_THREAD_NAME                          0x406D1388
#endif

#define EXCEPTION_CPP_MSC                        0xE06D7363
#define EXCEPTION_CPP_MSC_EH_MAGIC_NUMBER1       0x19930520
#define EXCEPTION_CPP_MSC_EH_MAGIC_NUMBER2       0x19930521
#define EXCEPTION_CPP_MSC_EH_MAGIC_NUMBER3       0x19930522
#define EXCEPTION_CPP_MSC_EH_PURE_MAGIC_NUMBER1  0x01994000

#define EXCEPTION_CPP_GCC                        0x20474343
#define EXCEPTION_CPP_GCC_UNWIND                 0x21474343
#define EXCEPTION_CPP_GCC_FORCED                 0x22474343
 
#define EXCEPTION_CLR_FAILURE                    0xE0434f4D
 
#define EXCEPTION_CPP_BORLAND_BUILDER            0x0EEDFAE6
#define EXCEPTION_CPP_BORLAND_DELPHI             0x0EEDFADE


#ifdef ADD_SIGNAL_HANDLERS
    static const char *ExceptionCodeToString(DWORD error_code)
    {   
        #define SWITCH_ERROR_CODE(x) \
            case x: \
                return #x ;

        switch (error_code)
        {

            SWITCH_ERROR_CODE(DBG_CONTROL_BREAK)
            SWITCH_ERROR_CODE(DBG_CONTROL_C)
            SWITCH_ERROR_CODE(DBG_PRINTEXCEPTION_C)
            SWITCH_ERROR_CODE(DBG_PRINTEXCEPTION_WIDE_C)
            SWITCH_ERROR_CODE(DBG_TERMINATE_PROCESS)
            SWITCH_ERROR_CODE(DBG_TERMINATE_THREAD)
            SWITCH_ERROR_CODE(DBG_THREAD_NAME)
            SWITCH_ERROR_CODE(EXCEPTION_ACCESS_VIOLATION)
            SWITCH_ERROR_CODE(EXCEPTION_ARRAY_BOUNDS_EXCEEDED)
            SWITCH_ERROR_CODE(EXCEPTION_BREAKPOINT)
            SWITCH_ERROR_CODE(EXCEPTION_CLR_FAILURE)
            SWITCH_ERROR_CODE(EXCEPTION_CPP_BORLAND_BUILDER)
            SWITCH_ERROR_CODE(EXCEPTION_CPP_BORLAND_DELPHI)
            SWITCH_ERROR_CODE(EXCEPTION_CPP_GCC)
            SWITCH_ERROR_CODE(EXCEPTION_CPP_GCC_FORCED)
            SWITCH_ERROR_CODE(EXCEPTION_CPP_GCC_UNWIND)
            SWITCH_ERROR_CODE(EXCEPTION_CPP_MSC)
            SWITCH_ERROR_CODE(EXCEPTION_DATATYPE_MISALIGNMENT)
            SWITCH_ERROR_CODE(EXCEPTION_FLT_DENORMAL_OPERAND)
            SWITCH_ERROR_CODE(EXCEPTION_FLT_DIVIDE_BY_ZERO)
            SWITCH_ERROR_CODE(EXCEPTION_FLT_INEXACT_RESULT)
            SWITCH_ERROR_CODE(EXCEPTION_FLT_INVALID_OPERATION)
            SWITCH_ERROR_CODE(EXCEPTION_FLT_OVERFLOW)
            SWITCH_ERROR_CODE(EXCEPTION_FLT_STACK_CHECK)
            SWITCH_ERROR_CODE(EXCEPTION_FLT_UNDERFLOW)
            SWITCH_ERROR_CODE(EXCEPTION_GUARD_PAGE)
            SWITCH_ERROR_CODE(EXCEPTION_ILLEGAL_INSTRUCTION)
            SWITCH_ERROR_CODE(EXCEPTION_IN_PAGE_ERROR)
            SWITCH_ERROR_CODE(EXCEPTION_INT_DIVIDE_BY_ZERO)
            SWITCH_ERROR_CODE(EXCEPTION_INT_OVERFLOW)
            SWITCH_ERROR_CODE(EXCEPTION_INVALID_DISPOSITION)
            SWITCH_ERROR_CODE(EXCEPTION_INVALID_HANDLE)
            SWITCH_ERROR_CODE(EXCEPTION_NONCONTINUABLE_EXCEPTION)
            SWITCH_ERROR_CODE(EXCEPTION_PRIV_INSTRUCTION)
            SWITCH_ERROR_CODE(EXCEPTION_SINGLE_STEP)
            SWITCH_ERROR_CODE(EXCEPTION_STACK_OVERFLOW)
            SWITCH_ERROR_CODE(RPC_S_SERVER_UNAVAILABLE)
            SWITCH_ERROR_CODE(RPC_S_UNKNOWN_IF)
            SWITCH_ERROR_CODE(STATUS_ASSERTION_FAILURE)
            SWITCH_ERROR_CODE(STATUS_FLOAT_MULTIPLE_FAULTS)
            SWITCH_ERROR_CODE(STATUS_POSSIBLE_DEADLOCK)
            SWITCH_ERROR_CODE(STATUS_STACK_BUFFER_OVERRUN)
            SWITCH_ERROR_CODE(STATUS_WX86_BREAKPOINT)
            default:
                return "Unknown error code.";
        }
        #undef SWITCH_ERROR_CODE
    }

    static void PrintVEHExceptionRecordDynamicParameters(EXCEPTION_RECORD *Record)
    {
        printf("- Total parameters: %ld\n", Record->NumberParameters);
        for (DWORD i = 0; i < Record->NumberParameters; ++i)
        {
            printf("    - Info[%lu]=%I64d\n", i, Record->ExceptionInformation[i]);
        }
        
    }

    static void PrintVEHExceptionRecord(EXCEPTION_RECORD *Record)
    {   
        printf("----------------------\n");
        printf("VEH ExceptionRecord At %p:\n", Record);
        {
            DWORD ecode = Record->ExceptionCode;
            printf("- error code: %ld [%s]\n",
                                ecode,
                                ExceptionCodeToString(ecode));
            printf("      see https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-exception_record\n");
            printf("      to find more info about exception.\n");
        }
        {
            DWORD flags = Record->ExceptionFlags;
            printf("error flags: %ld\n", flags);
            if (flags & EXCEPTION_NONCONTINUABLE)
            {
                printf("- Warning: EXCEPTION_NONCONTINUABLE:\n");
                printf("      this exception cannot be continued.\n");
                printf("      [will cause EXCEPTION_NONCONTINUABLE_EXCEPTION]\n");
            }
        }
        printf("- Exception Address IP=%p\n", Record->ExceptionAddress);

        PrintVEHExceptionRecordDynamicParameters(Record);
        
        if (Record->ExceptionRecord)
        {
            printf("Printing associated ExceptionRecord at %p\n", Record->ExceptionRecord);
            PrintVEHExceptionRecord(Record->ExceptionRecord);
        }
        else
        {
            printf("No more associated ExceptionRecords\n");            
        }
        
        printf("----------------------\n");
    }

    static LONG BestassertHandler(EXCEPTION_POINTERS *ExceptionInfo)
    {           
        struct gdb_instance gdb;
        
        printf("--------------------------------------\n");
        printf("------- VEH? catched exception: -------\n");
        PrintVEHExceptionRecord(ExceptionInfo->ExceptionRecord);
        printf("-------------------------------------\n");
        /* ok, start new gdb session */
        bestassert_run_gdb(&gdb, 1);
        printf("waiting...\n");
        bestassert_bestspinlock(&gdb);
        printf("Establish connection...\n");
        #ifdef WIN_RUN_GDB_IN_ERROR_HANDLER
            __asm__ __volatile__ ("int $3");
        #endif
        return EXCEPTION_CONTINUE_SEARCH;
        // return EXCEPTION_CONTINUE_EXECUTION;
    }


    __attribute__((constructor))
    void signal_handler_init()
    {
        static BOOL not_enabled = TRUE;
        if (not_enabled)
        {
            printf("VEH? enabled.\n");
            SetUnhandledExceptionFilter(&BestassertHandler);
            not_enabled = FALSE;
        }
    }

    void signal_handler_init_function()
    {
        signal_handler_init();
    }
#endif

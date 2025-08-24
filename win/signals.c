#include "stdio.h"

#include "best_assert.h"
#include "best_assert_local.h"

#ifdef ADD_SIGNAL_HANDLERS
    static const char *ExceptionCodeToString(DWORD error_code)
    {   
        #define SWITCH_ERROR_CODE(x) \
            case x: \
                return #x ; \

        switch (error_code)
        {
            SWITCH_ERROR_CODE(EXCEPTION_ACCESS_VIOLATION)
            SWITCH_ERROR_CODE(EXCEPTION_ARRAY_BOUNDS_EXCEEDED)
            SWITCH_ERROR_CODE(EXCEPTION_BREAKPOINT)
            SWITCH_ERROR_CODE(EXCEPTION_DATATYPE_MISALIGNMENT)
            SWITCH_ERROR_CODE(EXCEPTION_FLT_DENORMAL_OPERAND)
            SWITCH_ERROR_CODE(EXCEPTION_FLT_DIVIDE_BY_ZERO)
            SWITCH_ERROR_CODE(EXCEPTION_FLT_INEXACT_RESULT)
            SWITCH_ERROR_CODE(EXCEPTION_FLT_INVALID_OPERATION)
            SWITCH_ERROR_CODE(EXCEPTION_FLT_OVERFLOW)
            SWITCH_ERROR_CODE(EXCEPTION_FLT_STACK_CHECK)
            SWITCH_ERROR_CODE(EXCEPTION_FLT_UNDERFLOW)
            SWITCH_ERROR_CODE(EXCEPTION_ILLEGAL_INSTRUCTION)
            SWITCH_ERROR_CODE(EXCEPTION_IN_PAGE_ERROR)
            SWITCH_ERROR_CODE(EXCEPTION_INT_DIVIDE_BY_ZERO)
            SWITCH_ERROR_CODE(EXCEPTION_INT_OVERFLOW)
            SWITCH_ERROR_CODE(EXCEPTION_INVALID_DISPOSITION)
            SWITCH_ERROR_CODE(EXCEPTION_NONCONTINUABLE_EXCEPTION)
            SWITCH_ERROR_CODE(EXCEPTION_PRIV_INSTRUCTION)
            SWITCH_ERROR_CODE(EXCEPTION_SINGLE_STEP)
            SWITCH_ERROR_CODE(EXCEPTION_STACK_OVERFLOW)
            default:
                return "Unknown error code."
        }
        #undef SWITCH_ERROR_CODE
    }

    static void PrintVEHExceptionRecordDynamicParameters(EXCEPTION_RECORD *Record)
    {
        printf("- Total parameters: %d\n", Record->NumberParameters);
        for (int i = 0; i < Record->NumberParameters; ++i)
        {
            printf("    - Info[%d]=%p\n", i, Record->ExceptionInformation[i]);
        }
        
    }

    static void PrintVEHExceptionRecord(EXCEPTION_RECORD *Record)
    {   
        printf("----------------------\n");
        printf("VEH ExceptionRecord At %p:\n", Record);
        {
            DWORD ecode = Record->ExceptionCode;
            printf("- error code: %ld [%s]\n",
            printf("      see https://learn.microsoft.com/en-us/windows/win32/api/winnt/ns-winnt-exception_record\n",
            printf("      to find more info about exception.\n",
                    ecode,
                    ExceptionCodeToString(ecode));
        }
        {
            DWORD flags = Record->ExceptionFlags;
            printf("error flags: %ld\n", flags);
            if (flags & EXCEPTION_NONCONTINUABLE)
            {
                printf("- Warning: EXCEPTION_NONCONTINUABLE:\n")
                printf("      this exception cannot be continued.\n")
                printf("      [will cause EXCEPTION_NONCONTINUABLE_EXCEPTION]\n")
            }
        }
        printf("- Exception Address IP=%p\n", Record->ExceptionAddress);

        PrintVEHExceptionRecordDynamicParameters(Record);
        
        if (Record->ExceptionRecord)
        {
            printf("Printing Associated ExceptionRecord at %p\n", Record->ExceptionRecord);
            PrintVEHExceptionRecord(Record->ExceptionRecord);
        }
        
        printf("----------------------\n");
    }

    static LONG BestassertVEH(EXCEPTION_POINTERS *ExceptionInfo)
    {           
        printf("--------------------------------------\n");
        printf("------- VEH catched exception: -------\n");
        PrintVEHExceptionRecord(ExceptionInfo->ExceptionRecord);
        printf("-------------------------------------\n");
        /* ok, start new gdb session */
        bestassert_run_gdb(1);
        printf("waiting...\n");
        bestassert_bestspinlock();
        printf("Establish connection...\n");
        __asm__ __volatile__ ("int $3");
        return EXCEPTION_CONTINUE_EXECUTION;
    }


    __attribute__((constructor))
    void signal_handler_init_function()
    {
        AddVectoredContinueHandler(0, &BestassertVEH);
    }
#endif

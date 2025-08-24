#include "stdio.h"

#define ADD_SIGNAL_HANDLERS
#include "best_assert.h"



int main()
{    
    signal_handler_init_function();

    printf("I am bad!\n");
    
    15[(int *)NULL] = 16;
    
    printf("End main!\n");
    
    return 0;
}

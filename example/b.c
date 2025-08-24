#include "stdio.h"
#include "best_assert.h"



int main()
{
    printf("I am bad!\n");
    
    *(NULL + 15) = 16;
    
    printf("End main!\n");
    return 0;
}

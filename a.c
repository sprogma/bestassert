#include "stdio.h"
#include "my_assert.h"


int find_sum(int n)
{
    return n * (n - 1) / 2;
}


void testing()
{    
    int sum = 0;
    for (int i = 1; i <= 10; ++i)
    {
        sum += i;
    }

    int sum2 = find_sum(10);
    
    BESTASSERT(sum == 55);
    
    BESTASSERT(sum == sum2);
}


int main()
{
    testing();
    printf("End main!\n");
    return 0;
}

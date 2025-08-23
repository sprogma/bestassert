#include "stdio.h"

int main()
{   
    int a = 0;
    while (1)
    {
        a = getchar();
        a++;
        __debugbreak();
    }
    return 0;
}

gcc -Wall -Wextra -DCREATE_NEW_CONSOLE_FOR_GDB -DADD_SIGNAL_HANDLERS -DKILL_GDB -Iinclude -fPIC -fpic -O0 -g -c any/bestassert.c -o any/bestassert.o
gcc -Wall -Wextra -DCREATE_NEW_CONSOLE_FOR_GDB -DADD_SIGNAL_HANDLERS -DKILL_GDB -Iinclude -fPIC -fpic -O0 -g -c lin/bestassert.c -o lin/bestassert.o
gcc -Wall -Wextra -DCREATE_NEW_CONSOLE_FOR_GDB -DADD_SIGNAL_HANDLERS -DKILL_GDB -Iinclude -fPIC -fpic -O0 -g -c lin/signals.c -o lin/signals.o
gcc -fPIC -fpic any/bestassert.o lin/bestassert.o lin/signals.o -shared -o libbestassert.so

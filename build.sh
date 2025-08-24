gcc game/game.c -o game 2>/dev/null

gcc -Wall -Iinclude -DCREATE_NEW_CONSOLE_FOR_GDB -DADD_SIGNAL_HANDLERS -DKILL_GDB -O0 -g -c any/bestassert.c -o any/bestassert.o
gcc -Wall -Iinclude -DCREATE_NEW_CONSOLE_FOR_GDB -DADD_SIGNAL_HANDLERS -DKILL_GDB -O0 -g -c lin/bestassert.c -o lin/bestassert.o
gcc -Wall -Iinclude -DCREATE_NEW_CONSOLE_FOR_GDB -DADD_SIGNAL_HANDLERS -DKILL_GDB -O0 -g -c lin/signals.c -o lin/signals.o

gcc -fpic -fPIC any/bestassert.o lin/signals.o lin/bestassert.o -shared -o "libbestassert.so"

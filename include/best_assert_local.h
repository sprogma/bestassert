#ifndef BEST_ASSERT_LOCAL
#define BEST_ASSERT_LOCAL

#include "stdlib.h"

#include "best_assert.h"

#ifdef WIN32
    #include "conio.h"
#else
    #include "spawn.h"
#endif

void bestassert_send_text(struct gdb_instance *gdb, const char *);
int bestassert_gdbchar(struct gdb_instance *gdb);
void bestassert_bestspinlock(struct gdb_instance *gdb);



#endif

#include "cli.h"
#define GC_DEBUG
#include "gc.h"

int main(int argc, char** argv) {
#ifndef NO_GC
    gc_init();
#endif
    CliOptions *options = parse_cli(argc, argv);
    if (options == NULL) {
        return 1;
    }
    int status = run_cli(options);
#ifndef NO_GC
#ifdef GC_DEBUG
    printf("the program allocated %lu bytes\n", global_gc->size);
#endif
    gc_collect();
#endif

    return status;
}


#include "cli.h"
#include "gc.h"

int main(int argc, char** argv) {
    gc_init();
    CliOptions *options = parse_cli(argc, argv);
    if (options == NULL) {
        return 1;
    }
    int status = run_cli(options);
    gc_dump();
    gc_collect();
    gc_dump();

    return status;
}


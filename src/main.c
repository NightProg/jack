#include "cli.h"

int main(int argc, char** argv) {
    CliOptions *options = parse_cli(argc, argv);
    if (options == NULL) {
        return 1;
    }
    return run_cli(options);
}


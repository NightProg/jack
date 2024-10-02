#ifndef JACK_CLI_H
#define JACK_CLI_H

#include <stdio.h>
#include "string.h"
#include "gc.h"

typedef struct {
    int help;
    int version;
    int no_std;
    int no_libc;
    int compile_only;
    int generate_assembly;
    int generate_symbols;
    int generate_llvm_ir;
    int generate_static_lib;
    char *output_file;
    StringList *input_files;
} CliOptions;

void print_help();
void print_version();

CliOptions *parse_cli(int argc, char **argv);
int run_cli(CliOptions *options);

#endif //JACK_CLI_H

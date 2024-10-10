#define VERBOSE
#include "x.h"


int main(int argc, char** argv) {

    FILE* cflags_fp = popen_cmd(CMD("llvm-config", "--cflags"));
    FILE* ldflags_fp = popen_cmd(CMD("llvm-config", "--ldflags"));

    char cflags[1024];
    char ldflags[1024];

    fgets(cflags, 1024, cflags_fp);
    fgets(ldflags, 1024, ldflags_fp);
    rtrim(ldflags);
    Target* jack = new_target("jack", EXECUTABLE, NULL);
    set_target_compiler(jack, "gcc");
    add_target_source_pattern(jack, "src/*.c");
    add_target_flag(jack, "-Wall");
    add_target_flag(jack, "-Wextra");
    add_target_flag(jack, cflags);
    add_target_lib(jack, ldflags);
    add_target_lib(jack, "-lLLVM-18");
    set_target_output_file(jack, "jack");
    set_target_description(jack, "Compile the jack executable");

    Target* jack_debug = new_target("jack_debug", EXECUTABLE, NULL);
    set_target_compiler(jack_debug, "gcc");
    add_target_source_pattern(jack_debug, "src/*.c");
    add_target_flag(jack_debug, "-Wall");
    add_target_flag(jack_debug, "-Wextra");
    add_target_flag(jack_debug, "-g");
    add_target_flag(jack_debug, cflags);
    add_target_lib(jack_debug, ldflags);
    add_target_lib(jack_debug, "-lLLVM-18");
    set_target_output_file(jack_debug, "jack_debug");
    set_target_description(jack_debug, "Compile the jack executable with debug symbols");


    Target *x = new_target("x", EXECUTABLE, NULL);
    set_target_output_file(x, "x");
    set_target_compiler(x, "gcc");
    add_target_source(x, "x.c");
    set_target_description(x, "Compile the x executable");

    Target *syms = new_target("libc", OTHER, NULL);
    add_target_cmd(syms, CMD("cp", "lib/libc.jack", "~/.jack/"));
    set_target_description(syms, "Generate libc symbols");

    Target *std = new_target("std", OTHER, NULL);
    add_target_cmd(std, CMD("cp", "lib/std.jack", "~/.jack/"));
    set_target_description(std, "Generate std symbols");


    TargetCli *cli = new_target_cli();
    append_target_cli(cli, jack);
    append_target_cli(cli, x);
    append_target_cli(cli, syms);
    append_target_cli(cli, std);
    append_target_cli(cli, jack_debug);

    target_cli(cli, argc, argv);

    return 0;
}
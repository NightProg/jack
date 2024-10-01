#include "cli.h"
#include "parser.h"
#include "llvm_codegen.h"
#include "type.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <unistd.h>
#include <llvm-c/Target.h>
#include <llvm-c/ExecutionEngine.h>

void print_help() {
    printf("Usage: jack [options] <source file>\n");
    printf("Options:\n");
    printf("  -h, --help     Print this help message\n");
    printf("  -v, --version  Print the version\n");
    printf("  -o <file>      Output file\n");
    printf("  -c             Compile only\n");
    printf("  -S             Generate assembly\n");
    printf("  --static       Generate Static library\n");
    printf("  --emit-llvm    Generate LLVM IR\n");
    printf("  --no-std       No std support\n");
    printf("  --no-libc      No libc support\n");
    printf("  -s, --symbols  generate symbols\n");
}

void print_version() {
    printf("jack 0.1\n");
}

CliOptions *parse_cli(int argc, char **argv) {
    CliOptions *options = malloc(sizeof(CliOptions));
    if (options == NULL) {
        return NULL;
    }
    options->help = 0;
    options->version = 0;
    options->no_std = 0;
    options->no_libc = 0;
    options->compile_only = 0;
    options->generate_assembly = 0;
    options->generate_symbols = 0;
    options->output_file = NULL;
    options->input_files = new_string_list();
    if (options->input_files == NULL) {
        free(options);
        return NULL;
    }

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-h") == 0 || strcmp(argv[i], "--help") == 0) {
            options->help = 1;
        } else if (strcmp(argv[i], "-v") == 0 || strcmp(argv[i], "--version") == 0) {
            options->version = 1;
        } else if (strcmp(argv[i], "-o") == 0) {
            if (i + 1 >= argc) {
                printf("Missing output file\n");
                free(options);
                return NULL;
            }
            options->output_file = argv[i + 1];
            i++;
        } else if (strcmp(argv[i], "-c") == 0) {
            options->compile_only = 1;
        } else if (strcmp(argv[i], "-S") == 0) {
            options->generate_assembly = 1;
        } else if (strcmp(argv[i], "-s") == 0 || strcmp(argv[i], "--symbols") == 0) {
            options->generate_symbols = 1;
        } else if (strcmp(argv[i], "--emit-llvm") == 0) {
            options->generate_llvm_ir = 1;
        } else if (strcmp(argv[i], "--static") == 0) {
            options->generate_static_lib = 1;
        } else {
            append_string(options->input_files, new_string(argv[i]));
        }
    }

    return options;
}

int run_cli(CliOptions *options) {
    if (options->help) {
        print_help();
        return 0;
    }
    if (options->version) {
        print_version();
        return 0;
    }

    for (int i = 0; i < options->input_files->size; i++) {
        char* source_file = options->input_files->strings[i]->data;

        FILE *source_fp = fopen(source_file, "r");
        if (source_fp == NULL) {
            printf("Failed to open file: %s\n", source_file);
            return 1;
        }
        fseek(source_fp, 0, SEEK_END);
        long source_size = ftell(source_fp);
        fseek(source_fp, 0, SEEK_SET);
        char *source = malloc(source_size + 1);
        fread(source, 1, source_size, source_fp);
        source[source_size] = '\0';
        fclose(source_fp);

        StmtList *stmts = parse(source, source_file);
        if (stmts == NULL) {
            print_error_list();
            return 1;
        }
        Symbols *symbols = new_symbols();
        if (symbols == NULL) {
            printf("Failed to merge symbols\n");
            return 1;
        }

        TypeChecker *type_checker = new_type_checker(symbols, source_file);
        if (type_checker == NULL) {
            printf("Failed to create type checker\n");
            return 1;
        }
        if (!check_stmt_list(type_checker, stmts)) {
            print_error_list();
            return 1;
        }
        Symbols *tc_symbols = type_checker->symbols;
        LLVMCodeGen *codegen = new_llvm_codegen(stmts, tc_symbols, source_file);
        if (codegen == NULL) {
            printf("Failed to create codegen\n");
            return 1;
        }
        llvm_codegen(codegen);
        if (options->generate_llvm_ir) {
            LLVMDumpModule(codegen->module);
            return 0;
        }
        LLVMInitializeAllAsmParsers();
        LLVMInitializeAllAsmPrinters();
        LLVMInitializeAllTargetInfos();
        LLVMInitializeAllTargets();
        LLVMInitializeAllTargetMCs();
        if (options->compile_only || options->generate_assembly || options->generate_static_lib) {
            const char *target_triple = LLVMGetDefaultTargetTriple();
            LLVMTargetRef target;
            if (LLVMGetTargetFromTriple(target_triple, &target, NULL)) {
                printf("Failed to get target\n");
                return 1;
            }
            LLVMTargetMachineRef target_machine = LLVMCreateTargetMachine(target, target_triple, "", "", LLVMCodeGenLevelDefault, LLVMRelocDefault, LLVMCodeModelDefault);
            LLVMTargetDataRef target_data = LLVMCreateTargetDataLayout(target_machine);
            LLVMModuleRef module = codegen->module;
            LLVMSetTarget(module, target_triple);
            LLVMSetDataLayout(module, LLVMCopyStringRepOfTargetData(target_data));
            // generate a object file

            char* output_file = options->output_file;
            if (output_file == NULL) {
                output_file = malloc(strlen(source_file) + 3);
                strcpy(output_file, source_file);
                strcat(output_file, ".o");
            }
            char *error = NULL;
            LLVMCodeGenFileType file_type;
            if (options->generate_assembly) {
                file_type = LLVMAssemblyFile;
            }  else {
                file_type = LLVMObjectFile;
            }
            if (LLVMTargetMachineEmitToFile(target_machine, module, output_file, file_type, &error)) {
                printf("Failed to emit object file: %s\n", error);
                LLVMDisposeMessage(error);
                return 1;
            }
            if (options->output_file == NULL) {
                free(output_file);
            }
            if (options->compile_only || options->generate_assembly) {
                return 0;
            }
            // generate a static library
            if (options->generate_static_lib) {
                char* static_lib_file = malloc(strlen(output_file) + 6);
                strcpy(static_lib_file, "lib");
                strcat(static_lib_file, output_file);
                strcat(static_lib_file, ".a");
                char *ar_args[] = {"ar", "rcs", static_lib_file, output_file, NULL};
                execvp("ar", ar_args);

            }
            return 0;
        }

        LLVMExecutionEngineRef engine;
        char *error = NULL;
        if (LLVMCreateExecutionEngineForModule(&engine, codegen->module, &error)) {
            printf("Failed to create execution engine: %s\n", error);
            LLVMDisposeMessage(error);
            return 1;
        }
        LLVMValueRef main_function = llvm_get_function(codegen, "main");
        if (main_function == NULL) {
            printf("Failed to get main function\n");
            return 1;
        }
        typedef int (*MainFunction)();
        MainFunction main = (MainFunction)LLVMGetFunctionAddress(engine, "main");
        int result = main();
        LLVMDisposeExecutionEngine(engine);
        free_symbols(symbols);
        return result;

    }

    return 0;
}
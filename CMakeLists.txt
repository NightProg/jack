cmake_minimum_required(VERSION 3.28)
project(jack C)

set(CMAKE_C_STANDARD 17)

include_directories(/usr/local/Cellar/llvm/18.1.8/include)
link_directories(/usr/local/Cellar/llvm/18.1.8/lib)


add_executable(jack src/main.c
        src/lexer.h
        src/lexer.c
        src/string.c
        src/string.h
        src/ast.c
        src/ast.h
        src/parser.c
        src/parser.h
        src/type.c
        src/type.h
        src/span.c
        src/span.h
        src/llvm_codegen.c
        src/llvm_codegen.h
        src/error.c
        src/error.h
        src/collection.c
        src/collection.h
        src/cli.c
        src/cli.h
        src/mangler.c
        src/mangler.h
        src/optimizer.c
        src/optimizer.h
        src/gc.c
        src/gc.h)

target_link_libraries(jack LLVM-18)

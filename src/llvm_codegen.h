#ifndef JACK_LLVM_CODEGEN_H
#define JACK_LLVM_CODEGEN_H

#include <llvm-c/Core.h>

#include "ast.h"
#include "gc.h"

typedef struct {
    String *name;
    LLVMTypeRef type;
    Param *params;
} LLVMStructInfo;

LLVMStructInfo *new_llvm_struct_info(String *name, LLVMTypeRef type, Param *params);
void free_llvm_struct_info(LLVMStructInfo *info);

typedef struct {
    LLVMStructInfo **structs;
    size_t size;
    size_t capacity;
} LLVMStructInfoList;

LLVMStructInfoList *new_llvm_struct_info_list();
int append_llvm_struct_info(LLVMStructInfoList *list, String *name, LLVMTypeRef type, Param *params);
LLVMStructInfo *get_llvm_struct_info(LLVMStructInfoList *list, String *name);
void free_llvm_struct_info_list(LLVMStructInfoList *list);

typedef struct {
    String *name;
    LLVMValueRef value;
    LLVMTypeRef type;
} LLVMVarInfo;

LLVMVarInfo *new_llvm_var_info(String *name, LLVMValueRef value, LLVMTypeRef type);
void free_llvm_var_info(LLVMVarInfo *info);

typedef struct {
    LLVMVarInfo **vars;
    size_t size;
    size_t capacity;
} LLVMVarInfoList;

LLVMVarInfoList *new_llvm_var_info_list();
int append_llvm_var_info(LLVMVarInfoList *list, String *name, LLVMValueRef value, LLVMTypeRef type);
int clear_llvm_var_info(LLVMVarInfoList *list);
LLVMVarInfoList *filter_llvm_var_info(LLVMVarInfoList *list);
LLVMVarInfo *get_llvm_var_info(LLVMVarInfoList *list, String *name);
void free_llvm_var_info_list(LLVMVarInfoList *list);

typedef struct {
    LLVMModuleRef module;
    LLVMBuilderRef builder;
    LLVMContextRef context;
    LLVMValueRef current_function;
    Symbols *symbols;
    StmtList *stmts;
    LLVMVarInfoList *vars;
    LLVMStructInfoList* structs;
    int no_load;
    int should_be_i64;
    String *current_module;
} LLVMCodeGen;

LLVMCodeGen *new_llvm_codegen(StmtList *stmts, Symbols *symbols, const char *module_name);
void llvm_codegen(LLVMCodeGen *codegen);
void llvm_codegen_stmt(LLVMCodeGen *codegen, Stmt *stmt);
void* llvm_get_function(LLVMCodeGen *codegen, const char *name);
void free_llvm_codegen(LLVMCodeGen *codegen);


#endif //JACK_LLVM_CODEGEN_H


#ifndef JACK_MANGLER_H
#define JACK_MANGLER_H
#include "ast.h"
#include "string.h"
#include "gc.h"

String *mangle_struct_method(String *struc_name, String *method_name);
String *mangle_module(String *module_name, String *symbol_name);
String *mangle_namespace(StringList *namespace);
String *mangle_extension(Type *type, String *method_name);
String *mangle_op_overload(int op, String *sname);


#endif //JACK_MANGLER_H

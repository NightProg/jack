
#ifndef JACK_MANGLER_H
#define JACK_MANGLER_H
#include "ast.h"
#include "string.h"

String *mangle_struct_method(String *struc_name, String *method_name);
String *mangle_module(String *module_name, String *symbol_name);
String *mangle_namespace(StringList *namespace);


#endif //JACK_MANGLER_H

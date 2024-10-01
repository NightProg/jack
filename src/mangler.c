#include "mangler.h"

String *mangle_struct_method(String *struc_name, String *method_name) {
    String *mangled = new_string("");
    add_string(mangled, struc_name);
    add_string(mangled, new_string("_"));
    add_string(mangled, method_name);
    return mangled;
}

String *mangle_module(String *module_name, String *symbol_name) {
    String *mangled = new_string("");
    add_string(mangled, module_name);
    add_string(mangled, new_string("_"));
    add_string(mangled, symbol_name);
    return mangled;
}

String *mangle_namespace(StringList *namespace) {
    String *mangled = new_string("");
    for (size_t i = 0; i < namespace->size; i++) {
        add_string(mangled, namespace->strings[i]);
        if (i + 1 < namespace->size) {
            add_string(mangled, new_string("_"));
        }
    }
    return mangled;
}
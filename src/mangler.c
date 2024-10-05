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

String *mangle_extension(Type *type, String *method_name) {
    String *mangled = new_string("");
    add_string(mangled, display_type(type));
    add_string(mangled, new_string("_"));
    add_string(mangled, method_name);
    return mangled;
}

String *mangle_op_overload(int op, String *sname) {
    String *mangled;
    switch (op) {
        case BINOP_ADD: {
            mangled = new_string("add_");
            break;
        }
        case BINOP_SUB: {
            mangled = new_string("sub_");
            break;
        }
        case BINOP_MUL: {
            mangled = new_string("mul_");
            break;
        }
        case BINOP_DIV: {
            mangled = new_string("div_");
            break;
        }
        case BINOP_EQ: {
            mangled = new_string("eq_");
            break;
        }
        case BINOP_NEQ: {
            mangled = new_string("neq_");
            break;
        }
        case BINOP_LT: {
            mangled = new_string("lt_");
            break;
        }
        case BINOP_GT: {
            mangled = new_string("gt_");
            break;
        }
        case BINOP_LTE: {
            mangled = new_string("lte_");
            break;
        }
        case BINOP_GTE: {
            mangled = new_string("gte_");
            break;
        }
        default: {
            mangled = new_string("");
            break;
        }
    }
    add_string(mangled, sname);
    return mangled;
}
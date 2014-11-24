#ifndef _PRESENT_HEADER_
#define _PRESENT_HEADER_

#include "ast.h"
#include <typeinfo>
#include <string>

std::string ast_string(ASTNode *);
#define TYPE_NAME(obj) (typeid(obj).name())
#define IS_SAME_TYPE(a, b) (TYPE_NAME(a) == TYPE_NAME(b))

#define IS_PANY(obj, type) (IS_SAME_TYPE(*(obj), type))

#define P_TRANS(to, from, type) type *to = (type *) from
#define TAB "\t"

#endif

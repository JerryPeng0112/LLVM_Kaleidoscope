#ifndef UTILS_H
#define UTILS_H

#include "AST.h"

// LogError - helper function for error handling.
std::unique_ptr<ExprAST> log_error(const char *str) {
    fprintf(stderr, "Log error: %s\n", str);
    return nullptr;
}

#endif

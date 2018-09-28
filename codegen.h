#ifndef CODEGEN_H
#define CODEGEN_H

#include "AST.h"
#include "utils.h"

LLVMContext context;
IRBuilder<> Builder(context);
std::unique_ptr<Module> module;
std::map<std::string, Value *> named_values;

Value* log_error_v(const char *str);

#endif

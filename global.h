#ifndef GLOBAL_H
#define GLOBAL_H

#include "llvm/ADT/APFloat.h"
#include "llvm/ADT/STLExtras.h"
#include "llvm/IR/BasicBlock.h"
#include "llvm/IR/Constants.h"
#include "llvm/IR/DerivedTypes.h"
#include "llvm/IR/Function.h"
#include "llvm/IR/IRBuilder.h"
#include "llvm/IR/LLVMContext.h"
#include "llvm/IR/Module.h"
#include "llvm/IR/Type.h"
#include "llvm/IR/Verifier.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace llvm;

// lexer variables
extern std::string identifier_str;
extern double num_val;

// bin_op_precedence - This holds the precedence for each binary operator that is defined.
extern std::map<char, int> bin_op_precedence;

// LLVM context global variables
extern LLVMContext context;
extern std::unique_ptr<Module> module;
extern std::map<std::string, Value *> named_values;

#endif

#ifndef AST_H
#define AST_H

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
#include <map>
#include <memory>
#include <string>
#include <vector>

using namespace llvm;

// ExprAST - base class for all expression nodes (AST)
class ExprAST {
    public:
        virtual ~ExprAST() {}
        virtual Value* codegen() = 0;
};

// NumberExprAST - Expression class for numeric values
class NumberExprAST : public ExprAST {
    private:
        double val;
    public:
        NumberExprAST(double val) : val(val) {}
        virtual Value* codegen() override;
};

// VariableExprAST - Expression class for variables
class VariableExprAST : public ExprAST {
    private:
        std::string name;
    public:
        VariableExprAST(const std::string &name) : name(name) {}
        virtual Value* codegen() override;
};

// BinaryExprAST - Expression class for binary operators
class BinaryOpExprAST : public ExprAST {
    private:
        char op;
        std::unique_ptr<ExprAST> LHS, RHS;
    public:
        BinaryOpExprAST(char op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
            : op(op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
        virtual Value* codegen() override;
};

// CallExprAST - Expression class for function calls
class CallExprAST : public ExprAST {
    private:
        std::string func_name;
        std::vector<std::unique_ptr<ExprAST>> args;
    public:
        CallExprAST(const std::string &func_name, std::vector<std::unique_ptr<ExprAST>> args)
            : func_name(func_name), args(std::move(args)) {}
        virtual Value* codegen() override;
};

// PrototypeAST - This class represents the "prototype" for a function,
// which captures its name and argument names
class PrototypeAST {
    private:
        std::string name;
        std::vector<std::string> args;
    public:
        PrototypeAST(const std::string &name, std::vector<std::string> args)
            : name(name), args(std::move(args)) {}
        const std::string &get_name() const { return name; }
        Function* codegen();
};

/// FunctionAST - This class represents a function definition itself
class FunctionAST {
    private:
        std::unique_ptr<PrototypeAST> proto;
        std::unique_ptr<ExprAST> body;
    public:
        FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body)
            : proto(std::move(proto)), body(std::move(body)) {}
        Function* codegen();
};

#endif

#include "codegen.h"

Value* log_error_v(const char *str) {
    log_error(str);
    return nullptr;
}

Value* NumberExprAST::codegen() {
    return ConstantFP::get(context, APFloat(val));
}

Value* VariableExprAST::codegen() {
    Value* v = named_values[name];
    if (!v) {
        log_error_v("Unknown variable name");
    }
    return v;
}

Value* BinaryOpExprAST::codegen() {
    Value* L = LHS->codegen();
    Value* R = RHS->codegen();
    if (!L || !R) {
        return nullptr;
    }

    switch(op) {
    case '+':
        return Builder.CreateFAdd(L, R, "addtmp");
    case '-':
        return Builder.CreateFSub(L, R, "subtmp");
    case '*':
        return Builder.CreateFMul(L, R, "multmp");
    case '<':
        L = Builder.CreateFCmpULT(L, R, "cmptmp");
        // Convert bool 0/1 to double 0.0 or 1.0
        return Builder.CreateUIToFP(L, Type::getDoubleTy(context), "booltmp");
    default:
        return log_error_v("invalid binary operator");
    }
}

Value* CallExprAST::codegen() {
    // Look up the name in the global module table
    Function* caller_func = module->getFunction(func_name);
    if (!caller_func) {
        return log_error_v("Unknown function referenced");
    }

    // If argument mismatch error
    if (caller_func->arg_size() != args.size()) {
        return log_error_v("Incorrect # arguments passed");
    }

    std::vector<Value *> args_v;
    for (unsigned i = 0, e = args.size(); i != e; i++) {
        args_v.push_back(args[i]->codegen());
        if (!args_v.back()) {
            return nullptr;
        }
    }
    return Builder.CreateCall(caller_func, args_v, "calltmp");
}

Function* PrototypeAST::codegen() {
    // Make the function type: double(double, double) etc.
    std:: vector<Type*> doubles(args.size(), Type::getDoubleTy(context));
    FunctionType* ft = FunctionType::get(Type::getDoubleTy(context), doubles, false);
    Function* f = Function::Create(ft, Function::ExternalLinkage, name, module.get());

    // Set names for all arguments
    unsigned idx = 0;
    for (auto &arg : f->args()) {
        arg.setName(args[idx++]);
    }
    return f; 
}   

Function* FunctionAST::codegen() {
    // First, check for an existing function from a previous 'extern' declaration
    Function* func = module->getFunction(proto->get_name());
    
    if (!func) {
        func = proto->codegen();
    }
    if (!func) {
        return nullptr;
    }
    if (!func->empty()) {
        return (Function*)log_error_v("Function cannot be redefined.");
    }

    // Create a new basic block to start insertion into
    BasicBlock* bb = BasicBlock::Create(context, "entry", func);
    Builder.SetInsertPoint(bb);
    
    // Record the function arguments in the named_values map.
    named_values.clear();
    for (auto &arg : func->args()) {
        named_values[arg.getName()] = &arg;
    }

    if (Value* ret_val = body->codegen()) {
        // Funish off teh function
        Builder.CreateRet(ret_val);

        // Validate the generated code, checking for consistency
        verifyFunction(*func);

        return func;
    }
    
    // Error reading body, remove function.
    func->eraseFromParent();
    return nullptr;
}

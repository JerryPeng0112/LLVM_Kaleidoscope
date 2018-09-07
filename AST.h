namespace {
// ExprAST - base class for all expression nodes (AST)
class ExprAST {
    public:
        virtual ~ExprAST() {}
};

// NumberExprAST - Expression class for numeric values
class NumberExprAST : public ExprAST {
    private:
        double val;
    public:
        NumberExprAST(double val) : val(val) {}
        double get_val() { return val; }
};

// VariableExprAST - Expression class for variables
class VariableExprAST : public ExprAST {
    private:
        std::string name;
    public:
        VariableExprAST(const std::string &name) : name(name) {}
};

// BinaryExprAST - Expression class for binary operators
class BinaryOpExprAST : public ExprAST {
    private:
        char op;
        std::unique_ptr<ExprAST> LHS, RHS;
    public:
        BinaryOpExprAST(char op, std::unique_ptr<ExprAST> LHS, std::unique_ptr<ExprAST> RHS)
            : op(op), LHS(std::move(LHS)), RHS(std::move(RHS)) {}
        char get_op() { return op; }
};

// CallExprAST - Expression class for function calls
class CallExprAST : public ExprAST {
    private:
        std::string func_name;
        std::vector<std::unique_ptr<ExprAST>> args;
    public:
        CallExprAST(const std::string &func_name, std::vector<std::unique_ptr<ExprAST>> args)
            : func_name(func_name), args(std::move(args)) {}
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
};

/// FunctionAST - This class represents a function definition itself
class FunctionAST {
    private:
        std::unique_ptr<PrototypeAST> Proto;
        std::unique_ptr<ExprAST> Body;
    public:
        FunctionAST(std::unique_ptr<PrototypeAST> Proto, std::unique_ptr<ExprAST> Body)
            : Proto(std::move(Proto)), Body(std::move(Body)) {}
};
} // end anonymous namespace

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
};

// CallExprAST - Expression class for function calls
class CallExprAST : public ExprAST {
    private:
        std::string func_name;
        std::vector<std::unique_ptr<ExprAST>> args;
    public:
        CallExprAST(const std::string &func_name, std::vector<std::unique_ptr<ExprAST>> args)
            : func_name(func_name), args(std::move(args)) {}
}

// PrototypeAST - This class represents the "prototype" for a function,
// which captures its name and argument names
class PrototypeAST {
    private:
        std::string name;
        std::vector<std::string> args;
    public:
        PrototypeAST(const std::string &name, std::vector<std::string> args)
            : name(name), args(std::move(args)) {}
};

// FunctionAST - This class represents a function definition itself
class FunctionAST {
    private:
        std::unique_ptr<PrototypeAST> proto;
        std::unique_ptr<ExprAST> body;
    public:
        FunctionAST(std::unique_ptr<PrototypeAST> proto, std::unique_ptr<ExprAST> body)
            : proto(std::move(proto)), body(std::move(body)) {}
};

// cur_tok/get_next_tok() - provide a simple token buffer
static int cur_tok;
static int get_next_tok() {
    return cur_tok = gettok();
}

// LogError - helper function for error handling.
std::unique_ptr<ExprAST> log_error(const char *str) {
    fprintf(stderr, "Log error: %s\n", str);
    return nullptr;
}

std::unique_ptr<PrototypeAST> log_error_p(const char *str) {
    LogError(Str);
    return nullptr;
}

// NumberExpr ::= number
static std::unique_ptr<ExprAST> parse_number_expr() {
    auto result = llvm::make_unique<NumberExprAst>(num_val);
    get_next_tok();
    return std::move(result);
}

// ParenExpr ::= ( Expr )
static std::unique_ptr<ExprAST> parse_paren_expr() {
    // eat '('
    get_next_tok();
    auto V = parse_expression();
    if (!V)
        return nullptr;
    if (cur_tok != ')')
        return log_error("expected ')'");
    //eat ')'
    get_next_tok();
    return V;
}

// IdentifierExpr ::= identifier
//                ::= identifier ( expression* )
// The second production for function calls
static std::unique_ptr<ExprAST> parse_identifier_expr() {
    std::string id_name = identifier_str;
    //eat identifier.
    get_next_tok();
    
    // For simple variable reference
    if (cur_tok != '(') {
        return llvm::make_unique<VariableExprAST>(id_name);
    }

    // For function calls
    get_next_tok();
    std::vector<std::unique_ptr<ExprAST>> args;
    if (cur_tok != ')') {
        while (1) {
            if (auto arg = parse_expression()) {
                args.push_back(std::move(args));
            }
            else {
                return nullptr;
            }

            if (cur_tok == ')') {
                break;
            }

            if (cur_tok != ',')
                return log_error("Expected ')' or ',' in argument list");
            get_next_tok();
        }
    }

    // eat ')'
    get_next_tok();
    return llvm::make_unique<CallExprAST>(id_name, std::move(args));
}

// Primary ::= IdentifierExpr
//         ::= NumberExpr
//         ::= ParenExpr
static std::unique_ptr<ExprAST> parse_primary() {
    switch (cur_tok) {
    default:
        return log_error("unknow token when expecting an expression");
    case tok_identifier:
        return parse_identifier_expr();
    case tok_number:
        return parse_number_expr();
    case '(':
        return parse_paren_expr();
    }
}



#include "parser.h"

std::unique_ptr<ExprAST> log_error_e(const char *str) {
    log_error(str);
    return nullptr;
}

std::unique_ptr<PrototypeAST> log_error_p(const char *str) {
    log_error(str);
    return nullptr;
}

// cur_tok/get_next_tok() - provide a simple token buffer
int get_next_tok() {
    return cur_tok = gettok();
}

// NumberExpr ::= number
std::unique_ptr<ExprAST> parse_number_expr() {
    auto result = llvm::make_unique<NumberExprAST>(num_val);
    get_next_tok();
    return std::move(result);
}

// ParenExpr ::= '(' Expr ')'
std::unique_ptr<ExprAST> parse_paren_expr() {
    // eat '('
    get_next_tok();
    auto V = parse_expression();
    if (!V)
        return nullptr;
    if (cur_tok != ')')
        return log_error_e("expected ')'");
    //eat ')'
    get_next_tok();
    return V;
}

// IdentifierExpr ::= identifier
//                ::= identifier '(' expression* ')'
// The second production for function calls
std::unique_ptr<ExprAST> parse_identifier_expr() {
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
                args.push_back(std::move(arg));
            }
            else {
                return nullptr;
            }

            if (cur_tok == ')') {
                break;
            }

            if (cur_tok != ',')
                return log_error_e("Expected ')' or ',' in argument list");
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
std::unique_ptr<ExprAST> parse_primary() {
    switch (cur_tok) {
    default:
        return log_error_e("unknown token when expecting an expression");
    case tok_identifier:
        return parse_identifier_expr();
    case tok_number:
        return parse_number_expr();
    case '(':
        return parse_paren_expr();
    }
}

// get_tok_precedence - Get the precedence of binary operator token
int get_tok_precedence() {
    if (!isascii(cur_tok))
        return -1;
    // Make sure it's a declared binary operator
    int tok_prec = bin_op_precedence[cur_tok];
    if (tok_prec <= 0) return -1;
    return tok_prec;
}

// bin_op_rhs ::= ('+' primary)*
std::unique_ptr<ExprAST> parse_bin_op_rhs(int expr_prec, std::unique_ptr<ExprAST> LHS) {
    // If this is a binary operator, find its precedence
    while (1) {
        int tok_prec = get_tok_precedence();

        // If this is a binary operator that holds at least as tightly as the current
        // binary operator, consume it, otherwise we are done.
        if (tok_prec < expr_prec) {
            return LHS;
        }
        int bin_op = cur_tok;
        get_next_tok();

        // Parse the primary expression after the binary operator
        auto RHS = parse_primary();
        if (!RHS)
            return nullptr;

        // If binary operator binds less tightly with RHS than the operator after RHS, let
        // the pending operator take RHS as its LHS
        int next_prec = get_tok_precedence();
        if (tok_prec < next_prec) {
            RHS = parse_bin_op_rhs(tok_prec + 1, std::move(RHS));
            if (!RHS) {
                return nullptr;
            }
        }
        // Merge LHS/RHS
        LHS = llvm::make_unique<BinaryOpExprAST>(bin_op, std::move(LHS), std::move(RHS));
    }
}

// expression ::= primary bin_op_rhs
std:: unique_ptr<ExprAST> parse_expression() {
    auto LHS = parse_primary();
    if (!LHS) {
        return nullptr;
    }
    return parse_bin_op_rhs(0, std::move(LHS));
}

// prototype ::= id '(' id* ')'
std::unique_ptr<PrototypeAST> parse_prototype() {
    if (cur_tok != tok_identifier) {
        return log_error_p("Expected function name in prototype");
    }
    
    std::string  fn_name = identifier_str;
    get_next_tok();

    if (cur_tok != '(') {
        return log_error_p("Expected '(' in prototype");
    }
    
    // Read the list of argument names
    std::vector<std::string> arg_names;
    while (get_next_tok() == tok_identifier) {
        arg_names.push_back(identifier_str);
    }
    if (cur_tok != ')') {
        return log_error_p("Expected ')' in prototype");
    }

    // success
    get_next_tok();
    return llvm::make_unique<PrototypeAST>(fn_name, std::move(arg_names));
}

// definition ::= 'def' prototype expression
std::unique_ptr<FunctionAST> parse_definition() {
    // eat def
    get_next_tok();
    auto proto = parse_prototype();
    if (!proto) {
        return nullptr;
    }

    if (auto expr = parse_expression()) {
        return llvm::make_unique<FunctionAST>(std::move(proto), std::move(expr));
    }
    return nullptr;
}

// external ::= 'extern' prototype
std::unique_ptr<PrototypeAST> parse_extern() {
    // eat extern
    get_next_tok();
    return parse_prototype();
}

// top_level_expr ::= expression
std::unique_ptr<FunctionAST> parse_top_level_expr() {
    if (auto expr = parse_expression()) {
        // Make an anonymous proto
        auto proto = llvm::make_unique<PrototypeAST>("", std::vector<std::string>());
        return llvm::make_unique<FunctionAST>(std::move(proto), std::move(expr));
    }
    return nullptr;
}

// top ::= definition | external | expression | ';'
void main_loop() {
    while(1) {
        fprintf(stderr, "ready> ");
        switch (cur_tok) {
        case tok_eof:
            return;
        case ';': // ignore top-level semicolons
            get_next_tok();
            break;
        case tok_def:
            handle_definition();
            break;
        case tok_extern:
            handle_extern();
            break;
        default:
            handle_top_level_expression();
            break;
        }
    }
}

void handle_definition() {
    if (auto fn_AST = parse_definition()) {
        if (auto *fn_IR = fn_AST->codegen()) {
            fprintf(stderr, "Read function definition.");
            fn_IR->print(errs());
            fprintf(stderr, "\n");
        }
    } else {
        // Skip token for error recovery
        get_next_tok();
    }
}

void handle_extern() {
    if (auto proto_AST = parse_extern()) {
        if (auto *fn_IR = proto_AST->codegen()) {
            fprintf(stderr, "Read extern");
            fn_IR->print(errs());
            fprintf(stderr, "\n");
        }
    } else {
        // Skip token for error recovery
        get_next_tok();
    }
}

void handle_top_level_expression() {
    // Evaluate a top-level expression into an anonymous function
    if (auto fn_AST = parse_top_level_expr()) {
        if (auto *fn_IR = fn_AST->codegen()) { 
            fprintf(stderr, "Read top-level expression");
            fn_IR->print(errs());
            fprintf(stderr, "\n");
        }
    } else {
        // Skip token for error recovery.
        get_next_tok();
    }
}

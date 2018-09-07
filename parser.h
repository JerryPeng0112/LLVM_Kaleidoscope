#include "lexer.h"
#include "AST.h"
#include "llvm/ADT/STLExtras.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

static int get_next_tok();
std::unique_ptr<ExprAST> log_error(const char *str);
std::unique_ptr<PrototypeAST> log_error_p(const char *str);
static std::unique_ptr<ExprAST> parse_number_expr();
static std::unique_ptr<ExprAST> parse_paren_expr();
static std::unique_ptr<ExprAST> parse_identifier_expr();
static std::unique_ptr<ExprAST> parse_primary();
static int get_tok_precedence();
static std:: unique_ptr<ExprAST> parse_expression();
static std::unique_ptr<ExprAST> parse_bin_op_rhs(int expr_prec, std::unique_ptr<ExprAST> LHS);
static std::unique_ptr<PrototypeAST> parse_prototype();
static std::unique_ptr<FunctionAST> parse_definition();
static std::unique_ptr<PrototypeAST> parse_extern();
static std::unique_ptr<FunctionAST> parse_top_level_expr();
static void main_loop();
static void handle_definition();
static void handle_extern();
static void handle_top_level_expression();

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
    log_error(str);
    return nullptr;
}

// NumberExpr ::= number
static std::unique_ptr<ExprAST> parse_number_expr() {
    auto result = llvm::make_unique<NumberExprAST>(num_val);
    get_next_tok();
    return std::move(result);
}

// ParenExpr ::= '(' Expr ')'
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
//                ::= identifier '(' expression* ')'
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
                args.push_back(std::move(arg));
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

// bin_op_precedence - This holds the precedence for each binary operator that is defined.
static std::map<char, int> bin_op_precedence;

// get_tok_precedence - Get the precedence of binary operator token
static int get_tok_precedence() {
    if (!isascii(cur_tok))
        return -1;
    // Make sure it's a declared binary operator
    int tok_prec = bin_op_precedence[cur_tok];
    if (tok_prec <= 0) return -1;
    return tok_prec;
}

// expression ::= primary bin_op_rhs
static std:: unique_ptr<ExprAST> parse_expression() {
    auto LHS = parse_primary();
    if (!LHS) {
        return nullptr;
    }
    return parse_bin_op_rhs(0, std::move(LHS));
}

// bin_op_rhs ::= ('+' primary)*
static std::unique_ptr<ExprAST> parse_bin_op_rhs(int expr_prec, std::unique_ptr<ExprAST> LHS) {
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

// prototype ::= id '(' id* ')'
static std::unique_ptr<PrototypeAST> parse_prototype() {
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
static std::unique_ptr<FunctionAST> parse_definition() {
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
static std::unique_ptr<PrototypeAST> parse_extern() {
    // eat extern
    get_next_tok();
    return parse_prototype();
}

// top_level_expr ::= expression
static std::unique_ptr<FunctionAST> parse_top_level_expr() {
    if (auto expr = parse_expression()) {
        // Make an anonymous proto
        auto proto = llvm::make_unique<PrototypeAST>("", std::vector<std::string>());
        return llvm::make_unique<FunctionAST>(std::move(proto), std::move(expr));
    }
    return nullptr;
}

// top ::= definition | external | expression | ';'
static void main_loop() {
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

static void handle_definition() {
    if (parse_definition()) {
        fprintf(stderr, "Parsed a function definition.\n");
    } else {
        // Skip token for error recovery
        get_next_tok();
    }
}

static void handle_extern() {
    if (parse_extern()) {
        fprintf(stderr, "Parsed an extern\n");
    } else {
        // Skip token for error recovery
        get_next_tok();
    }
}

static void handle_top_level_expression() {
    // Evaluate a top-level expression into an anonymous function
    if (parse_top_level_expr()) {
        fprintf(stderr, "Parsed a top-level expr\n");
    } else {
        // Skip token for error recovery.
        get_next_tok();
    }
}

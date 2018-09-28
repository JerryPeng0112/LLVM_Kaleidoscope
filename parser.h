#ifndef PARSER_H
#define PARSER_H

#include "lexer.h"
#include "AST.h"
#include "utils.h"

/*
    Language Production
    top ::= definition | external | expression | ';'

    definition ::= 'def' prototype expression
    top_level_expr ::= expression
    external ::= 'extern' prototype

    prototype ::= id '(' id* ')'
    expression ::= primary bin_op_rhs

    Primary ::= IdentifierExpr
            ::= NumberExpr
            ::= ParenExpr
    bin_op_rhs ::= ('+' primary)*

    NumberExpr ::= number
    ParenExpr ::= '(' Expr ')'
    IdentifierExpr ::= identifier
                   ::= identifier '(' expression* ')'
    The second production for function calls
*/

int get_next_tok();
std::unique_ptr<PrototypeAST> log_error_p(const char *str);
std::unique_ptr<ExprAST> parse_number_expr();
std::unique_ptr<ExprAST> parse_paren_expr();
std::unique_ptr<ExprAST> parse_identifier_expr();
std::unique_ptr<ExprAST> parse_primary();
int get_tok_precedence();
std::unique_ptr<ExprAST> parse_bin_op_rhs(int expr_prec, std::unique_ptr<ExprAST> LHS);
std:: unique_ptr<ExprAST> parse_expression();
std::unique_ptr<PrototypeAST> parse_prototype();
std::unique_ptr<FunctionAST> parse_definition();
std::unique_ptr<PrototypeAST> parse_extern();
std::unique_ptr<FunctionAST> parse_top_level_expr();
void main_loop();
void handle_definition();
void handle_extern();
void handle_top_level_expression();

// token buffer
static int cur_tok;

// bin_op_precedence - This holds the precedence for each binary operator that is defined.
static std::map<char, int> bin_op_precedence;

#endif

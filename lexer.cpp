#include "lexer.h"

// gettok - Return the next token from standard input (lexer)
int gettok() {
    static int last_char = ' ';
    // Skip whitespaces
    while (isspace(last_char)) {
        last_char = getchar();
    }
    // identifier: [a-zA-Z][a-zA-Z0-9]*
    if (isalpha(last_char)) {
        identifier_str = last_char;
        while (isalnum((last_char = getchar())))
            identifier_str += last_char;

        if (identifier_str == "def") {
            return tok_def;
        }
        if (identifier_str == "extern") {
            return tok_extern;
        }
        return tok_identifier;
    }
    // Number: [0-9.]+
    if (isdigit(last_char) || last_char == '.') {
        std::string num_str;
        do {
            num_str += last_char;
            last_char = getchar();
        } while (isdigit(last_char) || last_char == '.');

        num_val = strtod(num_str.c_str(), 0);
        return tok_number;
    }
    // Comments
    if (last_char == '#') {
        do {
            last_char = getchar();
        } while (last_char != EOF && last_char != '\n');
            
        if (last_char != EOF) {
            return gettok();
        }
    }
    // EOF
    if (last_char == EOF) {
        return tok_eof;
    }
    // operators, undefined tokens
    int char_val = last_char;
    last_char = getchar();
    return char_val;
}

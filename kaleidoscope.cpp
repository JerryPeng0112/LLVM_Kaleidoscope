#include "AST.h"
#include "lexer.h"
#include "parser.h"

std::map<char, int> bin_op_precedence;

int main() {

    bin_op_precedence['<'] = 10; 
    bin_op_precedence['+'] = 20;       
    bin_op_precedence['-'] = 30;
    bin_op_precedence['*'] = 40;
    
    // Prime the first token.
    fprintf(stderr, "ready> ");
    get_next_tok();

    // Make the module, which holds all the code.
    module = llvm::make_unique<Module>("my cool jit", context);
   
    // Run themain "interpreter loop" now.
    main_loop();
   
    // Print out all of the generated code.
    module->print(errs(), nullptr);

    return 0;
}

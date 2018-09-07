#include "parser.h"
#include "llvm/ADT/STLExtras.h"
#include <algorithm>
#include <cctype>
#include <cstdio>
#include <cstdlib>
#include <map>
#include <memory>
#include <string>
#include <vector>

int main() {
     bin_op_precedence['<'] = 10; 
     bin_op_precedence['+'] = 20;       
     bin_op_precedence['-'] = 30;
     bin_op_precedence['*'] = 40;
     
     // Prime the first token
     fprintf(stderr, "ready> ");
     get_next_tok();

     // Run themain "interpreter loop" now
     main_loop();
     
     return 0;
}

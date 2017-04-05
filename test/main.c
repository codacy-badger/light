#include <stdio.h>

#include "../lexer.h"

int main () {
    Lexer* lexer = lexer_create("test/light.grammar");

    lexer_destroy(lexer);
    return 0;
}

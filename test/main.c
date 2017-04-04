#include <stdio.h>

#include "../lexer.h"

int main () {
    Lexer* lexer = lexer_create("test/light.grammar");
    printf("Lexer: %d\n", lexer);

    lexer_destroy(lexer);
    return 0;
}

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "lexer.h"
#include "error.h"

/* display colors for tokens */
#define C_RESET  "\x1b[0m"
#define C_KEY    "\x1b[35m" /* magenta */
#define C_ID     "\x1b[36m" /* cyan */
#define C_CONST  "\x1b[32m" /* green */
#define C_OP     "\x1b[31m" /* red */
#define C_SPEC   "\x1b[33m" /* yellow */

const char* tokenTypeName(TokenType t) {
    switch (t) {
        case TT_KEYWORD: return "keyword";
        case TT_IDENTIFIER: return "identifier";
        case TT_CONSTANT: return "constant";
        case TT_OPERATOR: return "operator";
        case TT_SPECIAL_CHARACTER: return "special character";
        case TT_UNKNOWN: return "unknown";
        case TT_END_OF_FILE: return "EOF";
        default: return "invalid";
    }
}
const char* tokenColor(TokenType t) {
    switch (t) {
        case TT_KEYWORD: return C_KEY;
        case TT_IDENTIFIER: return C_ID;
        case TT_CONSTANT: return C_CONST;
        case TT_OPERATOR: return C_OP;
        case TT_SPECIAL_CHARACTER: return C_SPEC;
        default: return C_RESET;
    }
}

int main(int argc, char *argv[]) {
    char filename[512];

    if (argc >= 2) {
        strncpy(filename, argv[1], sizeof(filename) - 1);
        filename[sizeof(filename)-1] = '\0';
    } else {
        printf("Enter source file name: ");
        if (scanf("%511s", filename) != 1) {
            fprintf(stderr, "Failed to read filename\n");
            return 2;
        }
    }

    if (!initializeLexer(filename)) {
        fprintf(stderr, "Cannot open input file: %s\n", filename);
        return 1;
    }

    printf("\n| %-12s | %-20s | %-18s |\n", "Position", "Lexeme", "Type");
    printf("|--------------|----------------------|--------------------|\n");

    while (1) {
        Token t = getNextToken();
        if (t.type == TT_END_OF_FILE) break;

        const char *col = tokenColor(t.type);
        printf("| l:%-3d c:%-3d  | %s%-20s%s | %s%-18s%s |\n",
               t.line, t.column,
               col, t.lexeme, C_RESET,
               col, tokenTypeName(t.type), C_RESET);
    }

    closeLexer();
    return 0;
}
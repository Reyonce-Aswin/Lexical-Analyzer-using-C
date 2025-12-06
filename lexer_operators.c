#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "error.h"

/* parse operator or special single/multi-char tokens.
   ch is the first character already consumed; startLine/startColumn denote its position.
*/
Token parseOperatorOrSpecial(int ch_input, int startLine, int startColumn) {
    Token token;
    token.lexeme[0] = '\0';
    token.line = startLine;
    token.column = startColumn;
    token.type = TT_UNKNOWN;

    int ch = ch_input;
    int i = 0;

    /* start lexeme */
    if (i < MAX_TOKEN_SIZE - 1) token.lexeme[i++] = (char)ch;

    /* check multi-character operators using peek */
    int next = peek();
    if (next != EOF) {
        char two[3] = { (char)ch, (char)next, '\0' };
        if (isOperator(two)) {
            /* consume next */
            advance();
            if (i < MAX_TOKEN_SIZE - 1) token.lexeme[i++] = (char)next;
        }
    }

    token.lexeme[i] = '\0';

    /* categorize */
    categorizeToken(&token);

    /* if token is unknown (rare), report error */
    if (token.type == TT_UNKNOWN) {
        reportLexerError(token.line, token.column, "Unrecognized token", token.lexeme);
    }

    return token;
}
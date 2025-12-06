#ifndef ERROR_H
#define ERROR_H

#include <stdio.h>

#define MAX_ERROR_MESSAGE 512

/* Color codes, prefixed to avoid namespace collisions */
#define ERR_RESET   "\x1b[0m"
#define ERR_RED     "\x1b[31m"
#define ERR_BOLD    "\x1b[1m"

void reportLexerError(int line, int column, const char *message, const char *lexeme);

#endif /* ERROR_H */
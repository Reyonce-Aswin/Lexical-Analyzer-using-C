#ifndef LEXER_H
#define LEXER_H

#include <stdio.h>

#define MAX_TOKEN_SIZE 256
#define MAX_KEYWORDS 24
#define RETRACT_STACK_CAP 65536

typedef enum {
    TT_KEYWORD,
    TT_IDENTIFIER,
    TT_CONSTANT,
    TT_OPERATOR,
    TT_SPECIAL_CHARACTER,
    TT_UNKNOWN,
    TT_END_OF_FILE
} TokenType;

typedef struct {
    TokenType type;
    char lexeme[MAX_TOKEN_SIZE];
    int line;    // starting line (1-based)
    int column;  // starting column (1-based)
} Token;

/* Public lexer API */
int initializeLexer(const char *filename); /* returns 1 on success, 0 on failure */
void closeLexer(void);

Token getNextToken(void); /* main entry to retrieve next token */

/* Position / I/O helpers exposed for parser modules */
extern FILE *sourceFile;
int advance(void);           /* get next character and advance position */
int peek(void);              /* peek next character without consuming */
void retract(int ch);        /* push back a single character and adjust line/column */
void getCurrentPosition(int *line, int *column); /* read current position */

/* Parsing entry points (implemented in parse modules) */
Token parseIdentifierOrKeyword(int ch, int startLine, int startColumn);
Token parseNumber(int ch, int startLine, int startColumn);
Token parseStringLiteral(int ch, int startLine, int startColumn);
Token parseOperatorOrSpecial(int ch, int startLine, int startColumn);

/* Helpers (lexer_helpers.c) */
void categorizeToken(Token *token);
int isKeyword(const char *str);
int isOperator(const char *str);
int isSpecialCharacterChar(char ch);
int isConstant(const char *str);
int isIdentifier(const char *str);

#endif /* LEXER_H */
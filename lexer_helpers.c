#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "error.h"

/* keywords */
static const char* keywords[MAX_KEYWORDS] = {
    "int", "float", "return", "if", "else", "while", "for", "do", "break", "continue",
    "char", "double", "void", "switch", "case", "default", "const", "static", "sizeof", "struct",
    "ifndef", "define", "include", "endif"
};

/* single char operators / specials */
static const char* singleOperators = "+-*/%=!<>|&^~.:";
static const char* specialCharacters = ",;{}()[]#";

/* multi operators */
static const char* multiOperators[] = {
    "==", "!=", "<=", ">=", "++", "--", "&&", "||",
    "+=", "-=", "*=", "/=", "%=", "<<", ">>", "->", "::"
};
static const int NUM_MULTI_OPS = sizeof(multiOperators) / sizeof(multiOperators[0]);

int isMultiOperator(const char* s) {
    if (!s) return 0;
    for (int i = 0; i < NUM_MULTI_OPS; ++i) {
        if (strcmp(s, multiOperators[i]) == 0) return 1;
    }
    return 0;
}

int isSingleOperatorChar(char ch) {
    return (strchr(singleOperators, ch) != NULL);
}

int isSpecialCharacterChar(char ch) {
    return (strchr(specialCharacters, ch) != NULL);
}

/* categorize token lexeme: set token->type */
void categorizeToken(Token* token) {
    if (!token || token->lexeme[0] == '\0') {
        token->type = TT_UNKNOWN;
        return;
    }
    if (isKeyword(token->lexeme)) {
        token->type = TT_KEYWORD;
        return;
    }
    if (isOperator(token->lexeme)) {
        token->type = TT_OPERATOR;
        return;
    }
    if (strlen(token->lexeme) == 1 && isSpecialCharacterChar(token->lexeme[0])) {
        token->type = TT_SPECIAL_CHARACTER;
        return;
    }
    if (token->lexeme[0] == '"' || isConstant(token->lexeme)) {
        token->type = TT_CONSTANT;
        return;
    }
    if (isIdentifier(token->lexeme)) {
        token->type = TT_IDENTIFIER;
        return;
    }
    token->type = TT_UNKNOWN;
}

/* keyword test */
int isKeyword(const char* str) {
    if (!str) return 0;
    for (int i = 0; i < MAX_KEYWORDS; ++i) {
        if (strcmp(str, keywords[i]) == 0) return 1;
    }
    return 0;
}

/* operator test (1 or 2 char) */
int isOperator(const char* str) {
    if (!str) return 0;
    size_t len = strlen(str);
    if (len == 1) {
        return isSingleOperatorChar(str[0]);
    } else if (len == 2) {
        return isMultiOperator(str);
    }
    return 0;
}

/* numeric constant test: integers or single-dot decimals */
int isConstant(const char* str) {
    if (!str || *str == '\0') return 0;
    int i = 0;
    int dotCount = 0;
    int digits = 0;
    if (str[i] == '.') {
        dotCount++;
        i++;
        if (!isdigit((unsigned char)str[i])) return 0;
    }
    for (; str[i] != '\0'; ++i) {
        if (isdigit((unsigned char)str[i])) {
            digits = 1;
            continue;
        }
        if (str[i] == '.') {
            dotCount++;
            if (dotCount > 1) return 0;
            continue;
        }
        return 0;
    }
    return digits;
}

/* identifier test */
int isIdentifier(const char* str) {
    if (!str || *str == '\0') return 0;
    if (!(isalpha((unsigned char)str[0]) || str[0] == '_')) return 0;
    for (int i = 1; str[i] != '\0'; ++i) {
        if (!isalnum((unsigned char)str[i]) && str[i] != '_') return 0;
    }
    return 1;
}
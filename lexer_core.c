#define _POSIX_C_SOURCE 200809L
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "error.h"

/* single definition of global file pointer */
FILE *sourceFile = NULL;

/* position tracking */
static int currentLine = 1;
static int currentColumn = 1;

/* stack to restore previous column on newline retract */
static int retractColumnStack[RETRACT_STACK_CAP];
static int retractStackTop = 0;

/* open/close */
int initializeLexer(const char *filename) {
    sourceFile = fopen(filename, "r");
    if (!sourceFile) {
        return 0;
    }
    currentLine = 1;
    currentColumn = 1;
    retractStackTop = 0;
    return 1;
}
void closeLexer(void) {
    if (sourceFile) {
        fclose(sourceFile);
        sourceFile = NULL;
    }
}

/* advance: consume one char and update currentLine/currentColumn */
int advance(void) {
    if (!sourceFile) return EOF;
    int ch = fgetc(sourceFile);
    if (ch == EOF) return EOF;

    if (ch == '\n') {
        /* push previous column onto stack for possible retract */
        if (retractStackTop < RETRACT_STACK_CAP) {
            retractColumnStack[retractStackTop++] = currentColumn;
        }
        currentLine++;
        currentColumn = 1;
    } else {
        currentColumn++;
    }
    return ch;
}

/* peek without consuming: preserve file pos and positions */
int peek(void) {
    if (!sourceFile) return EOF;
    fpos_t pos;
    if (fgetpos(sourceFile, &pos) != 0) return EOF;
    int savedLine = currentLine;
    int savedColumn = currentColumn;
    int ch = fgetc(sourceFile);
    /* restore */
    fsetpos(sourceFile, &pos);
    currentLine = savedLine;
    currentColumn = savedColumn;
    return ch;
}

/* retract a single character ch: undo the last advance of that char */
void retract(int ch) {
    if (!sourceFile) return;
    if (ch == EOF) return;

    /* push the char back to stream */
    if (ungetc(ch, sourceFile) == EOF) {
        /* if ungetc fails, we try to ignore; position may be inconsistent */
        return;
    }

    /* fix columns and lines */
    if (ch == '\n') {
        /* we must restore previous column and decrement line */
        if (retractStackTop > 0) {
            --retractStackTop;
            currentColumn = retractColumnStack[retractStackTop];
        } else {
            /* fallback: set to 1 */
            currentColumn = 1;
        }
        if (currentLine > 1) currentLine--;
    } else {
        if (currentColumn > 1) currentColumn--;
        else currentColumn = 1;
    }
}

/* retrieve current position (useful for token creation) */
void getCurrentPosition(int *line, int *column) {
    *line = currentLine;
    *column = currentColumn;
}

/* Main token loop: delegates to parsers, handles whitespace and comments */
Token getNextToken(void) {
    Token token;
    token.type = TT_UNKNOWN;
    token.lexeme[0] = '\0';
    token.line = 0;
    token.column = 0;

    if (!sourceFile) return token;

    int ch;
    for (;;) {
        /* skip whitespace */
        do {
            ch = advance();
        } while (ch != EOF && (ch == ' ' || ch == '\t' || ch == '\r' || ch == '\n' || ch == '\f' || ch == '\v'));

        if (ch == EOF) {
            token.type = TT_END_OF_FILE;
            token.lexeme[0] = '\0';
            return token;
        }

        /* compute token start position: position BEFORE the char we already consumed
           Because advance() increments column after consuming, the start column is currentColumn - 1
           However we want startLine/startColumn to point to the character we just consumed.
        */
        int startLine, startColumn;
        getCurrentPosition(&startLine, &startColumn);
        /* advance() has already incremented column; the character started at startColumn - 1 unless newline */
        if (ch != '\n') {
            startColumn = startColumn - 1;
            if (startColumn < 1) startColumn = 1;
        } else {
            /* newline token start is previous line's last column — we saved previous column on stack */
            if (retractStackTop > 0) {
                startColumn = retractColumnStack[retractStackTop - 1];
            } else {
                startColumn = 1;
            }
        }

        /* dispatch */
        if (ch == '"' ) {
            return parseStringLiteral(ch, startLine, startColumn);
        } else if (isalpha((unsigned char)ch) || ch == '_') {
            return parseIdentifierOrKeyword(ch, startLine, startColumn);
        } else if (isdigit((unsigned char)ch) || ch == '.') {
            /* if '.' ensure next is digit for number; otherwise treat '.' as operator */
            if (ch == '.') {
                int next = peek();
                if (next == EOF || !isdigit((unsigned char)next)) {
                    /* treat '.' as operator */
                    return parseOperatorOrSpecial(ch, startLine, startColumn);
                } else {
                    return parseNumber(ch, startLine, startColumn);
                }
            } else {
                return parseNumber(ch, startLine, startColumn);
            }
        } else {
            /* operator, special or comment */
            /* handle comments here non-recursively */
            if (ch == '/') {
                int next = peek();
                if (next == '/') {
                    /* single-line comment: consume until newline or EOF then continue outer loop */
                    while ((ch = advance()) != EOF && ch != '\n') { /* skip */ }
                    /* continue scanning tokens */
                    continue;
                } else if (next == '*') {
                    /* multi-line comment: consume until '*''/' or EOF (non-recursive) */
                    /* consume the '*' */
                    advance(); /* consume '*' */
                    int prev = 0;
                    int ended = 0;
                    while ((ch = advance()) != EOF) {
                        if (prev == '*' && ch == '/') {
                            ended = 1;
                            break;
                        }
                        prev = ch;
                    }
                    if (!ended) {
                        int errLine, errCol;
                        getCurrentPosition(&errLine, &errCol);
                        /* the error location: use current (where EOF reached) */
                        reportLexerError(errLine, errCol, "Unclosed multi-line comment at EOF.", NULL);
                    }
                    continue;
                } else {
                    return parseOperatorOrSpecial(ch, startLine, startColumn);
                }
            } else {
                return parseOperatorOrSpecial(ch, startLine, startColumn);
            }
        }
    } /* for(;;) */
}
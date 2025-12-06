#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include "lexer.h"
#include "error.h"

/* Identifier or keyword: ch is first char consumed */
Token parseIdentifierOrKeyword(int ch_input, int startLine, int startColumn) {
    Token token;
    token.line = startLine;
    token.column = startColumn;
    token.type = TT_UNKNOWN;
    int i = 0;
    int ch = ch_input;

    while (ch != EOF && (isalnum((unsigned char)ch) || ch == '_')) {
        if (i < MAX_TOKEN_SIZE - 1) token.lexeme[i++] = (char)ch;
        else {
            /* consume remainder silently but report overflow once */
            /* continue consuming until delimiter */
            int overflow = 0;
            while ((ch = peek()) != EOF && (isalnum((unsigned char)ch) || ch == '_')) {
                advance();
                overflow = 1;
            }
            if (overflow) {
                reportLexerError(startLine, startColumn, "Identifier too long (truncated)", NULL);
            }
            break;
        }
        ch = peek();
        if (ch == EOF) break;
        if (!(isalnum((unsigned char)ch) || ch == '_')) break;
        /* consume the next */
        advance();
        ch = ch; /* loop will add it on next iteration after assignment pattern above */
        /* note: we already arranged to keep current ch because we always consume after verifying */
        /* simpler approach: perform advance at top of loop — but keep current pattern for clarity */
    }

    token.lexeme[i] = '\0';

    /* ensure we've consumed the last character that belongs to the identifier
       (the first char was already consumed by caller; we used peek+advance strategy)
    */
    /* categorize */
    categorizeToken(&token);
    return token;
}

/* Number parsing: handles integers and single-decimal floats */
Token parseNumber(int ch_input, int startLine, int startColumn) {
    Token token;
    token.line = startLine;
    token.column = startColumn;
    token.type = TT_UNKNOWN;
    int i = 0;
    int ch = ch_input;
    int dotCount = 0;

    /* If started with '.' we already ensured next is digit before calling */
    while (ch != EOF && (isdigit((unsigned char)ch) || ch == '.')) {
        if (ch == '.') {
            dotCount++;
            if (dotCount > 1) break;
        }
        if (i < MAX_TOKEN_SIZE - 1) token.lexeme[i++] = (char)ch;
        else {
            reportLexerError(startLine, startColumn, "Numeric token too long (truncated)", NULL);
            /* consume rest of digits/dots */
            while ((ch = peek()) != EOF && (isdigit((unsigned char)ch) || ch == '.')) advance();
            break;
        }
        /* look ahead */
        int n = peek();
        if (n == EOF) {
            /* done */
            break;
        }
        if (!(isdigit((unsigned char)n) || n == '.')) {
            /* done */
            break;
        }
        /* consume next to continue loop */
        advance();
        ch = n;
    }

    token.lexeme[i] = '\0';

    categorizeToken(&token);
    return token;
}

/* String literal parsing: ch_input == '"' consumed already */
Token parseStringLiteral(int ch_input, int startLine, int startColumn) {
    Token token;
    token.line = startLine;
    token.column = startColumn;
    token.type = TT_UNKNOWN;

    int i = 0;
    int ch = ch_input;

    /* store opening quote */
    if (i < MAX_TOKEN_SIZE - 1) token.lexeme[i++] = (char)ch;

    int closed = 0;
    while (1) {
        ch = advance();
        if (ch == EOF) {
            token.lexeme[i] = '\0';
            reportLexerError(startLine, startColumn, "Unclosed string literal at EOF", token.lexeme);
            token.type = TT_UNKNOWN;
            return token;
        }
        if (ch == '\n') {
            /* error: newline in string (if not supported) */
            if (i < MAX_TOKEN_SIZE - 1) token.lexeme[i++] = (char)ch;
            token.lexeme[i] = '\0';
            reportLexerError(startLine, startColumn, "String literal not terminated before newline", token.lexeme);
            token.type = TT_UNKNOWN;
            return token;
        }
        /* handle escape: store backslash and next char if available */
        if (ch == '\\') {
            if (i < MAX_TOKEN_SIZE - 1) token.lexeme[i++] = (char)ch;
            int esc = advance();
            if (esc == EOF) {
                token.lexeme[i] = '\0';
                reportLexerError(startLine, startColumn, "Unclosed string literal after escape at EOF", token.lexeme);
                token.type = TT_UNKNOWN;
                return token;
            }
            if (i < MAX_TOKEN_SIZE - 1) token.lexeme[i++] = (char)esc;
            continue;
        }
        if (ch == '"') {
            if (i < MAX_TOKEN_SIZE -1) token.lexeme[i++] = (char)ch;
            closed = 1;
            break;
        }
        if (i < MAX_TOKEN_SIZE - 1) token.lexeme[i++] = (char)ch;
        else {
            /* consume rest but report truncation */
            reportLexerError(startLine, startColumn, "String literal too long (truncated)", NULL);
            /* continue consuming until closing quote or EOF */
            while ((ch = advance()) != EOF && ch != '"') { /* skip */ }
            if (ch == '"') {
                if (i < MAX_TOKEN_SIZE -1) token.lexeme[i++] = (char)ch;
                closed = 1;
            } else {
                reportLexerError(startLine, startColumn, "Unclosed string literal at EOF", token.lexeme);
                token.type = TT_UNKNOWN;
                token.lexeme[i] = '\0';
                return token;
            }
            break;
        }
    }

    token.lexeme[i] = '\0';
    if (closed) token.type = TT_CONSTANT;
    else token.type = TT_UNKNOWN;
    return token;
}
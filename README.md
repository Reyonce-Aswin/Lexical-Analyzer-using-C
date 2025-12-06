# Lexical Analyzer using C

## Overview

This project implements a lightweight and modular **Lexer (Tokenizer)**
written in **ANSI C**.\
The lexer converts raw input text into a sequence of tokens for use in
compilers, interpreters, or static analysis tools.

## Features

- Identifiers, keywords, numbers, strings\
- Operators and punctuation\
- `//` and `/*...*/` comments\
- Line/column tracking\
- Error messages\
- Modular architecture

## Project Structure

    lexer.h
    lexer_core.c
    lexer_constants.c
    lexer_operators.c
    lexer_helpers.c
    error.h / error.c
    main.c

## Example Usage

``` c
#include "lexer.h"
#include <stdio.h>

int main() {
    lexer_init("input.txt");

    Token tok;
    do {
        tok = get_next_token();
        printf("%s (%s) @ %d:%d
",
               tok.lexeme,
               token_type_to_string(tok.type),
               tok.line,
               tok.column);
        free_token(&tok);
    } while (tok.type != TOKEN_EOF);

    lexer_close();
    return 0;
}
```

## Build

    gcc lexer_core.c lexer_helpers.c lexer_constants.c lexer_operators.c error.c main.c -o lexer

## Run
```bash
    ./lexer filename.c
    or 
    ./lexer
```

---

## Author

**Reyonce Aswin T**  
*Student ID: 25021_181*  
Embedded Systems Student, Emertxe Information Technologies, Bangalore

---

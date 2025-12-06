#include "error.h"

// Function to print a formatted error message to stderr
void reportLexerError(int line, int column, const char *message, const char *lexeme)
{
    // Print the error in bold red to stand out
    fprintf(stderr, 
            ERR_BOLD ERR_RED "Error" ERR_RESET 
            ": (%d, %d) " 
            "%s" 
            "%s" 
            ERR_RESET "\n", 
            line, 
            column, 
            message, 
            lexeme ? lexeme : "");
}
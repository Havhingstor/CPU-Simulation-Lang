#include "y.tab.h"
#include <stdio.h>
#include <string.h>

extern int yylex();
void yyerror(char *s);
void printTokenType(int *t);
union YYSTYPE yylval;
void printVal();
int *tokenType;

int main() {
    for (int token = yylex(); token != 0; token = yylex()) {
        printTokenType(&token);
        printVal();
    }
    return 0;
}

void printTokenType(int *t) {
    char * assignment[] = {"NUMBER","IDENTIFIER","PROGRAM","BEGIN","END","VAR","PROCEDURE","FUNCTION","IF","THEN","ELSE","WHILE","DO","REPEAT","UNTIL","FOR","TO","BY","RETURN"};
    
    tokenType = t;

    printf("\n\n");

    int specialTokenNr = *t - 258;
    
    if (specialTokenNr < 0 || specialTokenNr > 17) {
        printf("Symbol %s", (char *) t);
    } else {
        printf("Token: %s", assignment [specialTokenNr]);
    }

    printf("\n");
}

void yyerror (char *s)
{
        fprintf(stderr, "%s\n", s);
}

void printVal() {
    if (*tokenType == 258) {
        printf("Value: %d\n", yylval.value);
    } else if (*tokenType == 259) {
        printf("Name: %s\n", yylval.name);
    } else {
        printf("No value!\n");
    }
}

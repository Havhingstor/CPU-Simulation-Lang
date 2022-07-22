    #include "y.tab.h"
    #include <stdlib.h>
    #include <stdio.h>

    char *transferStr(const char *origin);

%x comment
%option yylineno

whitespace [ \t\n]

%%

"(*"            BEGIN comment;
<comment>"*)"   BEGIN 0;
<comment>.      ;
<comment>{whitespace}   ;

(?i:program)    return _PROGRAM;
(?i:begin)      return _BEGIN;
(?i:end)        return _END;
(?i:var)        return _VAR;
(?i:procedure)  return _PROCEDURE;
(?i:function)   return _FUNCTION;
(?i:if)         return _IF;
(?i:then)       return _THEN;
(?i:else)       return _ELSE;
(?i:while)      return _WHILE;
(?i:do)         return _DO;
(?i:repeat)     return _REPEAT;
(?i:until)      return _UNTIL;
(?i:for)        return _FOR;
(?i:to)         return _TO;
(?i:by)         return _BY;
(?i:return)     return _RETURN;

[0-9]+                  { yylval.value = atoi(yytext); return NUMBER; }

{whitespace}            ;

[A-Za-z][A-Za-z0-9$_]*  { yylval.name = transferStr(yytext); return IDENTIFIER; }

[!-~]                   return yytext [0];

%%

int yywrap(void) {
    return 1;
}


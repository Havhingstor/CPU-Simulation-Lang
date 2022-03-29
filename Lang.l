	#include "y.tab.h"

%%

(?i:program)	return _PROGRAM;
(?i:begin)		return _BEGIN;
(?i:end)		return _END;
(?i:var)		return _VAR;
(?i:procedure)	return _PROCEDURE;
(?i:function)	return _FUNCTION;
(?i:if)			return _IF;
(?i:then)		return _THEN;
(?i:else)		return _ELSE;
(?i:while)		return _WHILE;
(?i:do)			return _DO;
(?i:repeat)		return _REPEAT;
(?i:until)		return _UNTIL;
(?i:for)		return _FOR;
(?i:by)			return _BY;
(?i:return)		return _RETURN;

.				{ yylval.name = yytext; return IDENTIFIER; }

%%

int yywrap(void) {
	return 1;
}

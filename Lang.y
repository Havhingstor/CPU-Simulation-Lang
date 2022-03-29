%{
#include <stdio.h>
int yylex();
void yyerror(char * s);
%}

%%

%token <value> NUMBER;
%token <name> IDENTIFIER;
%token _PROGRAM _BEGIN _END _VAR _PROCEDURE _FUNCTION _IF _THEN;
%token _ELSE _WHILE _DO _REPEAT _UNTIL _FOR _BY _RETURN;

%union {
	char * name;
	int value;
};

%left '+' '-';
%left '*' '/' '%';

%start program;

epsilon			:										{;}
				;

program			: head varSections procedures body		{;}
				;

head			: _PROGRAM IDENTIFIER ';'				{;}
				;

body			: _BEGIN instructionSequence _END
				IDENTIFIER '.'							{;}
				;

varSections		: varSections varSection				{;}
				| varSection							{;}
				| epsilon								{;}
				;

varSection		: _VAR varDeclarations ';'				{;}
				;

procedureVarSection
				: varSection							{;}
				| epsilon								{;}
				;

varDeclarations:
	  varDeclarations ',' varDeclaration		{;}
	| varDeclaration		{;};

varDeclaration:
	  IDENTIFIER '[' NUMBER ']'		{;}
    | IDENTIFIER		{;};

procedures:
	  procedures procedure		{;}
	| procedure		{;}
    | epsilon		{;};

procedure:
	  procedureHeader IDENTIFIER '(' paramList ')' procedureVarSection
	  	_BEGIN instructionSequence _END IDENTIFIER		{;};

procedureHeader:
	  _PROCEDURE		{;}
	| _FUNCTION		{;};

paramList:
	  paramList ',' parameter		{;}
	| parameter		{;}
	| epsilon		{;};

parameter:
	  _VAR varDeclaration		{;}
	| varDeclaration		{;};

instructionSequence:
	  instructionSequence ';' instruction		{;}
	| instruction		{;};

instruction:
	  assignment		{;}
	| conditionalInstruction		{;}
	| whileLoop		{;}
	| repeatUntilLoop		{;}
	| forLoop		{;}
	| procedureCall		{;}
	| returnStatement		{;};

assignment:
	varCall ':' '=' expression		{;};

varCall:
	  IDENTIFIER '[' expression ']'		{;}
	| IDENTIFIER		{;};

conditionalInstruction:
	_IF condition _THEN instructionSequence elseSection _END		{;};

elseSection:
	  _ELSE instructionSequence		{;}
	| epsilon		{;};

whileLoop:
	_WHILE condition _DO instructionSequence _END		{;};

repeatUntilLoop:
	_REPEAT instructionSequence _UNTIL condition		{;};

forLoop:
	_FOR IDENTIFIER ':' '=' expression iterativeAdvancement _DO instructionSequence _END		{;};

iterativeAdvancement:
	  _BY '+' NUMBER		{;}
	| _BY '-' NUMBER		{;}
	| epsilon		{;};

procedureCall:
	IDENTIFIER '(' paramListCall ')'		{;};

paramListCall:
	  paramListCall ',' expression		{;}
	| expression		{;}
	| epsilon		{;};

returnStatement:
	  _RETURN expression		{;}
	| _RETURN		{;};

condition:
	expression conditionalOperator expression		{;};

conditionalOperator:
	  '='		{;}
	| '<' '>'		{;}
	| '<'		{;}
	| '>'		{;}
	| '<' '='		{;}
	| '>' '='		{;};

expression:
	  '(' expression ')'		{;}
	| '-' expression %prec '*'		{;}
	| expression binaryOperator expression		{;}
	| value		{;};

binaryOperator:
	  '+'		{;}
	| '-'		{;}
	| '*'		{;}
	| '/'		{;}
	| '%'		{;};

value:
	  varCall		{;}
	| NUMBER		{;}
	| procedureCall		{;};

%%

int main() {
	return yyparse();
}

void yyerror (char *s)
{
	fprintf(stderr, "%s\n", s);
}

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

program			: head varSections procedures body		{printf("Program\n");}
				;

head			: _PROGRAM IDENTIFIER ';'				{printf("Head\n");}
				;

body			: _BEGIN instructionSequence _END
					IDENTIFIER '.'						{printf("Body\n");}
				;

varSections		: varSections varSection				{printf("Var Sections\n");}
				| varSection							{;}
				| epsilon								{;}
				;

varSection		: _VAR varDeclarations ';'				{printf("Var Section\n");}
				;

procedureVarSection
				: varSection							{printf("P Var Section\n");}
				| epsilon								{;}
				;

varDeclarations	: varDeclarations ',' varDeclaration	{printf("Var Declarations\n");}
				| varDeclaration						{;}
				;

varDeclaration	: IDENTIFIER '[' NUMBER ']'				{printf("Array Declaration\n");}
			    | IDENTIFIER							{printf("Int declaration\n");}
				;

procedures		: procedures procedure					{printf("Procedures\n");}
				| procedure								{;}
			    | epsilon								{;}
				;

procedure		: procedureHeader IDENTIFIER '(' paramList ')' ';'
		   			procedureVarSection _BEGIN instructionSequence _END
					IDENTIFIER ';'						{printf("Procedure\n");}
				;

procedureHeader	: _PROCEDURE							{printf("Procedure-Header\n");}
				| _FUNCTION								{;}
				;

paramList		: paramList ',' parameter				{printf("Param List\n");}
				| parameter								{;}
				| epsilon		{;}
				;

parameter		: _VAR varDeclaration					{printf("Parameter\n");}
				| varDeclaration						{;}
				;

instructionSequence
				: instruction ';' instructionSequence	{printf("Instruction Sequence\n");}
				| instruction ';'						{;}
				| instruction							{;}
				;

instruction		: assignment							{printf("Instruction: Assignment\n");}
				| conditionalInstruction				{printf("Instruction: Conditional Instruction\n");}
				| whileLoop								{printf("Instruction: While-Loop\n");}
				| repeatUntilLoop						{printf("Instruction: Repeat-Until-Loop\n");}
				| forLoop								{printf("Instruction: For-Loop\n");}
				| procedureCall							{printf("Instruction: Procedure-Call\n");}
				| returnStatement						{printf("Instruction: Return-Statement\n");}
				;

assignment		: varCall ':' '=' expression			{printf("Assignment\n");}
				;

varCall			: IDENTIFIER '[' expression ']'			{printf("Var-Call: Array\n");}
				| IDENTIFIER							{printf("Var-Call: Int\n");}
				;

conditionalInstruction
				: _IF condition _THEN instructionSequence
					elseSection _END					{;}
				;

elseSection		: _ELSE instructionSequence				{printf("Else-Segment\n");}
				| epsilon								{;}
				;

whileLoop		: _WHILE condition
		   			_DO instructionSequence _END		{;}
				;

repeatUntilLoop	: _REPEAT instructionSequence
					_UNTIL condition					{;}
				;

forLoop			: _FOR IDENTIFIER ':' '=' expression
		  			iterativeAdvancement
					_DO instructionSequence _END		{;}
				;

iterativeAdvancement
				: _BY '+' NUMBER						{printf("Positive iterative advancement\n");}
				| _BY '-' NUMBER						{printf("Negative iterative advancement\n");}
				| epsilon								{;}
				;

procedureCall	: IDENTIFIER '(' paramListCall ')'		{;}
			  	;

paramListCall	: paramListCall ',' expression			{printf("Param-List-Call\n");}
				| expression							{;}
				| epsilon								{;}
				;

returnStatement	: _RETURN expression					{printf("Return with Value\n");}
				| _RETURN								{printf("Return without Value\n");}
				;

condition		: expression conditionalOperator
		   			expression							{printf("Condition\n");}
				;

conditionalOperator
				: '='									{printf("Conditional =\n");}
				| '<' '>'								{printf("Conditional <>\n");}
				| '<'									{printf("Conditional <\n");}
				| '>'									{printf("Conditional >\n");}
				| '<' '='								{printf("Conditional <=\n");}
				| '>' '='								{printf("Conditional >=\n");}
				;

expression		: '(' expression ')'					{printf("Brackets\n");}
				| '-' expression %prec '*'				{printf("Negative\n");}
				| expression binaryOperator expression	{printf("Term\n");}
				| value									{;}
				;

binaryOperator	: '+'									{printf("Plus\n");}
				| '-'									{printf("Minus\n");}
				| '*'									{printf("Times\n");}
				| '/'									{printf("Divided by\n");}
				| '%'									{printf("Modulo\n");}
				;

value			: varCall								{printf("Value: Var\n");}
				| NUMBER								{printf("Value: Number\n");}
				| procedureCall							{printf("Value: Procedure\n");}
				;

%%

int main() {
	return yyparse();
}

void yyerror (char *s)
{
	fprintf(stderr, "%s\n", s);
}

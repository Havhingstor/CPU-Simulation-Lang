%%

%token <value> NUMBER;
%token <name> IDENTIFIER;
%token PROGRAM;
%token BEGIN;
%token END;
%token VAR;
%token PROCEDURE;
%token FUNCTION;
%token IF;
%token THEN;
%token ELSE;
%token WHILE;
%token DO;
%token REPEAT;
%token UNTIL;
%token FOR;
%token BY;
%token RETURN;

%union {
	char * name
	int value
};

%left '+' '-';
%left '*' '/' '%';

%start program;

program:
	head varSections procedures body;

head:
	PROGRAM IDENTIFIER ';';

body:
	BEGIN instructionSequence END IDENTIFIER '.';

varSections:
	  /* Epsilon */
	| varSection
	| varSections varSection;

varSection:
	VAR varDeclarations ';';

procedureVarSection:
	  /* Epsilon */
	| varSection;

varDeclarations:
	  varDeclaration
	| varDeclarations ',' varDeclaration;

varDeclaration:
      IDENTIFIER
	| IDENTIFIER '[' NUMBER ']';

procedures:
      /* Epsilon */
	| procedure
	| procedures procedure;

procedure:
	  procedureHeader IDENTIFIER '(' paramList ')' procedureVarSection
	  	BEGIN instructionSequence END IDENTIFIER;

procedureHeader:
	  PROCEDURE
	| FUNCTION;

paramList:
	  /* Epsilon */
	| parameter
	| paramList ',' parameter;

parameter:
	  VAR varDeclaration
	| varDeclaration;

instructionSequence:
	  instruction
	| instructionSequence ';' instruction;

instruction:
	  assignment
	| conditionalInstruction
	| whileLoop
	| repeatUntilLoop
	| forLoop
	| procedureCall
	| returnStatement;

assignment:
	varCall ':' '=' expression;

varCall:
	  IDENTIFIER
	| IDENTIFIER '[' expression ']';

conditionalInstruction:
	IF condition THEN instructionSequence elseSection END;

elseSection:
	  /* Epsilon */
	| ELSE instructionSequence;

whileLoop:
	WHILE condition DO instructionSequence END;

repeatUntilLoop:
	REPEAT instructionSequence UNTIL condition;

forLoop:
	FOR IDENTIFIER ':' '=' expression iterativeAdvancement DO instructionSequence END;

iterativeAdvancement:
	  /* Epsilon */
	| BY '+' NUMBER
	| BY '-' NUMBER;

procedureCall:
	IDENTIFIER '(' paramListCall ')';

paramListCall:
	  /* Epsilon */
	| expression
	| paramListCall ',' expression;

returnStatement:
	  RETURN
	| RETURN expression;

condition:
	expression conditionalOperator expression;

conditionalOperator:
	  '='
	| '<' '>'
	| '<'
	| '>'
	| '<' '='
	| '>' '=';

expression:
	  '(' expression ')'
	| expression binaryOperator expression
	| '-' expression %prec '*'
	| value;

binaryOperator:
	  '+'
	| '-'
	| '*'
	| '/'
	| '%';

value:
	  varCall
	| NUMBER
	| procedureCall;

%%

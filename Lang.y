%%

%token <value> NUMBER;
%token <name> IDENTIFIER;
%token PROGRAM BEGIN END VAR PROCEDURE FUNCTION IF THEN ELSE WHILE DO REPEAT UNTIL FOR BY RETURN;

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
	  varSections varSection
	| varSection
	|  /* Epsilon */;

varSection:
	VAR varDeclarations ';';

procedureVarSection:
	  varSection
	| /* Epsilon */;

varDeclarations:
	  varDeclarations ',' varDeclaration
	| varDeclaration;

varDeclaration:
	  IDENTIFIER '[' NUMBER ']'
    | IDENTIFIER;

procedures:
	  procedures procedure
	| procedure
    | /* Epsilon */;

procedure:
	  procedureHeader IDENTIFIER '(' paramList ')' procedureVarSection
	  	BEGIN instructionSequence END IDENTIFIER;

procedureHeader:
	  PROCEDURE
	| FUNCTION;

paramList:
	  paramList ',' parameter
	| parameter
	| /* Epsilon */;

parameter:
	  VAR varDeclaration
	| varDeclaration;

instructionSequence:
	  instructionSequence ';' instruction
	| instruction;

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
	  IDENTIFIER '[' expression ']'
	| IDENTIFIER;

conditionalInstruction:
	IF condition THEN instructionSequence elseSection END;

elseSection:
	  ELSE instructionSequence
	| /* Epsilon */;

whileLoop:
	WHILE condition DO instructionSequence END;

repeatUntilLoop:
	REPEAT instructionSequence UNTIL condition;

forLoop:
	FOR IDENTIFIER ':' '=' expression iterativeAdvancement DO instructionSequence END;

iterativeAdvancement:
	  BY '+' NUMBER
	| BY '-' NUMBER
	| /* Epsilon */;

procedureCall:
	IDENTIFIER '(' paramListCall ')';

paramListCall:
	  paramListCall ',' expression
	| expression
	| /* Epsilon */;

returnStatement:
	  RETURN expression
	| RETURN;

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
	| '-' expression %prec '*'
	| expression binaryOperator expression
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

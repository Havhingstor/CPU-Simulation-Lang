%{
#include <stdio.h>
#include <string.h>
#include "parsetree.h"
#include "main.h"

extern int yylex( void );
extern void yyerror(const char * s);
extern FILE *yyin;
char *transferStr(const char *origin);

parseToken *programToken;
%}

%%

%token <value> NUMBER;
%token <name> IDENTIFIER;
%token _PROGRAM _BEGIN _END _VAR _PROCEDURE _FUNCTION _IF _THEN;
%token _ELSE _WHILE _DO _REPEAT _UNTIL _FOR _TO _BY _RETURN;

%type <parsed> varSections procedures body varSection varDeclarations procedureVarSection;
%type <parsed> varDeclaration procedure procedureHeader paramList parameter instructionSequence instruction;
%type <parsed> varCall elseSection whileLoop repeatUntilLoop forLoop;
%type <parsed> iterativeAdvancement conditionalInstruction value procedureCall assignment;
%type <parsed> paramListCall returnStatement condition expression binaryExpression;
%type <value> conditionalOperator;
%type <name> head;

%union {
    char *name;
    int value;
    struct parseToken *parsed;
};

%left '+' '-';
%left '*' '/' '%';

%start program;

epsilon         :                                       {;}
                ;

program         : head varSections procedures body      {programToken = createProgram($1, $2, $3, $4);}
                ;

                head: _PROGRAM IDENTIFIER ';'           {$$ = $2;}
                ;

body            : _BEGIN instructionSequence _END
                    IDENTIFIER '.'                      {$$ = createBody($2, $4);}
                ;

varSections     : varSections varSection                {$$ = createVarSections($1, $2);}
                | varSection                            {$$ = createVarSections(NULL, $1);}
                | epsilon                               {$$ = createVarSections(NULL, NULL);}
                ;

varSection      : _VAR varDeclarations ';'              {$$ = createVarSection($2);}
                ;

procedureVarSection
                : varSection                            {$$ = createVarSections(NULL, $1);}
                | epsilon                               {$$ = createVarSections(NULL, NULL);}
                ;

varDeclarations : varDeclarations ',' varDeclaration    {$$ = createVarDeclarations($1, $3);}
                | varDeclaration                        {$$ = createVarDeclarations(NULL, $1);}
                ;

varDeclaration  : IDENTIFIER '[' NUMBER ']'             {$$ = createVarDeclaration($1, &$3);}
                | IDENTIFIER                            {$$ = createVarDeclaration($1, NULL);}
                ;

procedures      : procedures procedure                  {$$ = createProcedures($1, $2);}
                | procedure                             {$$ = createProcedures(NULL, $1);}
                | epsilon                               {$$ = createProcedures(NULL, NULL);}
                ;

procedure       : procedureHeader IDENTIFIER '(' paramList ')' ';'
                    procedureVarSection _BEGIN instructionSequence _END
                    IDENTIFIER ';'                      {$$ = createProcedure($1, $2, $4,
                        $7, $9, $11);}
                ;

procedureHeader : _PROCEDURE                            {$$ = createProcedureHeader();}
                | _FUNCTION                             {$$ = createFunctionHeader();}
                ;

paramList       : paramList ',' parameter               {$$ = createParamList($1, $3);}
                | parameter                             {$$ = createParamList(NULL, $1);}
                | epsilon                               {$$ = createParamList(NULL, NULL);}
                ;

parameter       : _VAR varDeclaration                   {$$ = createReferenceParameter($2);}
                | varDeclaration                        {$$ = createCopyParameter($1);}
                ;

instructionSequence
                : instruction ';' instructionSequence   {$$ = createInstructionSequence($1, $3);}
                | instruction ';'                       {$$ = createInstructionSequence($1, NULL);}
                | instruction                           {$$ = createInstructionSequence($1, NULL);}
                ;

instruction     : assignment                            {$$ = $1;}
                | conditionalInstruction                {$$ = $1;}
                | whileLoop                             {$$ = $1;}
                | repeatUntilLoop                       {$$ = $1;}
                | forLoop                               {$$ = $1;}
                | procedureCall                         {$$ = $1;}
                | returnStatement                       {$$ = $1;}
                ;

assignment      : varCall ':' '=' expression            {$$ = createAssignment($1, $4);}
                ;

varCall         : IDENTIFIER '[' expression ']'         {$$ = createArrayCall($1, $3);}
                | IDENTIFIER                            {$$ = createVarCall($1);}
                ;

conditionalInstruction
                : _IF condition _THEN instructionSequence
                    elseSection _END                    {$$ = createConditional($2, $4, $5);}
                ;

elseSection     : _ELSE instructionSequence             {$$ = createElseSection($2);}
                | epsilon                               {$$ = createElseSection(NULL);}
                ;

whileLoop       : _WHILE condition
                    _DO instructionSequence _END        {$$ = createWhileLoop($2, $4);}
                ;

repeatUntilLoop : _REPEAT instructionSequence
                    _UNTIL condition                    {$$ = createRepeatLoop($2, $4);}
                ;

forLoop         : _FOR assignment _TO expression
                    iterativeAdvancement
                    _DO instructionSequence _END        {$$ = createForLoop($2, $4, $5, $7);}
                ;

iterativeAdvancement
                : _BY '+' NUMBER                        {$$ = createPositiveAdvancement($3);}
                | _BY NUMBER                            {$$ = createPositiveAdvancement($2);}
                | _BY '-' NUMBER                        {$$ = createNegativeAdvancement($3);}
                | epsilon                               {$$ = createPositiveAdvancement(1);}
                ;

procedureCall   : IDENTIFIER '(' paramListCall ')'      {$$ = createProcedureCall($1, $3);}
                ;

paramListCall   : paramListCall ',' expression          {$$ = createParamListCall($1, $3);}
                | expression                            {$$ = createParamListCall(NULL, $1);}
                | epsilon                               {$$ = createParamListCall(NULL, NULL);}
                ;

returnStatement : _RETURN expression                    {$$ = createReturnStatement($2);}
                | _RETURN                               {$$ = createReturnStatement(NULL);}
                ;

condition       : expression conditionalOperator
                    expression                          {$$ = createCondition($1, $2, $3);}
                ;

conditionalOperator
                : '='                                   {$$ = 0;}
                | '<' '>'                               {$$ = 1;}
                | '<'                                   {$$ = 2;}
                | '>'                                   {$$ = 3;}
                | '<' '='                               {$$ = 4;}
                | '>' '='                               {$$ = 5;}
                ;

expression
                : '(' expression ')'                    {$$ = createBrackets($2);}
                | '-' expression                        {$$ = createNegation($2);}
                | value                                 {$$ = createUnaryExpression($1);}
                | binaryExpression                      {$$ = $1;}
                ;

binaryExpression: expression '+' expression             {$$ = createBinaryExpression($1, 0, $3);}
                | expression '-' expression             {$$ = createBinaryExpression($1, 1, $3);}
                | expression '*' expression             {$$ = createBinaryExpression($1, 2, $3);}
                | expression '/' expression             {$$ = createBinaryExpression($1, 3, $3);}
                | expression '%' expression             {$$ = createBinaryExpression($1, 4, $3);}
                ;

value           : varCall                               {$$ = createValueByCall($1);}
                | NUMBER                                {$$ = createValue($1);}
                | procedureCall                         {$$ = createValueByCall($1);}
                ;

%%

int main(int argc, char **argv) {
	FILE *in = fopen("", "r");
	if (argc > 1) {
		in = fopen(argv[1], "r");
		if (!in) {
			fprintf(stderr, "File couldn't be read, using stdin.\n");
		} else {
			yyin = in;
		}
	}
    int success = yyparse();
	if (in) {
		fclose(in);
	}
    return handle(programToken, success);
}

void yyerror (const char *s)
{
    extern int yylineno;
    fprintf(stderr, "An error occurred (%s) in line %i!\n", s, yylineno);
}
    
char *transferStr(const char *origin) {
    char *target = malloc(strlen(origin) + 1);
    strcpy(target, origin);
    return target;
}

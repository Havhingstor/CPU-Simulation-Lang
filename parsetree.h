#ifndef PARSETREE_H
#define PARSETREE_H

#include "y.tab.h"

typedef enum parseType parseType;

enum parseType {
	program,
	body,
	varSections,
	varSection,
	varDeclarations,
	varDeclaration,
	procedures,
	procedure,
	procedureHeader,
	functionHeader,
	paramList,
	copyParameter,
	referenceParameter,
	instructionSequence,
	instruction,
	assignment,
	varCall,
	arrayCall,
	conditionalInstruction,
	elseSection,
	whileLoop,
	repeatLoop,
	forLoop,
	positiveAdvancement,
	negativeAdvancement,
    emptyAdvancement,
	procedureCall,
	paramListCall,
	returnStatement,
	condition,
	negation,
	expression,
	value
};

char *stringFromParseType(parseType type);

typedef enum valueType valueType;

enum valueType {
	string,
	number,
	token
};

typedef struct parseToken parseToken;

struct parseToken {
	parseType type;
	YYSTYPE *values;
	valueType *valueTypes;
	int nVal;
	parseToken **subNodes;
	int nNodes;
};

void freeToken(parseToken *token);

parseToken *createProgram(char *name, parseToken *varSections,
		parseToken *procedures, parseToken *body);

parseToken *createBody(parseToken *instructionSequence, char *name);

parseToken *createVarSections(parseToken *prevVarSections,
		parseToken *varSection);

parseToken *createVarSection(parseToken *varDeclarations);

parseToken *createVarDeclarations(parseToken *prevVarDeclarations,
		parseToken *varDeclaration);

parseToken *createVarDeclaration( char *name, const int *arraySize);

parseToken *createProcedures(parseToken *prevProcedures, parseToken *procedure);

parseToken *createProcedure(parseToken *header, char *name, parseToken *paramList,
		parseToken *varSection, parseToken *instructionSequence,
		char *confirmIdentifier);

parseToken *createProcedureHeader( void );

parseToken *createFunctionHeader( void );

parseToken *createParamList(parseToken *prevParamList, parseToken *parameter);

parseToken *createCopyParameter(parseToken *varDeclaration);

parseToken *createReferenceParameter(parseToken *varDeclaration);

parseToken *createInstructionSequence(parseToken *newInstruction,
		parseToken *prevInstructionSequence);

parseToken *createAssignment(parseToken *var, parseToken *expr);

parseToken *createVarCall(char *name);

parseToken *createArrayCall(char *name, parseToken *index);

parseToken *createConditional(parseToken *cond,
		parseToken *instructions, parseToken *elseSection);

parseToken *createElseSection(parseToken *instructions);

parseToken *createWhileLoop(parseToken *condition, parseToken *instrutions);

parseToken *createRepeatLoop(parseToken *instructions, parseToken *condition);

parseToken *createForLoop(char *varName, parseToken *init,
		parseToken *target, parseToken *iteration, parseToken *instructions);

parseToken *createPositiveAdvancement(int num);

parseToken *createNegativeAdvancement(int num);

parseToken *createEmptyAdvancement( void );

parseToken *createProcedureCall(char *name, parseToken *paramList);

parseToken *createParamListCall(parseToken *prevParams, parseToken *param);

parseToken *createReturnStatement(parseToken *returnedExpression);

parseToken *createCondition(parseToken *firstOp, int opCode, parseToken *secondOp);

parseToken *createBrackets(parseToken *internal);

parseToken *createNegation(parseToken *internal);

parseToken *createBinaryExpression(parseToken *firstOp, int opCode, parseToken *secondOp);

parseToken *createUnaryExpression(parseToken *value);

parseToken *createValue(int num);

parseToken *createValueByCall(parseToken *call);

#endif //PARSETREE_H

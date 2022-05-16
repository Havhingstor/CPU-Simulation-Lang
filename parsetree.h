#ifndef PARSETREE_H
#define PARSETREE_H

#include "y.tab.h"

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
	parameter,
	instructionSequence,
	instruction,
	assignment,
	varCall,
	arrayCall,
	conditionalInstruction,
	whileLoop,
	repeatLoop,
	forLoop,
	iterativeAdvancement,
	procedureCall,
	returnStatement,
	condition,
	expression,
	value
};


struct parseToken {
	enum parseType type;
	YYSTYPE *values;
	int nVal;
	struct parseToken **subNodes;
	int nNodes;
};

struct parseToken *createProgram(char *name, struct parseToken *varSections,
		struct parseToken *procedures, struct parseToken *body);

struct parseToken *createBody(struct parseToken *instructionSequence, char *name);

struct parseToken *createVarSections(const struct parseToken *prevVarSections,
		const struct parseToken *varSection);

struct parseToken *createVarSection(struct parseToken *varDeclarations);

struct parseToken *createVarDeclarations(const struct parseToken *prevVarDeclarations,
		struct parseToken *varDeclaration);

struct parseToken *createVarDeclaration( char *name, const int *arraySize);

struct parseToken *createProcedures(const struct parseToken *prevProcedures, const struct parseToken *procedure);

struct parseToken *createProcedure(struct parseToken *header, char *name, struct parseToken *paramList,
		struct parseToken *varSection, struct parseToken *instructionSequence,
		char *confirmIdentifier);

struct parseToken *createProcedureHeader();

struct parseToken *createFunctionHeader();

struct parseToken *createParamList(struct parseToken *prevParamList, struct parseToken *parameter);

struct parseToken *createCopyParameter(struct parseToken *varDelaration);

struct parseToken *createReferenceParameter(struct parseToken *varDeclaration);

struct parseToken *createInstructionSequence(struct parseToken *newInstruction,
		struct parseToken *prevInstructionSequence);

struct parseToken *createAssignment(struct parseToken *var, struct parseToken *expr);

struct parseToken *createVarCall(char *name);

struct parseToken *createArrayCall(char *name, struct parseToken *index);

struct parseToken *createConditional(struct parseToken *cond,
		struct parseToken *instructions, struct parseToken *elseSection);

struct parseToken *createElseSection(struct parseToken *instructions);

struct parseToken *createWhileLoop(struct parseToken *condition, struct parseToken *instrutions);

struct parseToken *createRepeatLoop(struct parseToken *instructions, struct parseToken *condition);

struct parseToken *createForLoop(char *varName, struct parseToken *init,
		struct parseToken *target, struct parseToken *iteration, struct parseToken *instructions);

struct parseToken *createPositiveAdvancement(int num);

struct parseToken *createNegativeAdvancement(int num);

struct parseToken *createProcedureCall(char *name, struct parseToken *paramList);

struct parseToken *createParamListCall(struct parseToken *prevParams, struct parseToken *param);

struct parseToken *createReturnStatement(struct parseToken *returnedExpression);

struct parseToken *createCondition(struct parseToken *firstOp, int opCode, struct parseToken *secondOp);

struct parseToken *createBrackets(struct parseToken *internal);

struct parseToken *createNegation(struct parseToken *internal);

struct parseToken *createBinaryExpression(struct parseToken *firstOp, int opCode, struct parseToken *secondOp);

struct parseToken *createUnaryExpression(struct parseToken *value);

struct parseToken *createValue(int num);

struct parseToken *createValueByCall(struct parseToken *call);

#endif //PARSETREE_H

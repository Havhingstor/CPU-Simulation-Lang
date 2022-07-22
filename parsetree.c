#include "parsetree.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

char *stringFromParseType(parseType type) {
    char *values [] = {"program", "body", "varSections", "varSection", "varDeclarations", "varDeclaration",
        "procedures", "procedure", "procedureHeader", "functionHeader", "paramList",
    "copyParameter", "referenceParameter", "instructionSequence", "instruction", "assignment", "varCall",
    "arrayCall", "conditionalInstruction", "elseSection", "whileLoop", "repeatLoop","forLoop",
    "positiveAdvancement", "negativeAdvancement", "emptyAdvancement", "procedureCall", "paramListCall",
    "returnStatement", "condition", "negation", "expression", "value"};
    
    return values[type];
}

void initVals(parseToken *token, int n) {
    token->nVal = n;
    token->values = (YYSTYPE *) calloc(n, sizeof(YYSTYPE));
    token->valueTypes = (valueType *) calloc(n, sizeof(valueType));
}

void initNodes(parseToken *token, int n) {
    token->nNodes = n;
    token->subNodes = (parseToken **) calloc(n, sizeof(parseToken *));
}

void freeToken(parseToken *token)
{
    if(token == NULL) {
        return;
    }

    if(token->values != NULL) {
        free(token->values);
    }
    
    for(int i = 0; i < token->nNodes; ++i) {
        parseToken *subNode = token->subNodes[i];
        if(subNode != NULL) {
            freeToken(token->subNodes[i]);
        }
    }
    
    if(token->subNodes != NULL) {
        free(token->subNodes);
    }
    
    free(token);
}

parseToken *createProgram(char *name, parseToken *varSections,
    parseToken *procedures, parseToken *body)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = program;
    
    initVals(result, 1);
    result->values[0].name = name;
    result->valueTypes[0] = string;
    
    initNodes(result, 3);
    result->subNodes[0] = varSections;
    result->subNodes[1] = procedures;
    result->subNodes[2] = body;
    
    return result;
}

parseToken *createBody(parseToken *instructionSequence, char *name)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = body;
    
    initVals(result, 1);
    result->values[0].name = name;
    result->valueTypes[0] = string;
    
    initNodes(result, 1);
    result->subNodes[0] = instructionSequence;
    
    return result;
}

parseToken *createVarSections(parseToken *prevVarSections,
        parseToken *varSection)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = varSections;
    
    initVals(result, 0);
    
    if(prevVarSections != NULL) {
        initNodes(result, 2);
        result->subNodes[0] = prevVarSections;
        result->subNodes[1] = varSection;
    } else if(varSection != NULL) {
        initNodes(result, 1);
        result->subNodes[0] = varSection;
    } else {
        initNodes(result, 0);
    }
    
    return result;
}

parseToken *createVarSection(parseToken *varDeclarations)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = varSection;
    
    initVals(result, 0);
    
    initNodes(result, 1);
    result->subNodes[0] = varDeclarations;
    
    return result;
}

parseToken *createVarDeclarations(parseToken *prevVarDeclarations,
        parseToken *varDeclaration)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = varDeclarations;
    
    initVals(result, 0);
    
    if(prevVarDeclarations != NULL) {
        initNodes(result, 2);
        result->subNodes[0] = prevVarDeclarations;
        result->subNodes[1] = varDeclaration;
    } else {
        initNodes(result, 1);
        result->subNodes[0] = varDeclaration;
    }
    
    return result;

}

parseToken *createVarDeclaration( char *name, const int *arraySize)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = varDeclaration;
    
    if(arraySize != NULL) {
        initVals(result, 2);
        result->values[1].value = *arraySize;
        result->valueTypes[1] = number;
    } else {
        initVals(result, 1);
    }
    
    result->values[0].name = name;
    result->valueTypes[0] = string;
    
    initNodes(result, 0);
    
    return result;

}

parseToken *createProcedures(parseToken *prevProcedures, parseToken *procedure)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = procedures;
    
    initVals(result, 0);
    
    if(prevProcedures != NULL) {
        initNodes(result, 2);
        result->subNodes[0] = prevProcedures;
        result->subNodes[1] = procedure;
    } else if(procedure != NULL) {
        initNodes(result, 1);
        result->subNodes[0] = procedure;
    } else {
        initNodes(result, 0);
    }
    
    return result;
}

parseToken *createProcedure(parseToken *header, char *name, parseToken *paramList,
        parseToken *varSection, parseToken *instructionSequence,
        char *confirmIdentifier)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = procedure;
    
    initVals(result, 2);
    result->values[0].name = name;
    result->valueTypes[0] = string;
    result->values[1].name = confirmIdentifier;
    result->valueTypes[1] = string;
    
    initNodes(result, 4);
    result->subNodes[0] = header;
    result->subNodes[1] = paramList;
    result->subNodes[2] = varSection;
    result->subNodes[3] = instructionSequence;
    
    return result;
    
}

parseToken *createProcedureHeader( void )
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = procedureHeader;
    
    initVals(result, 0);
    
    initNodes(result, 0);
        
    return result;

}

parseToken *createFunctionHeader( void )
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = functionHeader;
    
    initVals(result, 0);
    
    initNodes(result, 0);
        
    return result;

}

parseToken *createParamList(parseToken *prevParamList, parseToken *parameter)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = paramList;
    
    initVals(result, 0);
    
    if(prevParamList != NULL) {
        initNodes(result, 2);
        result->subNodes[0] = prevParamList;
        result->subNodes[1] = parameter;
    } else if(parameter != NULL) {
        initNodes(result, 1);
        result->subNodes[0] = parameter;
    } else {
        initNodes(result, 0);
    }
    
    return result;

}

parseToken *createCopyParameter(parseToken *varDeclaration)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = copyParameter;
    
    initVals(result, 0);
    
    initNodes(result, 1);
    result->subNodes[0] = varDeclaration;
    
    return result;

}

parseToken *createReferenceParameter(parseToken *varDeclaration)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = referenceParameter;
    
    initVals(result, 0);
    
    initNodes(result, 1);
    result->subNodes[0] = varDeclaration;
    
    return result;

}

parseToken *createInstructionSequence(parseToken *newInstruction,
        parseToken *prevInstructionSequence)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = instructionSequence;
    
    initVals(result, 0);
    
    if(prevInstructionSequence != NULL) {
        initNodes(result, 2);
        result->subNodes[1] = prevInstructionSequence;
    } else {
        initNodes(result, 1);
    }
    result->subNodes[0] = newInstruction;
    
    return result;

}

parseToken *createAssignment(parseToken *var, parseToken *expr)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = assignment;
    
    initVals(result, 0);
    
    initNodes(result, 2);
    result->subNodes[0] = var;
    result->subNodes[1] = expr;
    
    return result;

}

parseToken *createVarCall(char *name)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = varCall;
    
    initVals(result, 1);
    result->values[0].name = name;
    result->valueTypes[0] = string;
    
    initNodes(result, 0);
    
    return result;
}

parseToken *createArrayCall(char *name, parseToken *index)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = arrayCall;
    
    initVals(result, 1);
    result->values[0].name = name;
    result->valueTypes[0] = string;
    
    initNodes(result, 1);
    result->subNodes[0] = index;
    
    return result;
}

parseToken *createConditional(parseToken *cond,
        parseToken *instructions, parseToken *elseSection)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = conditionalInstruction;
    
    initVals(result, 0);
    
    initNodes(result, 3);
    result->subNodes[0] = cond;
    result->subNodes[1] = instructions;
    result->subNodes[2] = elseSection;
    
    return result;

}

parseToken *createElseSection(parseToken *instructions)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = elseSection;
    
    initVals(result, 0);
    
    if(instructions != NULL) {
        initNodes(result, 1);
        result->subNodes[0] = instructions;
    } else {
        initNodes(result, 0);
    }
    
    return result;

}

parseToken *createWhileLoop(parseToken *condition, parseToken *instrutions)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = whileLoop;
    
    initVals(result, 0);
    
    initNodes(result, 2);
    result->subNodes[0] = condition;
    result->subNodes[1] = instrutions;
    
    return result;

}

parseToken *createRepeatLoop(parseToken *instructions, parseToken *condition)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = repeatLoop;
    
    initVals(result, 0);
    
    initNodes(result, 2);
    result->subNodes[0] = instructions;
    result->subNodes[1] = condition;
    
    return result;

}

parseToken *createForLoop(parseToken *assignment,
        parseToken *target, parseToken *iteration, parseToken *instructions)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = forLoop;
    
    initVals(result, 0);
    
    initNodes(result, 4);
    result->subNodes[0] = assignment;
    result->subNodes[1] = target;
    result->subNodes[2] = iteration;
    result->subNodes[3] = instructions;
    
    return result;

}

parseToken *createPositiveAdvancement(int num)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = positiveAdvancement;
    
    initVals(result, 1);
    result->values[0].value = num;
        result->valueTypes[0] = number;

    
    initNodes(result, 0);
    
    return result;

}

parseToken *createNegativeAdvancement(int num)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = negativeAdvancement;
    
    initVals(result, 1);
    result->values[0].value = num;
        result->valueTypes[0] = number;

    
    initNodes(result, 0);
    
    return result;

}

parseToken *createProcedureCall(char *name, parseToken *paramList)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = procedureCall;
    
    initVals(result, 1);
    result->values[0].name = name;
    result->valueTypes[0] = string;
    
    initNodes(result, 1);
    result->subNodes[0] = paramList;
    
    return result;

}

parseToken *createParamListCall(parseToken *prevParams, parseToken *param)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = paramListCall;
    
    initVals(result, 0);
    
    if(prevParams != NULL) {
        initNodes(result, 2);
        result->subNodes[0] = prevParams;
        result->subNodes[1] = param;
    } else if(param != NULL) {
        initNodes(result, 1);
        result->subNodes[0] = param;
    } else {
        initNodes(result, 0);
    }
    
    return result;

}

parseToken *createReturnStatement(parseToken *returnedExpression)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = returnStatement;
    
    initVals(result, 0);
    
    if(returnedExpression != NULL) {
        initNodes(result, 1);
        result->subNodes[0] = returnedExpression;
    } else {
        initNodes(result, 0);
    }
    
    return result;


}

parseToken *createCondition(parseToken *firstOp, int opCode, parseToken *secondOp)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = condition;
    
    initVals(result, 1);
    result->values[0].value = opCode;
        result->valueTypes[0] = number;

    
    initNodes(result, 2);
    result->subNodes[0] = firstOp;
    result->subNodes[1] = secondOp;
    
    return result;

}

parseToken *createBrackets(parseToken *internal)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = expression;
    
    initVals(result, 0);
    
    initNodes(result, 1);
    result->subNodes[0] = internal;
    
    return result;

}

parseToken *createNegation(parseToken *internal)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = negation;
    
    initVals(result, 0);
    
    initNodes(result, 1);
    result->subNodes[0] = internal;
    
    return result;

}

parseToken *createBinaryExpression(parseToken *firstOp, int opCode, parseToken *secondOp)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = expression;
    
    initVals(result, 1);
    result->values[0].value = opCode;
        result->valueTypes[0] = number;

    
    initNodes(result, 2);
    result->subNodes[0] = firstOp;
    result->subNodes[1] = secondOp;
    
    return result;

}

parseToken *createUnaryExpression(parseToken *value)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = expression;
    
    initVals(result, 0);
    
    initNodes(result, 1);
    result->subNodes[0] = value;
    
    return result;
}

parseToken *createValue(int num)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = value;
    
    initVals(result, 1);
    result->values[0].value = num;
        result->valueTypes[0] = number;

    
    initNodes(result, 0);
    
    return result;

}

parseToken *createValueByCall(parseToken *call)
{
    parseToken *result = (parseToken *) malloc(sizeof(parseToken));
    
    result->type = value;
    
    initVals(result, 0);
    
    initNodes(result, 1);
    result->subNodes[0] = call;
    
    return result;

}


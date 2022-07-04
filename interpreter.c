#include "interpreter.h"
#include "parsetree.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>


char *catStrs(char target[], const char origin[]);
char *createStr(void);
char *transferStr(const char *);
char *itostr(int);


typedef struct varList varList;
struct varList {
	char **vars;
	int *varIsArray;
	int *varIsReference;
	int nVars;
};

varList *getVarList(void) {
    varList *result = (varList *) malloc(sizeof(varList));
    result->nVars = 0;
    result->vars = (char **) malloc(0);
    result->varIsArray = (int *) malloc(0);
    result->varIsReference = (int *) malloc(0);
    
    return result;
}

varList *cpVarList(varList *origin) {
    varList *result = malloc(sizeof(varList));
    
    int number = origin->nVars;
    
    result->nVars = number;
    result->vars = (char **) calloc(number, sizeof(char *));
    result->varIsArray = (int *) calloc(number, sizeof(int));
    result->varIsReference = (int *) calloc(number, sizeof(int));
    
    for(int i = 0; i < number; ++i) {
        result->vars[i] = transferStr(origin->vars[i]);
        result->varIsReference[i] = origin->varIsReference[i];
        result->varIsArray[i] = origin->varIsArray[i];
    }
    
    return result;
}

void freeVarList(varList *vl) {
    for(int i = 0; i < vl->nVars; ++i) {
        free(vl->vars[i]);
    }
    free(vl->vars);
    free(vl->varIsArray);
    free(vl->varIsReference);
    
    free(vl);
}

typedef struct functionDef functionDef;
struct functionDef {
	char *name;
	int isFunction;
	varList *parameters;
};

functionDef *createFunction(char *name, int isFunction) {
	functionDef *result = (functionDef *) malloc(sizeof(functionDef));
	result->name = name;
	result->isFunction = isFunction;
    result->parameters = getVarList();
	return result;
}

void freeFunctionDef(functionDef *fd) {
	free(fd->name);
    freeVarList(fd->parameters);
	free(fd);
}

typedef struct internalFunctionVals internalFunctionVals;
struct internalFunctionVals {
    varList *internalVars;
    functionDef *function;
    int sizeVarsOnStack;
    int nParams;
    int sizeParams;
};

internalFunctionVals *getIFVs(functionDef *function) {
    internalFunctionVals *result = (internalFunctionVals *) malloc(sizeof(internalFunctionVals));
    
    result->function = function;
    result->internalVars = cpVarList(function->parameters);
    result->nParams = 0;
    result->sizeVarsOnStack = 0;
    result->sizeParams = 0;
    
    return result;
}

void freeIFVs(internalFunctionVals *ifvs) {
    ifvs->function = NULL;
    freeVarList(ifvs->internalVars);
    free(ifvs);
}



typedef struct interpreterRessources interpreterRessources;
struct interpreterRessources {
	int returnVal;
	char **markers;
	int nMarkers;
	int nGenericMarkers;
	varList *vars;
	functionDef **functions;
	int nFunctions;
    internalFunctionVals *currentFunction;
    char *name;
};


void allocateInterpreterRessources(interpreterRessources *ir) {
	ir->returnVal = 0;
	ir->nMarkers = 0;
	ir->nGenericMarkers = 0;
	ir->markers = malloc(0);
    ir->vars = getVarList();
	ir->nFunctions = 0;
	ir->functions = malloc(0);
    ir->currentFunction = NULL;
    ir->name = NULL;
}

void freeIR(interpreterRessources *ir) {
	for(int i = 0; i < ir->nMarkers; ++i) {
		free(ir->markers[i]);
	}
	free(ir->markers);

    freeVarList(ir->vars);
	
	for(int i = 0; i < ir->nFunctions; ++i) {
		freeFunctionDef(ir->functions[i]);
	}
	free(ir->functions);
}

varList *getAdditionalVarList(interpreterRessources *ir, int canBeEmpty) {
    if(ir->currentFunction != NULL) {
        internalFunctionVals *ifvs = ir->currentFunction;
        if(ifvs->internalVars == NULL) {
            ir->returnVal = 1;
            fprintf(stderr, "The internal function values don't include additional variables!\n");
            return NULL;
        }
        return ifvs->internalVars;
    } else if(ir->currentFunction == NULL && canBeEmpty) {
        return NULL;
    } else {
        ir->returnVal = 1;
        fprintf(stderr, "There is no current function!\n");
        return NULL;
    }
    
    return NULL;
}

typedef enum varCallType varCallType;
enum varCallType {
	callFailure,

	singleValue,
	valueInArray,
	wholeArray,

	singleValueLocal,
	valueInArrayLocal,
	wholeArrayLocal
};
varCallType resolveVarCall(parseToken *, interpreterRessources *);
varCallType collapseVCType(varCallType);
int varCallIsLocal(parseToken *, interpreterRessources *);
char *getLocalVarCall(parseToken *, interpreterRessources *);

typedef enum expressionType expressionType;
enum expressionType {
	exprFailure,

	computedValue,
	singleValueVar,
	literalValue,
	array
};

int registerMarker(char *, interpreterRessources *);
char *getNumberedMarker(interpreterRessources *);
int registerVar(char *, int, int, interpreterRessources *, functionDef *);
void registerPUSH(interpreterRessources *);
void registerPULL(interpreterRessources *);
void registerPUSHNr(interpreterRessources *, int);
void registerPULLNr(interpreterRessources *, int);
expressionType getExpressionType(parseToken *, interpreterRessources *);
char *getExpressionCall(parseToken *, interpreterRessources *);
int getLiteralExpressionValue(parseToken *tok, interpreterRessources *ir);
char *varAddressInSP(parseToken *, interpreterRessources *);
parseToken *getExpressionUnderlyingVarCall(parseToken *, interpreterRessources *);
int getArraySize(parseToken *, interpreterRessources *);

char *getProgram(parseToken *, interpreterRessources *);
char *getProcedures(parseToken *, interpreterRessources *);
char *getProcedure(parseToken *, interpreterRessources *);
char *getProcedureCall(parseToken *, interpreterRessources *, int);
char *getBody(parseToken *, char *, interpreterRessources *);
void parseVars(parseToken *, interpreterRessources *);
char *getGlobalVarString(interpreterRessources *);
char *getInstructionSequence(parseToken *, interpreterRessources *);
char *getInstruction(parseToken *, interpreterRessources *);
char *getExpression(parseToken *, interpreterRessources *);
char *getCondition(parseToken *, char *, interpreterRessources *, int);

char *createAssembly(parseToken *programToken, int *returnVal) {
	interpreterRessources ir;
	allocateInterpreterRessources(&ir);
	char *result = getProgram(programToken, &ir);
	*returnVal = ir.returnVal;
	
	freeIR(&ir);

	return result;
}

char *createFirstCommand(char *name) {
	char *result = createStr();

	result = catStrs(result, "\tJMP\t\t");
	result = catStrs(result, name);
	result = catStrs(result, "$Start\n");

	return result;
}

char *getProgram(parseToken *tok, interpreterRessources *ir) {
	if(tok->type != program) {
		ir->returnVal = 1;
		fprintf(stderr, "The first token is not a program, but a %s!\n",
				stringFromParseType(tok->type));
		return createStr();
	}

	char *name = tok->values[0].name;
    ir->name = name;

	char *result = createStr();

    char *firstCommand = createFirstCommand(name);
	result = catStrs(result, firstCommand);
	free(firstCommand);
	
	parseVars(tok->subNodes[0], ir);
    
    char *procedures = getProcedures(tok->subNodes[1], ir);
    result = catStrs(result, procedures);
    free(procedures);

	char *body = getBody(tok->subNodes[2], name, ir);
	result = catStrs(result, body);
	free(body);

	char *pVars = getGlobalVarString(ir);
	result = catStrs(result, pVars);
	free(pVars);

	return result;
}

char *getProcedures(parseToken *tok, interpreterRessources *ir) {
	if(tok->type != procedures) {
		ir->returnVal = 1;
		fprintf(stderr, "The token is not a procedures, but a %s!\n",
				stringFromParseType(tok->type));
		return createStr();
	}

	if(tok->nNodes == 0) {
		return createStr();
	}

	char *result;
	parseToken *newTok;

	if(tok->nNodes == 2) {
		result = getProcedures(tok->subNodes[0], ir);
		newTok = tok->subNodes[1];
	} else {
		result = createStr();
		newTok = tok->subNodes[0];
	}

	char *newProcedure = getProcedure(newTok, ir);
	result = catStrs(result, newProcedure);
	free(newProcedure);

	return result;
}

int resolveProcedureHeader(parseToken *tok, interpreterRessources *ir) {
	if(tok->type == procedureHeader) {
		return 0;
	} else if(tok->type == functionHeader) {
		return 1;
	} else {
		ir->returnVal = 1;
		fprintf(stderr,
				"The token is not a function- or procedure-header, but a %s!\n",
				stringFromParseType(tok->type));
		return 0;
	}
}

void parseParamDeclaration(parseToken *tok, interpreterRessources *ir, int reference, functionDef *functions) {
	if(tok->type != varDeclaration) {
		ir->returnVal = 1;
		fprintf(stderr, "The token is not a varDeclaration, but a %s!\n",
				stringFromParseType(tok->type));
		return;
	}

	int array = 0;

	if(tok->nVal == 2) {
		array = tok->values[1].value;
	}

	char *name = tok->values[0].name;

	registerVar(name, array, reference, ir, functions);
}

void parseParam(parseToken *tok, interpreterRessources *ir, functionDef *functions) {
	if(tok->type != copyParameter && tok->type != referenceParameter) {
		ir->returnVal = 1;
		fprintf(stderr, "The token is not a copy- or reference-parameter, but a %s!\n",
				stringFromParseType(tok->type));
		return;
	}
	
	int reference = tok->type == referenceParameter;

	parseParamDeclaration(tok->subNodes[0], ir, reference, functions);
}

void parseParams(parseToken *tok, interpreterRessources *ir, functionDef *functions) {
	if(tok->type != paramList) {
		ir->returnVal = 1;
		fprintf(stderr, "The token is not a paramList, but a %s!\n",
				stringFromParseType(tok->type));
		return;
	}

	parseToken *param;

	if(tok->nNodes == 2) {
		parseParams(tok->subNodes[0], ir, functions);
		param = tok->subNodes[1];
	} else {
		param = tok->subNodes[0];
	}

	if(tok->nNodes == 0) {
		return;
	}
    
	parseParam(param, ir, functions);
}

int getSizeOnStack(varList *list) {
    int result = 0;
    
    for(int i = 0; i < list->nVars; ++i) {
        if(list->varIsArray[i] && !list->varIsReference[i]) {
            result += list->varIsArray[i];
        } else {
            ++result;
        }
    }
    
    return result;
}

int testForNeededReturn(parseToken *tok, interpreterRessources *ir) {
    if(tok->type != instructionSequence) {
        return 0;
    }
    
    parseToken *instruction = tok->subNodes[0];
    
    if(instruction->type == returnStatement) {
        return 1;
    } else if(instruction->type == repeatLoop) {
        parseToken *internalInstructions = instruction->subNodes[0];
        if(testForNeededReturn(internalInstructions, ir)) {
            return 1;
        }
    } else if(instruction->type == conditionalInstruction) {
        int thenReturn = testForNeededReturn(instruction->subNodes[1], ir);
        
        parseToken *elseSection = instruction->subNodes[2];
        int elseReturn = 1;
        
        if(elseSection->nNodes == 1) {
            elseReturn = testForNeededReturn(elseSection->subNodes[0], ir);
        }
        
        if(thenReturn && elseReturn) {
            return 1;
        }
    }
    
    if(tok->nNodes == 2) {
        return testForNeededReturn(tok->subNodes[1], ir);
    } else {
        return 0;
    }
}

char *getProcedure(parseToken *tok, interpreterRessources *ir) {
	if(tok->type != procedure) {
		ir->returnVal = 1;
		fprintf(stderr, "The token is not a procedure, but a %s!\n",
			   stringFromParseType(tok->type));
		return createStr();
	}

	char *name = tok->values[0].name;
	char *confirm = tok->values[1].name;

	if(strcmp(name, confirm) != 0) {
		ir->returnVal = 1;
		fprintf(stderr,
				"The name of the procedure (\"%s\") doesn't match the name at the end of the body (\"%s\")!\n",
				name, confirm);
		return createStr();
	}

	int function = resolveProcedureHeader(tok->subNodes[0], ir);

	if(ir->returnVal != 0) {
		return createStr();
	}
    
	int nr = ++(ir->nFunctions);
	functionDef **tmp = (functionDef **) realloc(ir->functions, nr * sizeof(functionDef*));

	if(tmp == NULL) {
		ir->returnVal = 1;
		fprintf(stderr, "Couldn't assign memory to create function %s!\n", name);
		--(ir->nFunctions);
		return createStr();
	}

	ir->functions = tmp;
    
	functionDef *def = createFunction(name, function);
	ir->functions[nr - 1] = def;
    
	parseParams(tok->subNodes[1], ir, def);
    
    internalFunctionVals* ifvs = getIFVs(def);
    ir->currentFunction = ifvs;
    
    varList *procVars = ifvs->internalVars;

    ifvs->sizeParams = getSizeOnStack(procVars);
    ifvs->nParams = procVars->nVars;
    
	parseVars(tok->subNodes[2], ir);

    ifvs->sizeVarsOnStack = getSizeOnStack(procVars) + 1;
    
    if(function) {
        int containsReturn = testForNeededReturn(tok->subNodes[3], ir);
        if(!containsReturn) {
            ir->returnVal = 1;
            fprintf(stderr, "The function %s doesn't return a value on every path!\n", name);
            return createStr();
        }
    }
    
    int markerSuccess = registerMarker(name, ir);
    
    char *endMarker = createStr();
    endMarker = catStrs(endMarker, name);
    endMarker = catStrs(endMarker, "$End");
    
    int endMarkerSuccess = registerMarker(endMarker, ir);
    
    if(!markerSuccess || !endMarkerSuccess) {
        free(endMarker);
        return createStr();
    }
    
    char *result = createStr();
    
    result = catStrs(result, name);
    result = catStrs(result, ":\n");
    
    int nrInternalVars = ifvs->sizeVarsOnStack - ifvs->sizeParams - 1;
    
    if(nrInternalVars > 0) {
        result = catStrs(result, "\tRSV\t\t");
        
        char *nr = itostr(nrInternalVars);
        result = catStrs(result, nr);
        free(nr);
        
        result = catStrs(result, "\n");
    }
    
    char *body = getInstructionSequence(tok->subNodes[3], ir);
    result = catStrs(result, body);
    free(body);
    
    result = catStrs(result, endMarker);
    result = catStrs(result, ":\n");
    
    if(nrInternalVars > 0) {
        result = catStrs(result, "\tREL\t\t$");
        
        char *nr = itostr(nrInternalVars);
        result = catStrs(result, nr);
        free(nr);
        
        result = catStrs(result, "\n");
    }
    
    result = catStrs(result, "\tRTS\n");
    
    ir->currentFunction = NULL;
    freeIFVs(ifvs);
    
	return result;
}

parseToken *getExpressionUnderlyingVarCall(parseToken *tok, interpreterRessources *ir) {
    if(tok->type != expression && tok->type != negation && tok->type != value) {
        return 0;
    }
    
    if(tok->type == expression && tok->nNodes == 1) {
        return getExpressionUnderlyingVarCall(tok->subNodes[0], ir);
    } else if(tok->type == value && tok->nNodes == 1) {
        if(tok->subNodes[0]->type == varCall) {
            return tok->subNodes[0];
        } else {
            return NULL;
        }
    } else {
        return NULL;
    }
}

char *parseCalledParam(parseToken *tok, interpreterRessources *ir, functionDef *func, int index) {
    if(tok->type != paramListCall) {
        ir->returnVal = 1;
        fprintf(stderr, "The token is not a paramListCall, but a %s!\n",
                stringFromParseType(tok->type));
        return createStr();
    }
    
    if((tok->nNodes == 0 && index > -1) || (tok->nNodes == 1 && index > 0)) {
        ir->returnVal = 1;
        fprintf(stderr, "Too few arguments when calling function %s!\n", func->name);
        return createStr();
    } else if (tok->nNodes > 0 && index < 0) {
        ir->returnVal = 1;
        fprintf(stderr, "Too much arguments when calling function %s!\n", func->name);
    }
    
    if(index == -1) {
        return createStr();
    }
    
    char *result;
    parseToken *expr;
    if(tok->nNodes == 2) {
        result = parseCalledParam(tok->subNodes[0], ir, func, index - 1);
        expr = tok->subNodes[1];
    } else {
        result = createStr();
        expr = tok->subNodes[0];
    }
    
    char *name = func->parameters->vars[index];
    int reference = func->parameters->varIsReference[index];
    int array = func->parameters->varIsArray[index];
    
    expressionType type = getExpressionType(expr, ir);
    parseToken *varCall = getExpressionUnderlyingVarCall(expr, ir);
    
    if(!reference && !array) {
        char *load = getExpression(expr, ir);
        result = catStrs(result, load);
        free(load);
        
        result = catStrs(result, "\tPUSH\n");
        registerPUSH(ir);
    }
    
    if(array) {
        if(type != array) {
            ir->returnVal = 1;
            free(result);
            fprintf(stderr, "The parameter \"%s\" of the function %s expects an array, but doesn't receive one!\n", name, func->name);
            return createStr();
        }
        if(!reference) {
            
            char *nameVar = varCall->values[0].name;
            
            int sizeVar = getArraySize(varCall, ir);
            
            if(sizeVar != array) {
                ir->returnVal = 1;
                fprintf(stderr, "An array with a size of %i (\"%s\") cannot be assigned to an array with the size of %i(\"%s\")!\n",
                        sizeVar, nameVar, array, name);
                return createStr();
            }
            
            result = catStrs(result, "\tRSV\t\t");
            char *sizeStr = itostr(array);
            result = catStrs(result, sizeStr);
            free(sizeStr);
            result = catStrs(result, "\n");
            registerPUSHNr(ir, array);
            
            if(resolveVarCall(varCall, ir) > 3) {
                nameVar = getLocalVarCall(varCall, ir);
            }
            
            result = catStrs(result, "\tLOAD\t$");
            result = catStrs(result, nameVar);
            result = catStrs(result, "\n\tPUSH\n");
            registerPUSH(ir);
            
            for(int i = 0; i < array; ++i) {
                if(i > 0) {
                    result = catStrs(result, "\tLOAD\t0(SP)\n");
                    result = catStrs(result, "\tADD\t\t$1\n");
                    result = catStrs(result, "\tSTORE\t0(SP)\n");
                }
                result = catStrs(result, "\tLOAD\t@0(SP)\n");
                result = catStrs(result, "\tSTORE\t");
                
                char *addressShift = itostr(i + 1);
                result = catStrs(result, addressShift);
                free(addressShift);
                
                result = catStrs(result, "(SP)\n");
            }
            
            result = catStrs(result, "\tREL\t\t$1\n");
            registerPULL(ir);
            
            return result;
        }
    }
    
    if(!array && type == array) {
        ir->returnVal = 1;
        free(result);
        fprintf(stderr, "The parameter \"%s\" of the function %s doesn't expect an array, but receives one!\n", name, func->name);
        return createStr();
    }
    
    if(reference) {
        if(varCall == NULL) {
            ir->returnVal = 1;
            free(result);
            fprintf(stderr, "The parameter \"%s\" of the function %s expects a variable, but doesn't receive one!\n", name, func->name);
            return createStr();
        }
        
        char *load = varAddressInSP(varCall, ir);
        result = catStrs(result, load);
        free(load);
    }
    
    return result;
}

char *getParamCall(parseToken *tok, interpreterRessources *ir, functionDef *func) {
    int index = func->parameters->nVars - 1;
    
    return parseCalledParam(tok, ir, func, index);
}

char *getProcedureCall(parseToken *tok, interpreterRessources *ir, int shouldBeFunction) {
    if(tok->type != procedureCall) {
        ir->returnVal = 1;
        fprintf(stderr, "The token is not a procedureCall, but a %s!\n",
                stringFromParseType(tok->type));
        return createStr();
    }
    
    char *name = tok->values[0].name;
    
    functionDef *func = NULL;
    
    for(int i = 0; i < ir->nFunctions; ++i) {
        functionDef *current = ir->functions[i];
        if(strcmp(current->name, name) == 0) {
            func = current;
        }
    }
    
    if(func == NULL) {
        ir->returnVal = 1;
        fprintf(stderr, "There doesn't exist any function or procedure with the name \"%s\"!\n", name);
        return createStr();
    }
    
    if(shouldBeFunction && !func->isFunction) {
        ir->returnVal = 1;
        fprintf(stderr, "The procedure %s doesn't return any value!\n", name);
        return createStr();
    }
    
    char *result = getParamCall(tok->subNodes[0], ir, func);
    
    result = catStrs(result, "\tJSR\t\t");
    result = catStrs(result, name);
    result = catStrs(result, "\n");
    result = catStrs(result, "\tREL\t\t$");
    result = catStrs(result, itostr(getSizeOnStack(func->parameters)));
    result = catStrs(result, "\n");
    registerPULLNr(ir, getSizeOnStack(func->parameters));
    
    return result;
}

char *getReturnStatement(parseToken *tok, interpreterRessources *ir) {
    int returnsValue = tok->nNodes == 1;
    int canReturnValue = 0;
    
    if(ir->currentFunction != NULL) {
        canReturnValue = ir->currentFunction->function->isFunction;
    }
    
    if(returnsValue && !canReturnValue) {
        ir->returnVal = 1;
        fprintf(stderr, "The procedure %s tries to return a value, but is a non-returning procedure!\n",
                ir->currentFunction->function->name);
        return createStr();
    }
    
    if(!returnsValue && canReturnValue) {
        ir->returnVal = 1;
        fprintf(stderr, "The function %s tries to return without a value, but is a returning function!\n",
                ir->currentFunction->function->name);
        return createStr();
    }
    
    char *result;
    
    if(returnsValue) {
        parseToken *expr = tok->subNodes[0];
        expressionType type = getExpressionType(expr, ir);
        
        if(type == array) {
            ir->returnVal = 1;
            fprintf(stderr, "A function can only return a single value, but \"%s\" tries to return an array!\n",
                    ir->currentFunction->function->name);
            return createStr();
        }
        
        result = getExpression(expr, ir);
    } else {
        result = createStr();
    }
    
    char *name;
    
    if(ir->currentFunction != NULL) {
        name = ir->currentFunction->function->name;
    } else {
        name = ir->name;
    }
    
    result = catStrs(result, "\tJMP\t\t");
    result = catStrs(result, name);
    result = catStrs(result, "$End\n");
    
    return result;
}

char *createFirstMarker(char *name, interpreterRessources *ir) {
	char *result = catStrs(createStr(), name);
	result = catStrs(result, "$Start");
	if(registerMarker(result, ir)) {
		result = catStrs(result, ":\n");
		return result;
	} else {
		free(result);
		return createStr();
	}
}

char *createHold(char *name, interpreterRessources *ir) {
	char *result = catStrs(createStr(), name);
	result = catStrs(result, "$End");
	if(registerMarker(result, ir)) {
		result = catStrs(result, ":\n");
		result = catStrs(result, "\tHOLD\n");
		return result;
	} else {
		free(result);
		return createStr();
	}

}

char *getBody(parseToken *tok, char *name, interpreterRessources *ir) {
	if(tok->type != body) {
		ir->returnVal = 1;
		fprintf(stderr, "The token is not a body, but a %s!\n",
				stringFromParseType(tok->type));
		return createStr();
	}
	if(strcmp(tok->values[0].name, name) != 0) {
		ir->returnVal = 1;
		fprintf(stderr,
			"The name of the program (\"%s\") doesn't match the name in the body (\"%s\")!\n",
				name, tok->values[0].name);
		return createStr();
	}
	
	char *result = createFirstMarker(name, ir);
	
	char *instructions = getInstructionSequence(tok->subNodes[0], ir);
	result = catStrs(result, instructions);
    char *hold = createHold(name, ir);
	result = catStrs(result, hold);
	free(instructions);
    free(hold);

	return result;
}

void getVarDeclarations(parseToken *tok, interpreterRessources *ir) {
	if(tok->type != varDeclarations) {
		ir->returnVal = 1;
		fprintf(stderr,"The token is not a varDeclarations, but a %s!\n",
				stringFromParseType(tok->type));
		return;
	}
	
	parseToken *var;

	if(tok->nNodes == 1) {
		var = tok->subNodes[0];
	} else {
		var = tok->subNodes[1];
		getVarDeclarations(tok->subNodes[0], ir);
	}

	if(var->type != varDeclaration) {
		ir->returnVal = 1;
		fprintf(stderr,"The token is not a varDeclaration, but a %s!\n",
			stringFromParseType(var->type));
		return;
	}
	
	int array = 0;
	char *name = var->values[0].name;
	int nr = 1;


	if(var->nVal == 2) {
		nr = var->values[1].value;
		array = nr;
	}

	if(nr < 1) {
		ir->returnVal = 1;
		fprintf(stderr,
			"The size of an array has to be bigger than 0, but the array \"%s\" has a size of %i!\n",
			name, nr);
		return;
	}

	registerVar(name, array, 0, ir, NULL);
}

void parseVars(parseToken *tok, interpreterRessources *ir) {
	if(tok->type != varSections) {
		ir->returnVal = 1;
		fprintf(stderr,"The token is not a varSections, but a %s!\n",
				stringFromParseType(tok->type));
		return;
	}
	if(tok->nNodes == 0) {
		return;
	}
	
	parseToken *subVarSection;

	if(tok->nNodes == 1) {
		subVarSection = tok->subNodes[0];
	} else {
		subVarSection = tok->subNodes[1];
		parseVars(tok->subNodes[0], ir);
	}

	if(subVarSection->type != varSection) {
		ir->returnVal = 1;
		fprintf(stderr,"The token is not a varSection, but a %s!\n",
			stringFromParseType(subVarSection->type));
		return;
	}
    
    getVarDeclarations(subVarSection->subNodes[0], ir);
}

char *getGlobalVarString(interpreterRessources *ir) {
	varList *vars = ir->vars;

	char *result = createStr();

	for(int i = 0; i < vars->nVars; ++i) {
		char *var = vars->vars[i];
		int array = vars->varIsArray[i];
		result = catStrs(result, var);
		result = catStrs(result, ":\n");

		if(!array) {
			++array;
		}

		for(; array > 0; --array) {
			result = catStrs(result, "\tWORD\t0\n");
		}
	}

	return result;
}

char *getInstructionSequence(parseToken *tok, interpreterRessources *ir) {
	if(tok->type != instructionSequence) {
		ir->returnVal = 1;
		fprintf(stderr, "The token is not an instructionSequence, but a %s!\n",
			stringFromParseType(tok->type));
		return createStr();
	}

	char *result = getInstruction(tok->subNodes[0], ir);

	if(tok->nNodes > 1) {
		char *prevInstructions = getInstructionSequence(tok->subNodes[1], ir);
		result = catStrs(result, prevInstructions);
		free(prevInstructions);
	}

	return result;
}

char *varAddressInSP(parseToken *tok, interpreterRessources *ir) {
	char *name = tok->values[0].name;
    
    if(varCallIsLocal(tok, ir)) {
        name = getLocalVarCall(tok, ir);
    }
    
    int isNoArray = tok->nNodes == 0;
    
    char *result = createStr();
    
    if(!isNoArray) {
        expressionType typeEx = getExpressionType(tok->subNodes[0], ir);
        
        isNoArray = typeEx == literalValue && getLiteralExpressionValue(tok->subNodes[0], ir) == 0;
        
        if(!(typeEx == exprFailure || typeEx == array)) {
            char *expression = getExpression(tok->subNodes[0], ir);
            result = catStrs(result, expression);
            free(expression);
        } else {
            if(typeEx == array) {
                ir->returnVal = 1;
                fprintf(stderr, "The index of the array \"%s\" cannot be a whole array itself!\n", name);
            }
            return result;
        }
    }

	if(isNoArray) {
		free(result);
		result = catStrs(createStr(), "\tLOAD\t$");
	} else {
		result = catStrs(result, "\tADD\t\t$");
	}

	result = catStrs(result, name);
	result = catStrs(result, "\n\tPUSH\n");
    registerPUSH(ir);

	return result;
}

char *getInternalAssignment(parseToken *var, char *allocation, interpreterRessources *ir) {
	varCallType leftType = collapseVCType(resolveVarCall(var, ir));

	if(leftType == callFailure) {
		return createStr();
	}
	
	char *name = var->values[0].name;
    
    if(varCallIsLocal(var, ir)) {
        name = getLocalVarCall(var, ir);
    }

	char *result;
	
	switch(leftType) {
		case 1:
			result = allocation;
			result = catStrs(result, "\tSTORE\t");
			result = catStrs(result, name);
			result = catStrs(result, "\n");
			return result;
		case 2:
			result = varAddressInSP(var, ir);
			result = catStrs(result, allocation);
			free(allocation);
			result = catStrs(result, "\tSTORE\t@0(SP)\n");
            result = catStrs(result, "\tREL\t\t$1\n");
            registerPULL(ir);
			return result;
		default:
			free(allocation);
			return createStr();
	}

}

int getArraySize(parseToken *tok, interpreterRessources *ir) {
    if(tok->type != varCall) {
        return 0;
    }
    
    char *name = tok->values[0].name;
    
    varCallType type = resolveVarCall(tok, ir);
    
    varList *list = ir->vars;
    
    if(type > 3) {
        list = ir->currentFunction->internalVars;
    }
    
    for(int i = 0; i < list->nVars; ++i) {
        if(strcmp(name, list->vars[i]) == 0) {
            return list->varIsArray[i];
        }
    }
    
    return 0;
}

char *assignArray(parseToken *left, parseToken *right, int size, interpreterRessources *ir) {
    char *result = createStr();
    
    char *nameLeft = left->values[0].name;
    if(resolveVarCall(left, ir) > 3) {
        nameLeft = getLocalVarCall(left, ir);
    }
    
    result = catStrs(result, "\tLOAD\t$");
    result = catStrs(result, nameLeft);
    result = catStrs(result, "\n\tPUSH\n");
    registerPUSH(ir);
    
    char *nameRight = right->values[0].name;
    if(resolveVarCall(right, ir) > 3) {
        nameRight = getLocalVarCall(right, ir);
    }
    
    result = catStrs(result, "\tLOAD\t$");
    result = catStrs(result, nameRight);
    result = catStrs(result, "\n\tPUSH\n");
    registerPUSH(ir);
    
    for(int i = 0; i < size; ++i) {
        if(i > 0) {
            result = catStrs(result, "\tLOAD\t0(SP)\n");
            result = catStrs(result, "\tADD\t\t$1\n");
            result = catStrs(result, "\tSTORE\t0(SP)\n");
            
            result = catStrs(result, "\tLOAD\t1(SP)\n");
            result = catStrs(result, "\tADD\t\t$1\n");
            result = catStrs(result, "\tSTORE\t1(SP)\n");
        }
        
        result = catStrs(result, "\tLOAD\t@0(SP)\n");
        result = catStrs(result, "\tSTORE\t@1(SP)\n");
    }
    
    result = catStrs(result, "\tREL\t\t$2\n");
    registerPULLNr(ir, 2);
    
    return result;
}

char *getAssignment(parseToken *tok, interpreterRessources *ir) {
	parseToken *var = tok->subNodes[0];
	parseToken *expr = tok->subNodes[1];

	varCallType leftType = collapseVCType(resolveVarCall(var, ir));
	expressionType rightType = getExpressionType(expr, ir);

	if(leftType == callFailure || rightType == exprFailure) {
		return createStr();
	}
	
	char *name = var->values[0].name;

	if(leftType == 3 && rightType != array) {
		ir->returnVal = 1;
		fprintf(stderr, "A single value cannot be assigned to the array \"%s\"!\n", name);
		return createStr();
	} else if(leftType != 3 && rightType == array) {
		ir->returnVal = 1;
		fprintf(stderr, "A whole array cannot be assigned to the single variable \"%s\"!\n", name);
		return createStr();
	}
    
    if(rightType == array) {
        parseToken *array =getExpressionUnderlyingVarCall(expr, ir);
        
        char *nameLeft = var->values[0].name;
        char *nameRight = array->values[0].name;
        
        int sizeRight = getArraySize(array, ir);
        int sizeLeft = getArraySize(var, ir);
        
        if(sizeRight != sizeLeft) {
            ir->returnVal = 1;
            fprintf(stderr, "An array with a size of %i (\"%s\") cannot be assigned to an array with the size of %i(\"%s\")!\n",
                    sizeRight, nameRight, sizeLeft, nameLeft);
            return createStr();
        }
        
        return assignArray(var, array, sizeLeft, ir);
    } else {
        char *exprStr = getExpression(expr, ir);

        return getInternalAssignment(var, exprStr, ir);
    }
}

char *getWhileLoop(parseToken *tok, interpreterRessources *ir) {
	if(tok->type != whileLoop) {
		ir->returnVal = 1;
		fprintf(stderr, "The token is not a whileLoop, but a %s!\n",
				stringFromParseType(tok->type));
		return createStr();
	}

	char *startMarker = getNumberedMarker(ir);
	char *instructions = getInstructionSequence(tok->subNodes[1], ir);

	char *endMarker = getNumberedMarker(ir);

	if(startMarker == NULL || endMarker == NULL) {
		free(instructions);
		if(startMarker != NULL) {
			free(startMarker);
		}
		if(endMarker != NULL) {
			free(endMarker);
		}
		return createStr();
	}

	char *result = catStrs(createStr(), startMarker);
	result = catStrs(result, ":\n");
	
	char *condition = getCondition(tok->subNodes[0], endMarker, ir, 0);
	result = catStrs(result, condition);
	free(condition);

	result = catStrs(result, instructions);
	free(instructions);

	result = catStrs(result, "\tJMP\t\t");
	result = catStrs(result, startMarker);
	result = catStrs(result, "\n");
	result = catStrs(result, endMarker);
	result = catStrs(result, ":\n");

	free(startMarker);
	free(endMarker);

	return result;
}

char *getConditionalInstruction(parseToken *tok, interpreterRessources *ir) {
	if(tok->type != conditionalInstruction) {
		ir->returnVal = 1;
		fprintf(stderr, "The token is not a conditionalInstruction, but a %s!\n",
				stringFromParseType(tok->type));
		return createStr();
	}

	int elseExists = tok->subNodes[2]->nNodes > 0;
	
	
	char *instructions = getInstructionSequence(tok->subNodes[1], ir);
	char *elseMarker = getNumberedMarker(ir);

    char *endMarker = NULL;
    char *elseSection = NULL;
	if(elseExists) {
		elseSection = getInstructionSequence(tok->subNodes[2]->subNodes[0], ir);
		endMarker = getNumberedMarker(ir);
	}

	if(elseMarker == NULL || (endMarker == NULL && elseExists)) {
		free(instructions);
		if(elseMarker != NULL) {
			free(elseMarker);
		}
		if(endMarker != NULL) {
			free(endMarker);
		}
		if(elseSection != NULL) {
			free(elseSection);
		}
		return createStr();
	}

	char *result = getCondition(tok->subNodes[0], elseMarker, ir, 0);

	result = catStrs(result, instructions);
	free(instructions);
    
    if(elseExists) {
        result = catStrs(result, "\tJMP\t\t");
        result = catStrs(result, endMarker);
        result = catStrs(result, "\n");
    }
    
	result = catStrs(result, elseMarker);
	result = catStrs(result, ":\n");
    free(elseMarker);

	if(elseExists) {
		result = catStrs(result, elseSection);
		free(elseSection);

		result = catStrs(result, endMarker);
		result = catStrs(result, ":\n");
        free(endMarker);
	}

	return result;
}

char *getRepeatLoop(parseToken *tok, interpreterRessources *ir){
	if(tok->type != repeatLoop) {
		ir->returnVal = 1;
		fprintf(stderr, "The token is not a repeatLoop, but a %s!\n",
				stringFromParseType(tok->type));
		return createStr();
	}

	char *startMarker = getNumberedMarker(ir);

	if(startMarker == NULL) {
		return createStr();
	}
	
	char *result = catStrs(createStr(), startMarker);
	result = catStrs(result, ":\n");

	char *instructions = getInstructionSequence(tok->subNodes[0], ir);
	result = catStrs(result, instructions);
	free(instructions);

	char *condition = getCondition(tok->subNodes[1], startMarker, ir, 0);
	result = catStrs(result, condition);
	free(condition);

	free(startMarker);
	return result;
}

void prepareSpecialExpression(parseToken *expression) {
	parseToken *valueTok = expression->subNodes[0];
	valueTok->nNodes = 0;
}

void freeSpecialAssignment(parseToken *assignment) {
	assignment->subNodes[0] = assignment->subNodes[1];
	assignment->nNodes = 1;
	freeToken(assignment);
}

char *getForLoop(parseToken *tok, interpreterRessources *ir) {
    if(tok->type != forLoop) {
        ir->returnVal = 1;
        fprintf(stderr, "The token ist not a forLoop, but a%s!\n",
                stringFromParseType(tok->type));
        return createStr();
    }

	parseToken *assignmentToken = tok->subNodes[0];
	parseToken *varCallToken = assignmentToken->subNodes[0];

	if(resolveVarCall(varCallToken, ir) == 3) {
		ir->returnVal = 1;
		fprintf(stderr, "The count var can't be a whole array!\n");
		return createStr();
	}

	parseToken *varExpression = createUnaryExpression(createValueByCall(varCallToken));
	char *result = getAssignment(assignmentToken, ir);

	parseToken *targetToken = tok->subNodes[1];
	char *target = getExpression(targetToken, ir);
	result = catStrs(result, target);
	free(target);
	result = catStrs(result, "\tPUSH\n");
    registerPUSH(ir);

	char *marker = getNumberedMarker(ir);
	if(marker == NULL) {
		free(result);
		return createStr();
	}

	result = catStrs(result, marker);
	result = catStrs(result, ":\n");

	char *varCall = getExpression(varExpression, ir);
	result = catStrs(result,varCall);
	
	result = catStrs(result, "\tCMP\t\t0(SP)\n");
	
	parseToken *iteration = tok->subNodes[2];
	parseToken *instructionSequence = tok->subNodes[3];

	char *instructions = getInstructionSequence(instructionSequence, ir);

	char *endMarker = getNumberedMarker(ir);

	if(endMarker == NULL) {
		free(result);
		free(instructions);
		return createStr();
	}

	int negative = 0;

	if(iteration->type != negativeAdvancement) {
		result = catStrs(result, "\tJMPP\t");
	} else {
		negative = 1;
		result = catStrs(result, "\tJMPN\t");
	}

	result = catStrs(result, endMarker);
	result = catStrs(result, "\n");

	result = catStrs(result, instructions);
	free(instructions);

	parseToken *rightPart = createUnaryExpression(createValue(iteration->values[0].value));
	parseToken *binaryExpression = createBinaryExpression(varExpression, negative, rightPart);

	char *exprStr = getExpression(binaryExpression, ir);
	exprStr = catStrs(exprStr, "\tJMPV\t");
	exprStr = catStrs(exprStr, endMarker);
	exprStr = catStrs(exprStr, "\n");

	char *iterationStr = getInternalAssignment(varCallToken, exprStr, ir);
	result = catStrs(result, iterationStr);
	free(iterationStr);

	result = catStrs(result, "\tJMP\t\t");
	result = catStrs(result, marker);
	result = catStrs(result, "\n");
	result = catStrs(result, endMarker);
	result = catStrs(result, ":\n");

	result = catStrs(result, "\tREL\t\t$1\n");
    registerPULL(ir);

	prepareSpecialExpression(varExpression);
	free(marker);
	free(endMarker);
	free(varCall);
	return result;
}

char *getInstruction(parseToken *tok, interpreterRessources *ir) {
	switch(tok->type) {
		case assignment:
			return getAssignment(tok, ir);
		case whileLoop:
			return getWhileLoop(tok, ir);
		case conditionalInstruction:
			return getConditionalInstruction(tok, ir);
		case repeatLoop:
			return getRepeatLoop(tok, ir);
		case forLoop:
			return getForLoop(tok, ir);
		case procedureCall:
            return getProcedureCall(tok, ir, 0);
		case returnStatement:
			return getReturnStatement(tok, ir);
		default:
			ir->returnVal = 1;
			fprintf(stderr, "The token is not an instruction, but a %s!\n",
					stringFromParseType(tok->type));
			return createStr();
	}
}

int isCommutative(parseToken *tok) {
	int operator = tok->values[0].value;
	return operator == 0 || operator == 2;
}

int canBeOnSP(parseToken *tok) {
	if((tok->type == expression && tok->nNodes == 1) || (tok->type == value && tok->nNodes > 0)) {
		return canBeOnSP(tok->subNodes[0]);
	}
	if(tok->type == arrayCall) {
		return 1;
	}
	return 0;
}

char *getOnSP(parseToken *tok, interpreterRessources *ir) {
	if((tok->type == expression && tok->nNodes == 1) || (tok->type == value && tok->nNodes > 0)) {
		return getOnSP(tok->subNodes[0], ir);
    }
    if(tok->type == arrayCall) {
    	return varAddressInSP(tok, ir);
    }
    return createStr();
}

char *loadSecondOperand(char *prev, parseToken *tok, interpreterRessources *ir) {
	expressionType type = getExpressionType(tok, ir);
	char *result;
	if(type == computedValue) {
		char *spStr;
		char *loader;
		if(canBeOnSP(tok)) {
			loader = getOnSP(tok, ir);
			spStr = catStrs(createStr(), "@");
		} else {
			loader = catStrs(getExpression(tok, ir), "\tPUSH\n");
            registerPUSH(ir);
			spStr = createStr();
		}
		
		spStr = catStrs(spStr, "0(SP)\n");

		result = catStrs(loader, prev);
		free(prev);
		result = catStrs(result, spStr);
		free(spStr);
		result = catStrs(result, "\tREL\t\t$1\n");
        registerPULL(ir);
	} else {
		char *expressionCall = getExpressionCall(tok, ir);
		result = catStrs(prev, expressionCall);
		free(expressionCall);
		result = catStrs(result, "\n");
	}

	return result;
}

parseToken *getWithoutNegation(parseToken *tok) {
	if(tok->type == negation) {
		tok->nNodes = 0;
		return tok->subNodes[0];
	}
	if(tok->type == expression && tok->nNodes == 1) {
		return getWithoutNegation(tok->subNodes[0]);
	}
	return NULL;
}

char *getBinaryExpression(parseToken *tok, interpreterRessources *ir) {
	parseToken *left = tok->subNodes[0];
	parseToken *right = tok->subNodes[1];
	int opCode = tok->values[0].value;
	expressionType leftType = getExpressionType(left, ir);
	expressionType rightType = getExpressionType(right, ir);
	
	char *result;

	if(leftType == computedValue) {
		result = getExpression(left, ir);
	} else {
		result = catStrs(createStr(), "\tLOAD\t");
		char *expressionCall = getExpressionCall(left, ir);
		result = catStrs(result, expressionCall);
		free(expressionCall);
		result = catStrs(result, "\n");
	}
 	
	if(opCode == 0 || opCode == 1) {
		parseToken *newRight = getWithoutNegation(right);
		if(newRight != NULL) {
			tok->values[0].value = 1 - opCode;
			tok->subNodes[1] = newRight;
			freeToken(right);
            free(result);
			return getBinaryExpression(tok, ir);
		}
	}

	char *operator = catStrs(createStr(), "\t");

	switch(opCode) {
		case 1:
			operator = catStrs(operator, "SUB");
			break;
		case 2:
			operator = catStrs(operator, "MUL");
			break;
		case 3:
			operator = catStrs(operator, "DIV");
			break;
		case 4:
			operator = catStrs(operator, "MOD");
			break;
		default:
			operator = catStrs(operator, "ADD");
	}

	operator = catStrs(operator, "\t\t");

	if(leftType != computedValue && rightType == computedValue && !canBeOnSP(right) && isCommutative(tok)) {
		free(result);
		result = getExpression(right, ir);
		result = catStrs(result, operator);
		char *expressionCall = getExpressionCall(left, ir);
		result = catStrs(result, expressionCall);
		free(expressionCall);
		result = catStrs(result, "\n");
		return result;
	}
	
	result = catStrs(result, operator);
	
	free(operator);

	result = loadSecondOperand(result, right, ir);

	return result;
}

char *getExpression(parseToken *tok, interpreterRessources *ir) {
	expressionType typeEx = getExpressionType(tok, ir);

	if(typeEx == exprFailure) {
		return createStr();
	}

	if(typeEx == literalValue || typeEx == singleValueVar) {
		char *result = catStrs(createStr(), "\tLOAD\t");
        char *expression = getExpressionCall(tok, ir);
		result = catStrs(result, expression);
        free(expression);
		result = catStrs(result, "\n");
		return result;
	}

	if(tok->nNodes == 1 && tok->type == expression) {
		return getExpression(tok->subNodes[0], ir);
	}

	if(tok->nNodes == 2) {
		return getBinaryExpression(tok, ir);
	}

	if(tok->type == negation) {
		char *result = catStrs(createStr(), "\tLOAD\t$0\n\tSUB\t");

		result = loadSecondOperand(result, tok->subNodes[0], ir);
		return result;
	}

	if(tok->type == value) {
		parseToken *call = tok->subNodes[0];
		if(call->type == arrayCall) {
			char *result = varAddressInSP(call, ir);
			result = catStrs(result, "\tLOAD\t@0(SP)\n");
			result = catStrs(result, "\tREL\t\t$1\n");
            registerPULL(ir);
			return result;
        } else if(call->type == procedureCall) {
            return getProcedureCall(call, ir, 1);
        }
	}

	return createStr();
}

int switchCondition(int opCode) {
	switch(opCode) {
		case 2:
			return 3;
		case 3:
			return 2;
		case 4:
			return 5;
		case 5:
			return 4;
		default:
			return opCode;
	}
}

char *getConditionInternal(parseToken *left, int opCode, parseToken *right, interpreterRessources *ir,
		int jumpIfTrue){
	expressionType leftType = getExpressionType(left, ir);

	char *result;

	if(leftType == computedValue) {
		result = getExpression(left, ir);
	} else {
		result = catStrs(createStr(), "\tLOAD\t");
		char *expressionCall = getExpressionCall(left, ir);
		result = catStrs(result, expressionCall);
		free(expressionCall);
		result = catStrs(result, "\n");
	}

	result = catStrs(result, "\tCMP\t\t");

	result = loadSecondOperand(result, right, ir);
	
	char *operator = catStrs(createStr(), "\t");
	char *operatorTrue = createStr();
	char *operatorFalse = createStr();

	switch(opCode) {
		case 1:
			operatorTrue = catStrs(operatorTrue, "JMPNZ");
			operatorFalse = catStrs(operatorFalse, "JMPZ");
			break;
		case 2:
			operatorTrue = catStrs(operatorTrue, "JMPN");
			operatorFalse = catStrs(operatorFalse, "JMPNN");
			break;
		case 3:
			operatorTrue = catStrs(operatorTrue, "JMPP");
			operatorFalse = catStrs(operatorFalse, "JMPNP");
			break;
		case 4:
			operatorTrue = catStrs(operatorTrue, "JMPNP");
			operatorFalse = catStrs(operatorFalse, "JMPP");
			break;
		case 5:
			operatorTrue = catStrs(operatorTrue, "JMPNN");
			operatorFalse = catStrs(operatorFalse, "JMPN");
			break;
		default:
			operatorTrue = catStrs(operatorTrue, "JMPZ");
			operatorFalse = catStrs(operatorFalse, "JMPNZ");
	}
	
	if(jumpIfTrue) {
		operator = catStrs(operator, operatorTrue);
	} else {
		operator = catStrs(operator, operatorFalse);
	}

	free(operatorTrue);
	free(operatorFalse);

	operator = catStrs(operator, "\t");

	result = catStrs(result, operator);
	free(operator);

	return result;
}

char *getCondition(parseToken *tok, char *dest, interpreterRessources *ir, int jumpIfTrue) {
	if(tok->type != condition) {
		ir->returnVal = 1;
		fprintf(stderr, "The token is not a condition, but a %s!\n",
				stringFromParseType(tok->type));
		return createStr();
	}

	parseToken *left = tok->subNodes[0];
	parseToken *right = tok->subNodes[1];
	int opCode = tok->values[0].value;

	expressionType leftType = getExpressionType(left, ir);
	expressionType rightType = getExpressionType(right, ir);
	
	if(leftType == exprFailure || rightType == exprFailure) {
		return createStr();
	}

	char *result;

	if(leftType != computedValue && rightType == computedValue && !canBeOnSP(right)) {
		result = getConditionInternal(right, switchCondition(opCode), left, ir, jumpIfTrue);
	} else {
		result = getConditionInternal(left, opCode, right, ir, jumpIfTrue);
	}

	result = catStrs(result, dest);
	result = catStrs(result, "\n");
	
	return result;
}

char *catStrs(char *target, const char *origin) {
	unsigned long newSize = strlen(target) + strlen(origin) + 1;
	char *tmp = (char *) realloc(target, newSize);
	if(tmp != NULL) {
		target = tmp;
		strcat(target, origin);
	}
	return target;
}

char *createStr() {
	char *result = (char *) malloc(1);
	result[0] = '\0';
	return result;
}

char *itostr(int val) {
    int lengthVal = snprintf(NULL, 0, "%i", val);

    char *result = malloc(lengthVal + 1);
    snprintf(result, lengthVal + 1, "%i", val);
    
    return result;
}

int registerMarker(char *name, interpreterRessources *ir) {
	for(int i = 0; i < ir->nMarkers; ++i) {
		if(strcmp(ir->markers[i], name) == 0) {
			ir->returnVal = 1;
			fprintf(stderr, "The marker with the name \"%s\" does already exist!\n", name);
			return 0;
		}
	}
	varList *vars = ir->vars;
	for(int i = 0; i < vars->nVars; ++i) {
		if(strcmp(vars->vars[i], name) == 0) {
			ir->returnVal = 1;
			fprintf(stderr,
				"The marker with the name \"%s\" does already exist in form of a variable!\n",
				name);
			return 0;
		}
	}
	
	int newNr = ++(ir->nMarkers);
	char **tmp = realloc(ir->markers, newNr* sizeof(char *));
	if(tmp != NULL) {
		ir->markers = tmp;
		ir->markers[newNr - 1] = transferStr(name);
		return 1;
	} else {
		ir->returnVal = 1;
		--(ir->nMarkers);
		fprintf(stderr, "Too many markers!\n");
		return 0;
	}
}

void registerPUSH(interpreterRessources *ir) {
    if(ir->currentFunction != NULL) {
        ir->currentFunction->sizeVarsOnStack += 1;
    }
}

void registerPULL(interpreterRessources *ir) {
    if(ir->currentFunction != NULL) {
        ir->currentFunction->sizeVarsOnStack -= 1;
    }
}

void registerPUSHNr(interpreterRessources *ir, int nr) {
    if(ir->currentFunction != NULL) {
        ir->currentFunction->sizeVarsOnStack += nr;
    }
}

void registerPULLNr(interpreterRessources *ir, int nr) {
    if(ir->currentFunction != NULL) {
        ir->currentFunction->sizeVarsOnStack -= nr;
    }
}

int registerVar(char *name, int array, int reference, interpreterRessources *ir, functionDef *useFunction) {
	varList *vars;
	
    varList *additionalVars = getAdditionalVarList(ir, 1);
    
    if(useFunction != NULL) {
        vars = useFunction->parameters;
    } else if(additionalVars != NULL) {
		vars = additionalVars;
	} else {
		vars = ir->vars;
        
        for(int i = 0; i < ir->nMarkers; ++i) {
            if(strcmp(ir->markers[i], name) == 0) {
                ir->returnVal = 1;
                fprintf(stderr,
                    "There exists already a marker named \"%s\", which forbids variables with that name!\n",
                    name);
                return 0;
            }
        }
	}
    
    for(int i = 0; i < vars->nVars; ++i) {
		if(strcmp(vars->vars[i], name) == 0) {
			ir->returnVal = 1;
			fprintf(stderr,
				"The variable with the name \"%s\" does already exist!\n",
				name);
			return 0;
		}
	}

	
	int newNr = ++(vars->nVars);
	char **tmp_1 = realloc(vars->vars, newNr * sizeof(char *));
	int *tmp_2 = realloc(vars->varIsArray, newNr* sizeof(int));
	int *tmp_3 = realloc(vars->varIsReference, newNr* sizeof(int));
	if(tmp_1 != NULL && tmp_2 != NULL && tmp_3 != NULL) {
		vars->vars = tmp_1;
		vars->varIsArray = tmp_2;
		vars->varIsReference = tmp_3;
		vars->vars[newNr - 1] = transferStr(name);
		vars->varIsArray[newNr - 1] = array;
        vars->varIsReference[newNr - 1] = reference;
		return 1;
	} else {
        if(tmp_1 != NULL) {
            vars->vars = tmp_1;
        }
        if(tmp_2 != NULL) {
            vars->varIsArray = tmp_2;
        }
		if(tmp_3 != NULL) {
			vars->varIsReference = tmp_3;
		}
		--(vars->nVars);
		ir->returnVal = 1;
		fprintf(stderr, "Too many vars!\n");
		return 0;
	}
}

char *getNumberedMarker(interpreterRessources *ir) {
	int nr = ++(ir->nGenericMarkers);

	char *result = catStrs(createStr(), "m$");

	char *nrStr = itostr(nr);

	result = catStrs(result, nrStr);
	free(nrStr);

	if(registerMarker(result, ir)) {
		return result;
	}

	free(result);

	return NULL;
}

varCallType resolveVarCall(parseToken *tok, interpreterRessources *ir) {
	int array;
	int index = -1;
	if(tok->type == varCall) {
		array = 0;
	} else if(tok->type == arrayCall) {
		array = 1;
		if(getExpressionType(tok->subNodes[0], ir) == literalValue) {
			index = getLiteralExpressionValue(tok->subNodes[0], ir);
		}
	} else {
		ir->returnVal = 1;
		fprintf(stderr, "The token is not a var- or arrayCall, but a %s!\n",
				stringFromParseType(tok->type));
		return 0;
	}


	char *name = tok->values[0].name;
	
	varList *vars;
    
    varList *additionalVars = NULL;
    
    if(ir->currentFunction != NULL) {
        additionalVars = ir->currentFunction->internalVars;
    }
    
    if (additionalVars != NULL) {
        vars = additionalVars;
        for(int i = 0; i < vars->nVars; ++i) {
            char *var = vars->vars[i];
            int varArray = vars->varIsArray[i];
            if(strcmp(var, name) == 0) {
                if(array == 1 && varArray == 0) {
                    ir->returnVal = 1;
                    fprintf(stderr, "Tried to access the variable \"%s\" as an array!\n", name);
                    return callFailure;
                } else if(array == 0 && varArray == 0) {
                    return singleValueLocal;
                } else if(array == 1 && varArray > 0) {
                    if(index == 0) {
                        return singleValueLocal;
                    } else {
                        return valueInArrayLocal;
                    }
                } else if(array == 0 && varArray > 0) {
                    return wholeArrayLocal;
                }
            }
        }
    }
    
    vars = ir->vars;
    
	for(int i = 0; i < vars->nVars; ++i) {
		char *var = vars->vars[i];
		int varArray = vars->varIsArray[i];
		if(strcmp(var, name) == 0) {
			if(array == 1 && varArray == 0) {
				ir->returnVal = 1;
				fprintf(stderr, "Tried to access the variable \"%s\" as an array!\n", name);
				return callFailure;
			} else if(array == 0 && varArray == 0) {
				return singleValue;
			} else if(array == 1 && varArray > 0) {
				if(index == 0) {
					return singleValue;
				} else {
					return valueInArray;
				}
			} else if(array == 0 && varArray > 0) {
				return wholeArray;
			}
		}
	}
	ir->returnVal = 1;
	fprintf(stderr, "Tried to access the non-existing variable \"%s\"!\n", name);
	return callFailure;
}

varCallType collapseVCType(varCallType type) {
    if(type > 3) {
        return type - 3;
    }
    
    return type;
}

int varCallIsLocal(parseToken *tok, interpreterRessources *ir) {
    varCallType type = resolveVarCall(tok, ir);
    if(type > 3) {
        return 1;
    }
    return 0;
}

char *getLocalVarCall(parseToken *tok, interpreterRessources *ir) {
    internalFunctionVals *func = ir->currentFunction;
    if(func == NULL) {
        return createStr();
    }
    
    int remaining = func->sizeVarsOnStack;
    int offset = 0;
    int finalOffset = 0;
    varList *list = func->internalVars;
    char *name = tok->values[0].name;
    
    for(int i = list->nVars - 1; i >= 0; --i) {
        if(strcmp(list->vars[i], name) != 0) {
            offset += list->varIsArray[i];
            
            if(list->varIsArray[i] < 1) {
                ++offset;
            }
        } else {
            finalOffset = offset;
            
            if(i >= func->nParams) {
                --finalOffset;
            }
        }
        
        remaining -= list->varIsArray[i];
        
        if(list->varIsArray[i] < 1) {
            --remaining;
        }
    }
    
    finalOffset += remaining;
    
    char *result = catStrs(createStr(), itostr(finalOffset));
    result = catStrs(result, "(SP)");
    
    return result;
}

expressionType varCallToExpressionType(varCallType type) {
	switch(collapseVCType(type)) {
		case 1:
			return singleValueVar;
		case 2:
			return computedValue;
		case 3:
			return array;
		default:
			return exprFailure;
	}
}

expressionType combineExpressionTypes(expressionType leftType, expressionType rightType) {
	if(leftType == exprFailure || rightType == exprFailure) {
		return exprFailure;
	}

	if(leftType == literalValue && rightType == literalValue) {
		return literalValue;
	}

	return computedValue;
}

expressionType getExpressionType(parseToken *tok, interpreterRessources *ir) {
	if(tok->type != expression && tok->type != negation && tok->type != value) {
		ir->returnVal = 1;
		fprintf(stderr, "The token is not an expression, negation or value, but a %s!\n",
				stringFromParseType(tok->type));
		return exprFailure;
	}
	
	if(tok->type == expression && tok->nNodes == 1) {
		return getExpressionType(tok->subNodes[0], ir);
	} else if(tok->type == negation) {
		expressionType type = getExpressionType(tok->subNodes[0], ir);
		if(type == array) {
			ir->returnVal = 1;
			fprintf(stderr, "A whole array can't be negated!\n");
			return exprFailure;
		}
		if(type == singleValueVar) {
			return computedValue;
		}
		return type;
	} else if (tok->type == expression && tok->nNodes == 2) {
		expressionType leftType = getExpressionType(tok->subNodes[0], ir);
		expressionType rightType = getExpressionType(tok->subNodes[1], ir);
		if(leftType == array || rightType == array) {
			ir->returnVal = 1;
			fprintf(stderr, "A whole array can't be used in a operation!\n");
			return exprFailure;
		}
		return combineExpressionTypes(leftType, rightType);

	} else {
		if(tok->nVal == 1) {
			return literalValue;
		}
		if(tok->subNodes[0]->type == procedureCall) {
			return computedValue;
		}
		varCallType type = resolveVarCall(tok->subNodes[0], ir);
		return varCallToExpressionType(type);
	}
}

int getRecursiveExpressionValue(parseToken *tok) {
	switch(tok->type) {
		case negation:
			return -1 * getRecursiveExpressionValue(tok->subNodes[0]);
		case value:
			return tok->values[0].value;
		default:
			if(tok->nNodes == 2) {
				parseToken *leftSide = tok->subNodes[0];
				parseToken *rightSide = tok->subNodes[1];
				int leftValue = getRecursiveExpressionValue(leftSide);
				int rightValue = getRecursiveExpressionValue(rightSide);
				int operator = tok->values[0].value;
				switch(operator) {
					case 1:
						return leftValue - rightValue;
					case 2:
						return leftValue * rightValue;
					case 3:
						return leftValue / rightValue;
					case 4:
						return leftValue % rightValue;
					default:
						return leftValue + rightValue;
				}
			} else {
				return getRecursiveExpressionValue(tok->subNodes[0]);
			}
	}
}

int getLiteralExpressionValue(parseToken *tok, interpreterRessources *ir) {
	if(getExpressionType(tok, ir) == literalValue) {
		return getRecursiveExpressionValue(tok);
	}
	return 0;
}

char *getRecursiveExpressionCall(parseToken *tok, interpreterRessources *ir) {
	if(tok->type == expression || tok->type == value) {
		return getRecursiveExpressionCall(tok->subNodes[0], ir);
	}
	if(tok->type == varCall) {
        if(varCallIsLocal(tok, ir)) {
            return getLocalVarCall(tok, ir);
        }
		return catStrs(createStr(), tok->values[0].name);
	}
	if(tok->type == arrayCall && tok->values[1].value == 0) {
        if(varCallIsLocal(tok, ir)) {
            return getLocalVarCall(tok, ir);
        }
		return catStrs(createStr(), tok->values[0].name);
	}
	return createStr();
}

char *getExpressionCall(parseToken *tok, interpreterRessources *ir) {
	expressionType type = getExpressionType(tok, ir);

	if(type == exprFailure) {
		return createStr();
	}

	if(type == literalValue) {
		int nr = getLiteralExpressionValue(tok, ir);
        
        char *nrStr = itostr(nr);
		
		char *result = catStrs(createStr(), "$");
		result = catStrs(result, nrStr);
		free(nrStr);

		return result;
	} else if(type == singleValueVar) {
		return getRecursiveExpressionCall(tok, ir);
	}
	return createStr();
}


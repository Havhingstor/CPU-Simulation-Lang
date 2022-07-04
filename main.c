//
//  main.c
//  CPU-Simulation-Lang
//
//  Created by Paul on 16.05.22.
//

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "parsetree.h"
#include "interpreter.h"

void printTabs(int indent) {
    int i = indent;
    while(i > 0) {
        printf("  ");
        --i;
    }
}

void printInfo(parseToken *programToken, int indent) {
    printTabs(indent);
    printf("%s\n\n", stringFromParseType(programToken->type));
    for(int i = 0; i < programToken->nVal; ++i) {

		if(programToken->valueTypes[i] == string){
			printTabs(indent);
 	    	printf("%s\n",programToken->values[i].name);
        } else {
			printTabs(indent);
        	printf("%i\n",programToken->values[i].value);
		}
    }
	if(programToken->nVal > 0) {
		printf("\n");
	}
    for(int i = 0; i < programToken->nNodes; ++i) {
        printInfo(programToken->subNodes[i], indent + 1);
    }
	if(programToken->nNodes > 0) {
		printf("\n");
	}
}

int handle(parseToken *programToken, int success) {
    if(success == 0) {
       	//printInfo(programToken, 0);
		char *assembly = createAssembly(programToken, &success);
		if(success == 0) {
			printf("%s", assembly);
			fprintf(stderr, "Successfully parsed!\n");
		}

		free(assembly);
    }
	
    freeToken(programToken);
    
    return success;
}

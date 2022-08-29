/*#include "TwoRuns.c"

#include "MacroEliminate.c"

#include "ErrorChecks.c"

#include "ConvertTo32.c"

#include "SupportFunctions.c"*/

#include "header.h"
/*calls preProcessor and firstIteration function. firstIteration calls secondIteration*/

int main(int argc, char* argv[]) {

    /*initiate arrays*/

    instruction* instructionsArray;

    data* dataArray;

    label* labelsArray;

    /* iterates over filenames supplied*/

    int counter = 1;

    while (counter <= argc-1) {
    	
    	instructionsArray = malloc(sizeof(instruction)); dataArray = malloc(sizeof(data)); labelsArray = malloc(sizeof(label));

        printf("filename: %s\n", argv[counter]);

        if (callPreProcessor(argv[counter])) {
        	printf("preProcessor succeeded\n");

            firstIteration(argv[counter], &instructionsArray, &dataArray, &labelsArray); /*filename parameter should come from args!*/

            /*second iteration is called from within first iteration*/

        } free(instructionsArray); free(dataArray); free(labelsArray);

        printf("__________________________________________________________________________________\n");

        ++counter;

    }

    printf("finished iterating over files \n");
    exit(ZERO);


}




#include "header.h"

/*First Run: Parse over each line in file.am and save contents of line into individual fields. If errorCheck clears with no errors, save 
line contents in instructionsArray or dataArray. These arrays represent the two main separate memory sections.
Base32 codes for memory are calculated during the saving of line contents in applicable arrays.
Second Run: if firstIteration passed smoothly with no errors, iterate over instructionsArray and fill in base32 code for labels, using labelsArray defined
in firstIteration.
Finally, create .obj file, .ext file and .ent file*/

 void firstIteration(char* filename, instruction** instructionsArray, data** dataArray, label** labelsArray) { 

    /*variables for field separation*/
    char* label_name = malloc(MAX_STR_SIZE);
    char* data_type = malloc(MAX_STR_SIZE);
    char* instruction_name = malloc(MAX_STR_SIZE);
    char* operands;
    char* temp = malloc(MAX_STR_SIZE); /*temporarily stores str for strtok manipulation*/
    char* token = temp; 


    /*array counters*/
    int IC = 0; /*instruction counter*/
    int DC = 0; /*data counter*/
    int L = 0; /*labels array counter*/
    int address = BASE_ADDRESS; /*counts address of each line in memory */
    int line_counter = 0; /*counts line numbers in file*/
    
    int size; /*stores amount of words each sentence should take in memory*/
    data* data_realloc;
    instruction* instruction_realloc;
    label* labels_realloc;
    int errorsFound = 0;

    /*read from example.am*/
    char* filename_am = makeNewFilename(filename, ".am");
    char str[MAX_STR_SIZE];
    FILE* ptr = fopen(filename_am, "r");/*Opening file in reading mode*/
    if (NULL == ptr) {
        printf("file can't be opened \n");
        return; 
    }

    while (fgets(str, MAX_STR_SIZE, ptr) != NULL) {
        ++line_counter;
      
        /*printf("sentence: %s", str);*/

        if (str[0] == '\n' || str[0] == ';') { /*empty or comment line */
            continue; 
        }
        
        /*save separate fields into variables*/
        strcpy(temp, str);
        token = strtok(temp, " \t\r"); /*get first word*/
        if (strchr(token, CHAR_COLON)) {/*first word is label*/
            strcpy(label_name, token);
            label_name[strchr(label_name, CHAR_COLON) - label_name] = NULL_CHAR; /*remove CHAR_COLON from label name*/
            token = strtok(NULL, " \t\r"); /*token now holds second word*/
        }
        else {
            *label_name = NULL_CHAR;
        }
        if (token){
		    /*first word is data type or instruction name*/
		    if (strchr(token, '.')) {/*word is data type*/
		        strcpy(data_type, token);
		        if (strchr(data_type, '\n')) {
		            (strchr(data_type, '\n'))[0] = NULL_CHAR;
		        }
		        *instruction_name = NULL_CHAR;
		    }
		    else {/*word is instruction name */
		        strcpy(instruction_name, token);
		        if (*instruction_name == '\n' && (!label_name || *label_name == '\0')) {
		            continue; /*empty line*/
		        }
		        *data_type = NULL_CHAR;
		        if (strchr(instruction_name, '\n')) {
		            (strchr(instruction_name, '\n'))[0] = NULL_CHAR;
		        }
		    }
		    operands = strtok(NULL, "");
		    /*clean up whitespace in beginning of operands*/
		    while (operands && !isalnum(operands[0]) && operands[0] != '-' && operands[0] != '+' && operands[0] != '#' && operands[0] != '\"' && operands[0] != '\n' && operands[0] != ','){
		        operands++;
		    }
		    /*remove '\n'from token*/
		    if (operands && strchr(operands, '\n')) { 
		        (strchr(operands, '\n'))[0] = NULL_CHAR;
		    }
        }
        else{ /*rest of sentence is empty*/
        	*instruction_name = NULL_CHAR;
        	*data_type = NULL_CHAR;
        	*operands = NULL_CHAR;
        }
        /*check for errors in current line. If no errors have been found yet in this file, set errorsFound variable to the result of the errorCheck.*/
        if (!errorsFound){ 
        	errorsFound = ErrorChecker(label_name, labelsArray, L, instruction_name, data_type, operands, line_counter);
        }else{
        	ErrorChecker(label_name, labelsArray, L, instruction_name, data_type, operands, line_counter);
        }
        
        if (!errorsFound) { /*save in memory only if no errors were found*/
            /*save label in labels array*/
            if (*label_name != NULL_CHAR || !strcmp(data_type, ".entry") || !strcmp(data_type, ".extern")){
                if (*label_name == NULL_CHAR) {
                    strcpy(label_name, str);
                }
                saveInLabelsArray(labelsArray, label_name, address, L, data_type);
                ++L; /*L is index of next empty cell*/
                labels_realloc = realloc(*labelsArray, (L+1) * sizeof(label));
                if (!labels_realloc) {
                    printf("realloc failed\n"); 
                    exit(1);
                }*labelsArray = labels_realloc;
            }

            /*save instruction sentence or data sentence in appropriate array*/
            size = calculateSize(str, data_type, instruction_name, operands);
            if (*data_type != NULL_CHAR) { /*data sentence*/
                /*don't save anything if this is .extern or .entry*/
                if (!strcmp(data_type, ".extern") || (!strcmp(data_type, ".entry"))) {
                    continue;
                }
                /*increase space in data array for new words*/
                data_realloc = realloc(*dataArray, (DC + size) * sizeof(data)); 
                if (!data_realloc) {
                    printf("realloc failed\n");
                    exit(1);
                } *dataArray = data_realloc;
                /*save type , contents and index */
                saveInDataArray(dataArray, operands, data_type, DC, size-1, address);
                DC += size; 
                address += size;
            }
            else if (*instruction_name != NULL_CHAR) {/*instruction sentence*/
                /*increase space in instructions array for new words*/
                size += 1; /*instruction sentences have an extra word in memory - first word */
                instruction_realloc = realloc(*instructionsArray, ((IC + size) * sizeof(instruction))); 

                if (!instruction_realloc) {
                    printf("realloc failed\n");
                    exit(1);
                } *(instructionsArray) = instruction_realloc;
                /*save name of instruction and contents of word*/

                saveInInstructionsArray(instructionsArray, str, operands, IC, size-1, address, instruction_name);
                IC += size;
                address += size; 
            }
        }
    }
   
    free(temp); fclose(ptr);
    if (label_name){free(label_name);}
	if (data_type){free(data_type); }
	if (instruction_name){free(instruction_name);}
    
    /*second run - check if no errors were found in first run, then check if no errors occurred during secondIteration (filling up final base 32 codes)
    , only then save in .ent and .obj. The .ext file is created separately directly from secondIteration*/
    if (!errorsFound && secondIteration(instructionsArray, labelsArray, IC, DC, L, filename)) {
    	printf("no errors found in first Iteration and second Iteration succeeded. \n");
        createEntFile(labelsArray, L, filename);
    	createObjFile(instructionsArray, dataArray, IC, DC, filename);
    }
    
}
/*Functions for saving words in arrays */
 int calculateSize(char* sentence, char* data_type, char* instruction_name, char* operands) {
    /*counts the amount of EXTRA blocks needed for instructions according to their sentence type. 
    The size returned does NOT include the first word saving the contents of the actual instruction*/
    int size;
    int count = 0;
    char* temp_operands = malloc(MAX_STR_SIZE); /*will store operands so as not to mess up original pointer*/
    char* token = temp_operands; /*will run strtok on this*/
    int r_flag = 0; /*flag that turns on when one of the operands is a register*/

    if (!operands) {
        /*printf("no operands supplied to size calculation!\n");*/
        return 0;
    }

    /*if sentence is data type*/
    if (*data_type != NULL_CHAR) {
        sentence = strchr(sentence, '.'); /*assumes that sentence is definitely data type so strchr is valid*/
        size = sizeOfDataSentence(sentence);
        free(temp_operands);
        return size;
    }

    /*if sentence is instruction*/
    else if (*instruction_name != NULL_CHAR && temp_operands) {
        /*get operands  */
        strcpy(temp_operands, operands);
        token = strtok(token, " \t\r,");

        while (token) {
            if (strchr(token, 'r')) {/*if one of the operands is a register*/ 
                if (r_flag == 1) { /*previous operand was also a register*/
                    return 1; /*two register operands - one word*/
                }
                r_flag = 1; /*else set register flag to be 1*/
            }
            count += sizeOfAddressingType(getAddressingType(token));
            token = strtok(NULL, " \t\r,");
        }
        free(temp_operands);
        return count;
    }

    else { /*empty or comment sentence*/
        free(temp_operands);
        return 0;
    }
}

/*Functions for saving data sentences*/
 int getAddressingType(char* operand) {
    /* this function returns addressing type for instruction sentences*/
    /*three types of operands - #number, r1-9 register, label. check first two, if not them, then its a label*/
    char c; /*will store first letter of operands*/
    char* p = operand; /*used in strchr to search for '.'*/
    int r;
    

    if (!operand) {
        /*printf("operand %s doesn't exist\n", operand);*/
        return -1;
    }
    /*strcpy(p, operand);*/
    c = operand[0];
    if (c == CHAR_HASH) {
        r = 0;
    }
    else if (c == 'r' && strlen(operand) == 2 && isdigit(operand[1])) {
        r = 3; 
    }
    else {
        /*operand is a label - either with index or without.*/
        /* we need to find a '.' to check type*/
        if (!strchr(p, CHAR_DOT)) {
        	if (!strIsAlnum(operand)){
        		r = -1;
        	}else{
		        r = 1;
        	}
        }
        else {
		    r = 2;
        }
    }
    return r;

}

 int sizeOfAddressingType(int type) {
    if (type < 0 || type>3) {
        printf("ERROR: not possible addressing type\n");
        return -1;
    }
    switch (type) {
    case 2:
        return 2;
    default:
        return 1;
    }

}

 int sizeOfDataSentence(char* sentence) {
    /*sentence is without label name*/
    char* temp = malloc(MAX_STR_SIZE);
    char* data_type;
    char* token = temp;
    int count = 0;
    if (temp) {
        strcpy(temp, sentence);
    }

    data_type = strtok(temp, " \t\r\n");
    if (!strcmp(data_type, ".struct")) {
        count = 1;
        token = strtok(NULL, " \t\r,\n"); /*get number*/
        token = strtok(NULL, " \t\r,\t\r\n"); /*get string*/
        count += (int)(strlen(token) - 1);  /*string length needs to include NULL_CHAR*/
        

    }
    else if (!strcmp(data_type, ".data")) {
        token = strtok(NULL, " \t\r,\n");
        while (token) {
            token = strtok(NULL, " \t\r,\n");
            ++count;
        }
    }
    else if (!strcmp(data_type, ".string")) {
        token = strtok(NULL, " \t\r\n");
        count = (int)(strlen(token) - 1);
    }
    else if (!strcmp(data_type, ".extern") || (!strcmp(data_type, ".entry"))) { /*no memory saved for .extern or .entry sentences*/
        count = 0;
    }

    else {
        printf("ERROR: Not an existing data type!");
    }
    free(temp);
    return count;
}

 void saveInDataArray(data** dataArray, char* contents, char* data_type, int DC, int size, int address) 
{ /*saves line in data array
  size refers to the amount of blocks applicable*/
    
    char* temp_contents = malloc(MAX_STR_SIZE);
    char* token = temp_contents; /*used for strtok with contents*/
    int index;
    index = 0;

    if (!strcmp(data_type, ".string")) { /*remove "" from contents*/
        contents += 1;
        contents[6] = NULL_CHAR;
    }

    /*save contents of individual data items*/
    while (index <= size) {
        /*save invidual contents*/
        if (!strcmp(data_type, ".string")) {
            dataArray[0][DC + index].contents[0] = contents[index];
            strcpy(dataArray[0][DC + index].contents + 1, ""); /*empty the rest of the contents array*/
        }

        else if (!strcmp(data_type, ".data")) {
            /*get first number from contents*/
            if (index == 0 && token) { /*first iteration of while loop*/
                strcpy(temp_contents, contents);
                token = strtok(token, " /t/r,"); 
            }
            else {
                token = strtok(NULL, " /t/r,");
            }
            if (strlen(token) == 1) {
                dataArray[0][DC + index].contents[0] = *token;
                strcpy(dataArray[0][DC + index].contents + 1, ""); /*empty the rest of the contents array*/
            }
            else {
                strcpy(dataArray[ZERO][DC + index].contents, token);
            }
        }

        else if (!strcmp(data_type, ".struct") && token) {
            if (index == ZERO) {/*first iteration - save number*/
                strcpy(temp_contents, contents);
                token = strtok(token, " /t/r, /t/r");
                if (strlen(token) == 1) {
                    dataArray[ZERO][DC + index].contents[ZERO] = *token;
                    strcpy(dataArray[ZERO][DC + index].contents + 1, ""); /*empty the rest of the contents array*/
                }
                else {
                    strcpy(dataArray[ZERO][DC + index].contents, token);
                }
            }
            else {
                if (index == 3) { /*save '\0' in binary */
                    strcpy(dataArray[ZERO][DC + index].contents, "");
                }
                else {
                    token = (strchr(contents, '"') + 1); /*pointer that holds the string*/
                    dataArray[ZERO][DC + index].contents[ZERO] = token[index-1];
                    strcpy(dataArray[ZERO][DC + index].contents + 1, "");
                }
            }
        }
        strcpy(dataArray[ZERO][DC+index].type, data_type);
        dataArray[ZERO][DC+index].index = index;
        saveBase32ContentsInDataArray(dataArray, data_type, DC + index);
        /*strcpy(dataArray[DC+index].base32, ""); initialise all base32 fields to be 0, later this will be sorted using base32 tools*/
        dataArray[ZERO][DC + index].address = address + index;
        /*printDataArray(*dataArray, DC+index);*/
        ++index;
    }
    free(temp_contents);
}

 void printDataArray(data* dataArray, int index) {
    printf("address: %d\n", dataArray[index].address);
    printf("contents: %s\n", dataArray[index].contents);
    printf("data type: %s\n", dataArray[index].type);
    printf("in base32: %s\n\n", dataArray[index].base32);
}

/*Functions for saving instruction sentences*/
 void saveInInstructionsArray(instruction** instructionsArray, char str[MAX_STR_SIZE], char* operands, int IC, int size, int address, char instruction[3]) {
    /*saves given instruction in instructions array - str stores the whole instruction, operands are individual operands in the instruction
    IC is the index of the next available block and size is the number of blocks to create NOT including the first word*/
    int index = ZERO; /*will iterate until size*/
    char* temp_operands = malloc(MAX_STR_SIZE); /*move operands here */
    char* token = temp_operands; /*save individual operands here*/
    char* temp2 = malloc(MAX_STR_SIZE); /*used for second strtok in struct addressing mode */
    int addressingType;

    /*save first word*/
    
    strcpy(instructionsArray[ZERO][IC+ index].name, instruction);
    if (!operands) {
        strcpy(instructionsArray[ZERO][IC+ index].operands, "");
    }
    else {
        strcpy(instructionsArray[ZERO][IC+ index].operands, operands);
    }
    saveBase32InstructionArray(instructionsArray, IC, index, instruction);
    instructionsArray[ZERO][IC + index].address = address;
    index = 1;

    /*save rest of words*/
    if (!operands) {
        return;
    }
    if (temp_operands) {
        strcpy(temp_operands, operands);
    }
    token = strtok(token, " \t\r,"); /*split using ',' delimeter*/

    
    while (index <= size) {
        strcpy(instructionsArray[ZERO][IC + index].name, instruction);/*save whole instruction in each word*/
        strcpy(instructionsArray[ZERO][IC+ index].base32, "");
        instructionsArray[ZERO][IC + index].address = address + index;

        /*separate operands and use getAddressingType to save individual operands*/
        addressingType = getAddressingType(token);
        if (addressingType == 2 && temp2 && token && temp_operands) { /*struct addressing type*/
            /*first word - saves actual label*/
            strcpy(temp2, token);
            temp2 = strtok(temp2, "."); /*run strtok on temp2 only once so we can still free it*/
            strcpy(instructionsArray[ZERO][IC + index].operands, temp2);
            strcpy(instructionsArray[ZERO][IC + index].base32, ""); /*no code for base32 for a label at this stage*/
            ++index;
            /*save label index*/
            strcpy(instructionsArray[ZERO][IC + index].operands, strchr(token, '.')+1);
            strcpy(instructionsArray[ZERO][IC + index].name, instruction);
            strcpy(instructionsArray[ZERO][IC + index].base32, "");
            instructionsArray[ZERO][IC + index].address = address + index;


            /*next operand:*/
            strcpy(temp_operands, operands);
            token = temp_operands;
            token = strtok(token, "\t\r,"); /*strtok needs different first value*/
            token = strtok(NULL, "\t\r, \t\r"); 
            
            

        }
        else { /*addressing type - immediate, direct label or register*/
            if (addressingType == ZERO && token) { /*immediate addressing type*/
                token = strchr(token, CHAR_HASH) + 1;
            }
            if (token) {
                strcpy(instructionsArray[ZERO][IC + index].operands, token);
            }
            
            /*next operand:*/
            token = strtok(NULL, "\t\r, \t\r");
           
        }
        if (token && index == 1 && getAddressingType(token) == 3) { /*next operand is also register*/
            strcpy(instructionsArray[ZERO][IC + index].operands, operands); /*save both registers as operands*/
        }

        saveBase32InstructionArray(instructionsArray, IC, index, instruction);
        ++index;
    }

    free(temp_operands); free(temp2);
    temp_operands = NULL; temp2 = NULL;
    
    

}

 void printInstructionsArray(instruction* instructionsArary, int index) {
    printf("address: %d\n", instructionsArary[index].address);
    printf("name: %s\n", instructionsArary[index].name);
    printf("operands: %s\n", instructionsArary[index].operands);
    printf("base32: %s\n\n", instructionsArary[index].base32);

}

 void saveInLabelsArray(label** labelsArray, char* label_name, int address, int L, char* data_type){
    /*save label_name in labels array.
    L is index of LabelsArray
    address if memory address for label
    in second iteration - we will use address to make sure that the correct label contents will be saved*/
    char* temp_labelname = malloc(MAX_STR_SIZE);
    char* token = temp_labelname;

    
    if ((!strcmp(data_type, ".extern") || (!strcmp(data_type, ".entry"))) && temp_labelname) {
        strcpy(labelsArray[ZERO][L].type, data_type);
        labelsArray[ZERO][L].address = -1; /*correct address will be calculated in second run*/
        strcpy(temp_labelname, label_name);
        token = strtok(token, " \t\r");
        token = strtok(NULL, " \t\r");
        if (strchr(token, '\n')) {
            (strchr(token, '\n'))[ZERO] = NULL_CHAR;
        }
        strcpy(labelsArray[ZERO][L].name, token);
    }
    else {
        strcpy(labelsArray[ZERO][L].name, label_name);
        labelsArray[ZERO][L].address = address;
        strcpy(labelsArray[ZERO][L].type, "");
    }

    free(temp_labelname);
}




 int secondIteration(instruction** instructionsArray, label** labelsArray, int IC, int DC, int L, char* filename) {
    /*filling labels 32 code in instructions array. Returns 0 if encounters error*/
    int index; 
    char* prev_name = calloc(MAX_STR_SIZE, 1); /*save name of each word in order to compare to know when line from original file changes */
    char* operand = malloc(MAX_STR_SIZE); /*saves operand from memory (label name)*/
    int address;
    FILE* ptr; /* stores .ext file*/
    int created_ext = 0;
    if (operand && prev_name) {
        for (index = ZERO; index <= IC - 1; index++) {
            if (strcmp(instructionsArray[ZERO][index].name, prev_name)) {
                strcpy(prev_name, instructionsArray[ZERO][index].name );
            }
            else { /*not first word*/
                strcpy(operand, instructionsArray[ZERO][index].operands);
                if (strchr(operand, CHAR_DOT)) { /* remove '.'*/
                    strchr(operand, CHAR_DOT)[ZERO] = NULL_CHAR;
                }
                if (!strIsInteger(operand) && getAddressingType(operand) != 3 && getAddressingType(operand) != -1) { /*operand is a label*/
                    address = findLabelAddress(*labelsArray, operand, L);
                    if (address == ZERO) { /*extern label*/
                        address += 1;
                        if (!created_ext){
                        	printf("created .ext file\n");
                        	ptr = fopen(makeNewFilename(filename, ".ext"), "w");
                        	created_ext = 1;
                        }
                        createExtFile(operand, instructionsArray[0][index].address, ptr);
                    }
                    else if (address == -1) {
                        printf("MAJOR ERROR: Use of label %s which isn't defined in file\n", operand);
                        return ZERO;
                    }
                    else {
                        address = address << 2;
                        address += 2;
                    }
                    strcpy(instructionsArray[ZERO][index].base32, convertDecTo32(address));
                }
            }
        }
    }
    if (created_ext){    
		fclose(ptr);
    }
    free(operand); free(prev_name);
    return 1;

}

 int findLabelAddress(label* labelsArray, char* label_name, int L) {
    /* searches for label_name in labelsArray, if found returns it's address, else returns ZERO*/
    int index = ZERO;
    for (index = ZERO; index <= L; index++) {
        if (!strcmp(label_name, labelsArray[index].name)) {
            /*if not entry and extern type */
            if (strcmp(labelsArray[index].type, ".entry") && strcmp(labelsArray[index].type, ".extern")) {
                return labelsArray[index].address;
            } /*if entry type - label saved twice in labelsArray and address is in the next occurence*/
            if (!strcmp(labelsArray[index].type, ".extern")) { /*address unkown for .extern*/
                return ZERO;
            }
        }
    }return -1;
}

 void createObjFile(instruction** instructionsArray, data** dataArray, int IC, int DC, char* filename) {
    int indexI = ZERO; int indexD = ZERO;
    int addressI, addressD; /*save addresses from two arrays to compare with each other*/
    FILE* ptr;
    char* output32 = malloc(MAX_STR_SIZE);
    char* filename_obj = makeNewFilename(filename, ".obj");

    ptr = fopen(filename_obj, "w");
    if (NULL == ptr) {
        printf("example.obj wasn't created\n");
        return;
    }

    if (output32) {
        strcpy(output32, convertDecTo32(IC));
        strcat(output32, " ");
        strcat(output32, convertDecTo32(DC));
        strcat(output32, "\n");
        fprintf(ptr, "%s", output32);
        while ((indexI <= IC - 1) || (indexD <= DC - 1)) {
            /*get two next addresses*/
            if (indexI <= IC - 1) {
                addressI = instructionsArray[ZERO][indexI].address;
                /*++indexI;*/
            }
            else {
                addressI = INT_MAX; /*reached end of instructions array so set addressI to max number*/
            }
            if (indexD <= DC - 1) {
                addressD = dataArray[ZERO][indexD].address;
            }
            else {
                addressD = INT_MAX;
            }
            /*save contents of smaller address*/
       
            if (addressI < addressD) {
                strcpy(output32, convertDecTo32(addressI));
                strcat(output32, " ");
                strcat(output32, instructionsArray[ZERO][indexI].base32);
                strcat(output32, "\n");
                fprintf(ptr, "%s", output32);
                ++indexI;
            }
            else {
                strcpy(output32, convertDecTo32(addressD));
                strcat(output32, " ");
                strcat(output32, dataArray[ZERO][indexD].base32);
                strcat(output32, "\n");
                fprintf(ptr, "%s", output32);
                ++indexD;
            }
        }
    }
    fclose(ptr);
    free(output32);
    printf("created .obj file\n");

}

 void createEntFile(label** labelsArray, int L, char* filename) {
    char* filename_ent = makeNewFilename(filename, ".ent");
    int created = ZERO; /*turns to 1 once file has been created */
    int index; int index_s;
    int search = ZERO; /* turns to 1 when we find a entry label to search for*/
    char* label_name = malloc(MAX_STR_SIZE);
    char* output = malloc(MAX_STR_SIZE);
    FILE* ptr = NULL;

    for (index = ZERO; index <= L - 1; index++) {
        if (!search && !strcmp(labelsArray[ZERO][index].type, ".entry")) {
            search = 1;
            if (label_name) { strcpy(label_name, labelsArray[ZERO][index].name); }
            else {
                printf("problem with labelname malloc\n");
                exit(1);
            }
            /*search for label name*/
            for (index_s = ZERO; index_s <= L - 1; index_s++) {
                if (!strcmp(labelsArray[ZERO][index_s].name, label_name) && (strcmp(labelsArray[ZERO][index_s].type, ".entry")) && output) { /*found label name with address*/
                    strcpy(output, label_name);
                    strcat(output, " ");
                    strcat(output, convertDecTo32(labelsArray[0][index_s].address));
                    strcat(output, "\n");

                    if (!created) {
                        created = 1;
                        ptr = fopen(filename_ent, "w");
                    }
                    
                    if (ptr) {
                        fprintf(ptr, "%s", output);
                    }
                    else {
                        printf("ptr wasn't created\n");
                        return;
                    }
                    search = ZERO;
                    index_s = L;
                }
            } 
            if (search) { /*didn't find label address*/
                printf("ERROR: label name %s is declared as .entry but no location was found in the file\n", label_name);
                exit(1); /*cannot create .entry file nor other files*/
            }
        }
    }
    if (ptr){
    	printf("created .ent file\n");
        fclose(ptr);
    }
    free(output); free(label_name);
}

 void createExtFile(char* label_name, int address, FILE* ptr) {
    /*append to .ext file - label name of .ext type with address in memory where it is used*/
    char* output = malloc(MAX_STR_SIZE);
    if (!ptr) {
        printf("Didn't open ptr successfuly\n");
        return;
    }

    if (output) {
        strcpy(output, label_name);
        strcat(output, " ");
        strcat(output, convertDecTo32(address));
        strcat(output, "\n");
        fprintf(ptr, "%s", output);
    }
    free(output);

}

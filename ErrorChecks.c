#include "header.h"


 int ErrorChecker(char* label_name, label** labelsArray, int L, char* instruction_name, char* data_type, char* operands, int line_no) {
    int result = 0;
    /*variables for instructions*/
    char opcodes[16][4] = { "mov","cmp", "add", "sub", "not", "clr", "lea", "inc", "dec", "jmp", "bne",
                        "get", "prn", "jsr", "rts", "hlt" };
    int index = 0;
    int found = 0;
    char* s[] = { "not", "clr","inc", "dec", "jmp", "bne", "get", "prn", "jsr", "rts" }; /*opcodes w/o origin operand*/

    /*variables for operands*/
    char* temp_operands = malloc(MAX_STR_SIZE);
    char* token = temp_operands;
    char* origin_o = temp_operands; /*origin operand*/
    char* dest_o = temp_operands; /*destination operand*/
    int count = 0;

    /*check mallocs*/
    if (!temp_operands || !origin_o || !dest_o) {
        printf("ERROR: malloc didn't work\n");
        exit(1);
    }

    /*labels check*/
    if (*label_name != NULL_CHAR) {
        if (!strIsAlnum(label_name)) { /*label name contains numbers*/
            printf("MAJOR ERROR: Label name %s in line %d contains non alphanumeric characters!\n", label_name, line_no);
            result = 1;
        }
        if (strlen(label_name) > MAX_LABEL_LEN) {
            printf("MAJOR ERROR: Label name %s in line %d contains more than 30 characters!\n", label_name, line_no);
            result = 1;
        }
        if (L >= 1) { /*check that same label isn't defined twice*/
            for (index = 0; index <= L-1; index++) {
                if (!strcmp(labelsArray[0][index].name, label_name) && strcmp(labelsArray[0][index].type, ".entry")) {
                    printf("MAJOR ERROR: Illegal attempt to connect label %s in line %d to more than one location\n", label_name, line_no);
                    result = 1;
                    index = L + 1;
                }
            }
        }
        if (*instruction_name == '\0' && *data_type == '\0' && *operands == '\0'){ /*empty line - no label contents*/
        	printf("MAJOR ERROR: Label name %s in line %d contains no contents\n", label_name, line_no);
        }
    }

    /*instruction sentences*/
    if (*instruction_name != NULL_CHAR) {
        for (index = 0; index <= 15; index++) {/*check that instruction name exists */
            if (!strcmp(instruction_name, opcodes[index])) {
                found = 1;
                index = 16;/*break the loop*/
            }
        }
        if (!found) {
            printf("MAJOR ERROR: Instruction name %s in line %d doesn't exist!\n", instruction_name, line_no);
            result = 1;
        }

        /*splitting operands, check addressing type errors*/
        if (operands) {
            strcpy(temp_operands, operands);
        }
        else {
            temp_operands = NULL;
            token = NULL;
        }
        origin_o = strtok(token, " \t\r,");
        dest_o = strtok(NULL, " \t\r,");
        if (inArray(instruction_name, s, 10)) {
            dest_o = origin_o;
            origin_o = NULL;
        }
        if (AddressingTypeError(instruction_name, 1, getAddressingType(origin_o), line_no) || AddressingTypeError(instruction_name, 0, getAddressingType(dest_o), line_no)) {
            result = 1;
        }
        /*register and immediate addressing type error checks*/
        if (origin_o) {
            if (origin_o[0] == 'r') {
                result = registerErrorCheck(origin_o, line_no) || result;
                if (registerErrorCheck(origin_o, line_no)){
		        	printf("MAJOR ERROR: Operand %s in line %d doesn't contain a correct number!\n", origin_o, line_no);
		        }
            }
            else if (origin_o[0] == '#') {
                result =  !strIsInteger(origin_o + 1) || result;
		        if (!strIsInteger(origin_o+1)){
		        	printf("MAJOR ERROR: Operand %s in line %d doesn't contain a correct number!\n", origin_o, line_no);
		        }
            }
        }if (dest_o) {
            if (dest_o[0] == 'r') {
                result =  registerErrorCheck(dest_o, line_no) || result;
                if (registerErrorCheck(dest_o, line_no)){
            	printf("MAJOR ERROR: Operand %s in line %d doesn't contain a correct number!\n", dest_o, line_no);
            	}
            }
            else if (dest_o[0] == '#') {
                result = !strIsInteger(dest_o + 1)  || result;
                if (!strIsInteger(dest_o+1)){
            	printf("MAJOR ERROR: Operand %s in line %d doesn't contain a correct number!\n", dest_o, line_no);
            	}
            }
            
        }

    }

    /*data sentences*/
    if (*data_type != NULL_CHAR) { /*check that data type exists*/
        if (strcmp(data_type, ".string") && (strcmp(data_type, ".data") && (strcmp(data_type, ".struct")) && (strcmp(data_type, ".entry")) && (strcmp(data_type, ".extern")))) {
            printf("MAJOR ERROR: Data type %s in line %d doesn't exist!\n", data_type, line_no);
            result = 1;
        }
        else if (!strcmp(data_type, ".data")) {
            result =  DataTypeErrorCheck(operands, line_no)  || result;
        }
        else if (!strcmp(data_type, ".string")) {
            result = stringTypeErrorCheck(operands, line_no) || result ;
        }
        else if (!strcmp(data_type, ".struct")) {
            result = structTypeErrorCheck(operands, line_no) || result  ;
        }
        else if (!strcmp(data_type, ".entry")) {
            if (*label_name != NULL_CHAR) {
                printf("Warning in line %d: You cannot have a label to a .entry type\n", line_no);
                *label_name = NULL_CHAR;
            }
        }
        else if (!strcmp(data_type, ".extern")) {
            if (*label_name != NULL_CHAR) {
                printf("Warning in line %d: You cannot have a label to a .extern type\n", line_no);
                *label_name = NULL_CHAR;
            }
        }
    }


    /*operands*/
    if (!operands && (*data_type != NULL_CHAR && strcmp(instruction_name, "rts") && strcmp(instruction_name, "hlt"))) {
        printf("MAJOR ERROR: No operands supplied in line %d! \n", line_no);
        result = 1;
    }
    if (operands) {
        /*operand count is done according to individual sentence types therefore comma count isn't checked here*/
        /*check if first and last operand isn't a comma*/
        if (begginningEndComma(operands)) {
            printf("MAJOR ERROR: Operands \"%s\" in line %d contain an incorrect comma in the beginning/end\n", operands, line_no);
            result = 1;
        }

        /*checks for comma sequences*/
        if (commaSequence(operands)) {
            printf("MAJOR ERROR: Operands \"%s\" in line %d contain a comma sequence!\n", operands, line_no);
            result = 1;
        }

        /*count num of operands*/
        strcpy(temp_operands, operands);
        index = 0; count = 0;
        token = temp_operands;
        token = strtok(token, " \t\r,");
        while (token) {
            ++count;
            token = strtok(NULL, " \t\r,");
        }
        if (*operands != '\0' && count != commaCount(operands) + 1) {
            printf("MAJOR ERROR: Incorrect usage of commas in operands %s in line %d\n", operands, line_no);
            result = 1;
        }
        /*number of operands is too large for certain sentence types*/
        if ((count > 2 && *instruction_name != NULL_CHAR) || (!strcmp(data_type, ".extern") && count > 1) || (!strcmp(data_type, ".entry") && count > 1)) {
            printf("MAJOR ERROR: More operand than allowed in line %d!\n", line_no);
            result = 1;
        }
    }
    free(temp_operands);
    return result;


}

 int AddressingTypeError(char* opcode, int origin, int add_type, int line_no) {
    /* Checks that supplied opcode and operands use the correct addressing types.
    if origin = 1 its an origin operand, else its a destination operand*/
    int index1 = ZERO;
    int index2 = ZERO;
    int index3 = ZERO;
    int flag = ZERO;/*turn flag on when we find addressing type - return error (1) only if flag stays off*/
    address_group* addressing_types = malloc(6 * sizeof(address_group));
    /*address_group a, b, c, d, e, f;*/
    /*initialise address_group options for origin operand*/
    address_group a = {{0,1,2,3},
    					{"mov", "cmp", "add", "sub"},
    					1};
    address_group b = { {-1, -1, -1, -1},
						{"not", "clr", "inc", "dec", "jmp", "bne", "get", "prn", "jsr", "rts", "hlt"},
						1 };
    address_group c = {{1,2,2,2}, 
    					{"lea"},
    					1 };

    /*initialise addressing group options for destination operand*/
    address_group d = {{1,2,3, 3},
    					{"mov", "add", "sub", "not", "clr", "lea", "inc", "dec", "jmp", "bne", "get", "jsr"},
    					0 };
    address_group e = {{0,1,2,3},
    					{"cmp", "prn"},
    					 0};
    address_group f = {{-1, -1, -1, -1},
    				 {"rts", "hlt"},
    				  0 };
    
    addressing_types[0] = a; addressing_types[1] = b; addressing_types[2] = c;
    addressing_types[3] = d; addressing_types[4] = e; addressing_types[5] = f;

	if (!addressing_types){
		printf("malloc on addressing_types didn't work\n");
		exit(1);
	}

    if (origin) {
        for (index1 = ZERO; index1 <= 2; index1++) { /*iterate through addresing_types*/
            for (index2 = ZERO; index2 <= 12; index2++) { /*iterate through all opcodes in given addressing group*/
                if (!strcmp(addressing_types[index1].opcodes[index2], opcode)) {
                    for (index3 = ZERO; index3 <= 4; index3++) { /*if found opcode - iterate through all possible addresses*/
                        if (addressing_types[index1].addresses[index3] == add_type) { /*if given address is correct for opcode - turn flag on*/
                            flag = 1;
                            index3 = 4; index2 = 12; index1 = 2;
                        }
                    }
                }
            }
        }


    }
    else { /* destination operand*/
        for (index1 = 3; index1 <= 5; index1++) { /*iterate through addresing_types*/
            for (index2 = ZERO; index2 <= 12; index2++) { /*iterate through all opcodes in given addressing group*/
                if (!strcmp(addressing_types[index1].opcodes[index2], opcode)) {
                    for (index3 = ZERO; index3 <= 4; index3++) { /*if found opcode - iterate through all possible addresses*/
                        if (addressing_types[index1].addresses[index3] == add_type) { /*if given address is correct for opcode - turn flag on*/
                            flag = 1;
                            index3 = 4; index2 = 12; index1 = 5;
                        }
                    }
                }
            }
        }
    }

    if (!flag) {
        printf("MAJOR ERROR: Incorrect addressing type for opcode %s in line %d\n", opcode, line_no);
        free(addressing_types);
        return 1;
    }
    free(addressing_types);

	return ZERO;



}

 int DataTypeErrorCheck(char* operands, int line_no) {
    /*Checks that operands are whole numbers,*/
    char* temp_operands = malloc(MAX_STR_SIZE);
    char* token = temp_operands;
    int result = ZERO;

	if (!operands){
		/*printf("MAJOR ERROR: No operands in line %d\n", line_no);*/
		free(temp_operands);
		return 1;
	}

    /*check if operands are integers*/
    if (temp_operands) {
        strcpy(temp_operands, operands);
    }
    token = strtok(token, " \t\r,");
    while (token) {
        if (!strIsInteger(token)) {
            printf("MAJOR ERROR: Operand %s in line %d is not an integer! \n", token, line_no);
            result = 1;
        }
        token = strtok(NULL, " \t\r,");
    }

    free(temp_operands);
    return result;
}

 int stringTypeErrorCheck(char* operands, int line_no) {
    char* temp_operands = malloc(MAX_STR_SIZE);
    char* str_cut = temp_operands;
    int result = ZERO;
    int ind;

	if (!operands){
		/*printf("MAJOR ERROR: No operands in line %d\n", line_no);*/
		free(temp_operands);
		return 1;
	}
	
    if (temp_operands) {
        strcpy(temp_operands, operands);
        /*check that contains " twice*/
        if (!strchr(temp_operands, '\"')) {
            printf("MAJOR ERROR: operands %s in line %d don't contain quotation marks \n", temp_operands, line_no);
            result = 1;
        }
        else {
            str_cut = strchr(temp_operands, '\"') + 1; /*assign string following first " to strcut*/
            if (!strchr(str_cut, '\"')) { /*doesn't contain second "*/
                /*printf("MAJOR ERROR: operands  %s", temp_operands);*/
                printf("MAJOR ERROR: operands ");
                printf(" %s ", temp_operands);
                printf("in line %d don't contain second quotation marks\n", line_no);
                result = 1;
            }
        }
    }

    if (!result && temp_operands) {
        /*remove "" from string*/
        str_cut = strchr(temp_operands, '\"') + 1; /*remove first "*/
        ind = (int)(strchr(str_cut, '\"') - str_cut); /*index of second "*/
        str_cut[ind] = NULL_CHAR; /* remove second "*/

        /*check if string contains only letters*/
        if (!strIsAlpha(str_cut)) {
            printf("MAJOR ERROR: operands \"%s\" in line %d contain non-alpha letters!\n", operands, line_no);
            result = 1;
        }
    }

    free(temp_operands);
    return result;
}

 int structTypeErrorCheck(char* operands, int line_no) {
    /*Returns 1 if found an error*/
    char* temp_operands = malloc(MAX_STR_SIZE);
    char* token = temp_operands;
    int result = ZERO;
    
    if (!operands){
		/*printf("MAJOR ERROR: No operands in line %d\n", line_no);*/
		free(temp_operands);
		return 1;
	}
    
    if (temp_operands) {
        strcpy(temp_operands, operands);
        /*check that there is a comma*/
        if (!strchr(temp_operands, ',')) {
            printf("MAJOR ERROR: No comma in operands %s in line %d with struct data type!\n", operands, line_no);
            result = 1;
        }
        /*check that first operand is a number*/
        token = strtok(token, " \t\r,");
        result = DataTypeErrorCheck(token, line_no) || result ;

        /*check that second operand is a string*/
        strcpy(temp_operands, operands);
        token = strtok(token, " \t\r,");
        token = strtok(NULL, " \t\r,");
        result = stringTypeErrorCheck(token, line_no)  || result ;

        free(temp_operands);
    }
    return result;
}

 int registerErrorCheck(char* r, int line_no) {
    int num; char str[2];
    int result = ZERO;

    if (!strIsInteger(r + 1)) {
        printf("MAJOR ERROR: Register %s in line %d contains non-number characters\n", r, line_no);
        result = 1;
    }

    if (!result) {
        str[0] = r[1]; /*save register number in string*/
        str[1] = NULL_CHAR;
        num = atoi(str); /*convert str to number*/
        if (num < ZERO || num > 7) {
            printf("MAJOR ERROR: Register %s in line %d doesn't exist\n", r, line_no);
            result = 1;
        }
    }
    return result;
}

 int commaCount(char* operands) {
    /*Also checks if last operand is a comma*/
    char c;
    int index = ZERO; int count = ZERO;

    c = operands[index];

    while (c != NULL_CHAR) {
        if (c == ',') {
            ++count;
        }
        ++index;
        c = operands[index];
    }

    return count;
}

 int begginningEndComma(char* operands) {
    /*Returns 1 if there is a comma in the beginning of the operands or in the end*/

    char* temp_operands = malloc(MAX_STR_SIZE);
    char* p = temp_operands;
    int ind;/*index of first or last occurence of comma*/
    int index = ZERO;
    int result = ZERO;

    /*check if there even are commas*/
    if (!strchr(operands, ',')) {
        free(temp_operands);
        return ZERO;
    }
    /*checks for beginning comma*/
    if (temp_operands) {
        strcpy(temp_operands, operands);
        ind = (int)(strchr(temp_operands, ',') - temp_operands); /*index of first comma*/
        if (ind > ZERO) {
            p[ind] = NULL_CHAR;
            p = strtok(p, " \t\r"); /* remove whitespace from p*/
            if (!p || *p == NULL_CHAR) { /*check space until first comma is empty (has only whitespace) */
                result = 1;
            }
        }
        /*checks for end comma*/
        strcpy(temp_operands, operands);
        p = temp_operands;
        while (operands[index] != NULL_CHAR) {
            if (operands[index] == ',') {
                ind = index;
            }
            ++index;
        }
        p = temp_operands + ind + 1; /*get space following last comma*/
        p = strtok(p, " \t\r");
        if (!p || *p == NULL_CHAR) {
            result = 1;
        }


        free(temp_operands);
    }

    return result;

}

 int commaSequence(char* operands) {
    /*returns 1 if there is an incorrect comma sequence in the middle of the operands*/
    char* temp_operands = malloc(MAX_STR_SIZE);
    char* cut_operands = temp_operands; /*will remove operands from here*/
    char* token = cut_operands; /*will store the values between each comma*/

    /*check if there even are commas*/
    if (!strchr(operands, ',')) {
        free(temp_operands); 
        return ZERO;
    }

    if (temp_operands && cut_operands && token) {
        strcpy(temp_operands, operands);
        cut_operands = strchr(cut_operands, ',') + 1; /*start following the first comma*/
        if (strchr(cut_operands, ',')) {
            /*strcpy(token, cut_operands);*/
            token = cut_operands;
            token[strchr(cut_operands, ',') - cut_operands] = NULL_CHAR; /*get space between first and second commma*/
        }
        else { /*if only one comma in string then return*/
            free(temp_operands);
            return ZERO;
        }
    }

    while (cut_operands) {
    	token = strtok(token, " \t\r");
        if (!token) { /*if space between two commas is whitespace then we have a comma sequence*/
            return 1;
        }
        if (strchr(cut_operands, ',')){
        	cut_operands = strchr(cut_operands, ',') + 1;
		    if (token) {
		        if (strchr(cut_operands, ',')) {
		        	token = cut_operands;
		            token[strchr(cut_operands, ',') - cut_operands] = NULL_CHAR;
		        }
		        else {
		            cut_operands = NULL;
		        }
		    }
        }else{
	        cut_operands = NULL;
        }
    }
    free(temp_operands); 
    return ZERO;

}

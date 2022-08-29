#include "header.h"

int callPreProcessor(char* filename) { /* "main"*/
	/*prepares variables for main preProcessor function*/
	
    int index = -1; /*keeps track of number of macros in macro array*/
    char* filename_as = makeNewFilename(filename, ".as");
    macro* macrosArray = malloc(sizeof(macro)); 
    if (macrosArray) {
        macrosArray[0].name = malloc(MAX_STR_SIZE);
        macrosArray[0].contents = malloc(MAX_STR_SIZE);
    }

    if (!countLineLength(filename_as)){/*check that no line has more than MAX_STR_SIZE chars*/
    	return ZERO;
    } 
    if (!preProcessor(macrosArray, index, filename_as)) { /*if preProcessor returns 0 then can't continue wiht rest of program*/
        free(macrosArray);
        return ZERO;
    }
    free(macrosArray);
    free(filename_as);
    return 1;
}

int preProcessor(macro* macrosArray, int index, char filename[]) {
    /* this function returns a .am file without macros */
    
    char* filename_am; /*will store filename with .am */
    FILE* ptr; /*represents file .as*/
    FILE* new_file; /*will store file without macros*/

    char str[MAX_STR_SIZE]; /*stores each string in the file, upper limit on length of each line in file is MAX_STR_SIZE    */
    int flag = ZERO; /*when flag is 1 we know to save contents of macro*/
    char* macro_buffer; /*used in mallocs reallocs */
    char* new_buffer; /*used in mallocs reallocs */
    size_t sz = MAX_STR_SIZE; /*used as a size buffer for macro_contents, initial value is MAX_STR_SIZE which is the upper limit for line size*/
    char* macro_contents = malloc(MAX_STR_SIZE); /*stores contents of each macro we find*/
    char* temp = malloc(MAX_STR_SIZE); /*used when filling up  macro_contents */
    *macro_contents = NULL_CHAR;

    ptr = fopen(filename, "r");/* Opening file in reading mode*/
    if (NULL == ptr) {
        printf("file %s can't be opened \n", filename);
        return ZERO;
    }

    /*create filename with.am */
    filename_am = makeNewFilename(filename, ".am");
    new_file = fopen(filename_am, "w"); /*creates new file for writing */
    if (NULL == new_file) {
        printf("newfile %s wasn't created\n", filename_am);
        return ZERO;
    }
    new_file = fopen(filename_am, "a"); /*using new .am file for appending */
    if (NULL == new_file) {
        printf("newfile %s wasn't created\n", filename_am);
        return ZERO;
    }


    while (fgets(str, MAX_STR_SIZE, ptr) != NULL) {
        flag = hasNewMacro(str, macrosArray, index, flag); /*check if need to start saving macro contents*/
        if (flag == -1) { /*can't continue with preprocessor*/
        	fclose(ptr); fclose(new_file);
            return ZERO;
        }
        outputAMfile(new_file, str, macrosArray, index, flag); /*save or don't save current line in .am file*/

        if (flag == 1) {
        	
            if (*macro_contents == NULL_CHAR) { /*first time finding new macro - save current line in macro contents*/
                strcpy(macro_contents, str);
            }
            else {
                sz += MAX_STR_SIZE;
                macro_buffer = (char*)realloc(macro_contents, sz); /*increase space in macrocontents*/
                if (!macro_buffer) {
                    printf("out of memory for macro contents, error: %s\n", strerror(errno));
                    exit(1);
                }
                macro_contents = macro_buffer;

                /*if the line "macro [macro_name] is saved then use strcpy to get rid of it"*/
                strcpy(temp, macro_contents);
                if (!strcmp(strtok(temp, " \t\n"), "macro")) { /*check if first word in macrocontents is the macro introduction line*/
                    strcpy(macro_contents, str);
                }
                else {
                    strcat(macro_contents, str);
                }
            }
        }
        else if (flag == ZERO && *macro_contents != NULL_CHAR) { /*save macro contents in macrosArray*/
            ++index;

            new_buffer = (char*)realloc(macrosArray[index].contents, sz); /*initialises space for macro contents*/
            if (!new_buffer) {
                printf("new_buffer didn't work, error: %s\n", strerror(errno));
                exit(1);
            }
            macrosArray[index].contents = new_buffer; /*assign newly allocated space to macrosArray contents*/

            strcpy(macrosArray[index].contents, macro_contents);
            *macro_contents = NULL_CHAR; /*empty macro contents*/

        }
    }
    fclose(ptr);
    fclose(new_file);
    free(macro_contents);
    free(temp);
    freeMacrosArray(macrosArray, index);
    return 1;
}

int hasNewMacro(char str[], macro* macrosArray, int index, int flag) {
    /*returns 1 if we found a newly defined macro, else returns ZERO. 
    Returns -1 in special case where preprocessor needs to stop (problem with macro definition). 
    Also intialises space in macrosArray*/
    char* temp_str = malloc(MAX_STR_SIZE);
    char* first_word = temp_str; 
    char* macro_name = temp_str;
    char* malloc1;
    char* malloc2;
    macro* temprealloc;
    
    if (temp_str) {
        strcpy(temp_str, str); 
        first_word = strtok(temp_str, " \t\n\r"); /*check first word in string - it is the only one that can be macro*/
        if (first_word && !strcmp(first_word, "macro")) { /*defining new macro - initialise space in macros array*/
            macro_name = strtok(NULL, " \t\n\r");

            ++index;
            if (macroNameOK(macro_name) == ZERO) { /*can't continue with preprocessor*/
                return -1;
            }
            if (index >= ZERO) { /*not first time saving in macros array - realloc is needed*/
            	if (!macroExists(macrosArray, macro_name, index)){
            	
		            temprealloc = (macro*)realloc(macrosArray, (index+1) * sizeof(macro)); /*create space for new macro*/
		            if (!temprealloc) {
		                printf("realloc in hasnewmacro didn't work\n");
		                exit(ZERO);
		            }
		            macrosArray = temprealloc;
		        
		            /*create space for macro name and contents*/
		            malloc1 = (char*)malloc(MAX_STR_SIZE);
		            if (malloc1) {
		                macrosArray[index].name = malloc1;
		                strcpy(macrosArray[index].name, macro_name);
		            }
		            malloc2 = (char*)malloc(MAX_STR_SIZE);
		            if (malloc2) {
		                macrosArray[index].contents = malloc2;
		            }
            	}else{
            		free(temp_str);
            		return -1;
            	}
            }
            else {
                strcpy(macrosArray[index].name, macro_name);
            }

            free(temp_str);
            return 1;
        }
        else if (first_word && !strcmp(first_word, "endmacro")) {
            free(temp_str);
            return ZERO;
        }
    }
    free(temp_str);
    return flag; /*returns last known flag - 1 if we need to continue saving macro contents or 0 if not */
}

void outputAMfile(FILE* filep, char str[MAX_STR_SIZE], macro* macrosArray, int len, int flag) {
    /*saves appropriate instructions in .am file*/
    int index = ZERO;
    char* temp_str = malloc(MAX_STR_SIZE);
    char* first_word = temp_str;

    if (filep == NULL) {
        printf("filep wasn't created properly\n");
        exit(1);
    }

    if (temp_str) {
        strcpy(temp_str, str);
        first_word = strtok(first_word, " \t\n\r");
        if (first_word && (flag == 1 || (!strcmp(first_word, "endmacro")))) {
        	free(temp_str);
            return;
        }
    }

    while (index <= len) { /*insert macro contents*/
        if (first_word && !strcmp(macrosArray[index].name, first_word)) { /*instruction name is equal to existing macro*/
            fprintf(filep, "%s", macrosArray[index].contents);
            free(temp_str);
            return;
        }
        ++index;
    }
    fprintf(filep, "%s", str);
    free(temp_str);
    return;
}

 int countLineLength(char* filename)
{ /*makes sure each line in file isn't more than MAX_STR_SIZE chars. REturns 0 if file not good*/
    char c;
    int count = ZERO;
    int whitespace_count = ZERO;
    FILE* fp = fopen(filename, "r");/* Opening file in reading mode*/
    if (NULL == fp) {
        printf("file %s can't be opened\n", filename);
        return ZERO;
    }

    do {
        c = fgetc(fp);
        if (feof(fp))
            break;
        if (c == '\n' || c == EOF) {

            count = ZERO;
        }
        else if (c == ' ' || c == '\t' || c == 'r') {
            ++whitespace_count;
        }
        else {
            ++count;
            count += whitespace_count;
            whitespace_count = ZERO;
            if (count > MAX_STR_SIZE) {
                printf("ERROR: Line in input file contains more than MAX_STR_SIZE chars");
                fclose(fp);
                return ZERO;
            }
        }

    } while (1);
    fclose(fp);
    return 1;

}

 int macroExists(macro* macrosArray, char* macro_name, int len) {
    /*given a macrosArray with at least one item, check if new macroname doesn't exist already in the array.
    Return 1 if it exists, else 0*/

    int index = ZERO;
    for (index = ZERO; index <= len; index++) {
        if (!strcmp(macrosArray[index].name, macro_name)) {
        	printf("Warning in preprocessor: found double declaration of macro.%s\n", macro_name);
            return 1;
        }
    }
    return ZERO;
}

 int macroNameOK(char* macro_name) {
    /*Returns 1 if macro name is applicable - not empty, contains only alphanumeric characters. 
    Else returns 0*/
    int result = 1;
    if (!macro_name || !strIsAlnum(macro_name)) {
        result = ZERO;
    }
    
    if (!result) {
        printf("ERROR: macro name %s is not applicable\n", macro_name);
    }

    return result;

}

void freeMacrosArray(macro* macrosArray, int len){
	int index;
	for (index = 0; index <= len; index++){
		if (macrosArray[index].name){
			free(macrosArray[index].name);
		}
		if (macrosArray[index].contents){
			free(macrosArray[index].contents);
		}
		
		
	}
}

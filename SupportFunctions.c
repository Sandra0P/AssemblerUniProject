#include "header.h"


/*Functions for checking data types of given string*/
 int strIsInteger(char* str) {
    /*returns 0 if there is a non integer digit in str, else returns 1 (whole str is comprised of numbers)*/
    int index = ZERO;
    for (index = ZERO; str[index] != NULL_CHAR; index++) {
        if (!isdigit(str[index]) && str[index] != '-' && str[index] != '+') {
            return ZERO;
        }
    }
    return 1;
}

 int strIsAlnum(char* str) {
    /*Returns 1 if string is comprised of only alphanumeric characters */
    int index = ZERO;
    int result = 1;

    while (str[index] != NULL_CHAR) {
        if (!isalnum(str[index]) && str[index] != '-' && str[index] != '+') {
            result = ZERO;
        }
        ++index;
    }
    return result;
}

 int strIsAlpha(char* str) {
    /*Returns 1 if string contains only alpha characters */
    int index = ZERO;
    int result = 1;

    while (str[index] != NULL_CHAR) {
        if (!isalpha(str[index])) {
            result = ZERO;
        }
        ++index;
    }
    return result;
}

/*-------------------------------------------------*/

 int inArray(char* str, char** strArray, int len) {
    /*returns 1 if str is in the string array. len is length of array*/
    int index;
    for (index = ZERO; index <= len - 1; index++) {
        if (!strcmp(strArray[index], str)) {
            return 1;
        }
    }return ZERO;
}

char* makeNewFilename(char filename[], char* ending) {
    /*concatenates filename with ending*/
    int len = (int)strlen(filename);
    char* filename_full = malloc(len + 1); /*+1 to account for last NULL_CHAR */
    char* temp_realloc;

    if (filename_full) {
        strcpy(filename_full, filename);
        if (strchr(filename_full, '.')) {
            filename_full = strtok(filename_full, "."); /*get actual filename  */
        }
        temp_realloc = realloc(filename_full, len + 4 + 1); /*length of ending - 4 characters. add 1 for NULL_CHAR*/
        if (!temp_realloc) {
            printf("error");
            exit(1);
        }filename_full = temp_realloc;
        strcat(filename_full, ending); /*adding .ending*/
        return filename_full;
    }
    printf("MALLOC in getNewFilename didn't work\n");
    exit(1);

}

#include "header.h"


 void saveBase32ContentsInDataArray(data** dataArray, char* data_type, int index) {
    /*given a dataArray with the index of a particular word, we use the data type to decide how to convert the contents and save their base32 value*/
    int num; /*will store number that needs to pass 32 conversion*/
    unsigned num_neg;
    int mask = 1023; /*in binary this is 1111111111 (10 one's)*/
    if (!strcmp(data_type, ".string")) {
        num = (int)(dataArray[ZERO][index].contents[ZERO]);
        strcpy(dataArray[ZERO][index].base32, convertDecTo32(num));
    }
    else if (!strcmp(data_type, ".data")) {
        num = atoi(dataArray[ZERO][index].contents);
        if (num < ZERO) {
            num_neg = num & mask;
            strcpy(dataArray[ZERO][index].base32, convertDecTo32(num_neg));
        }
        else {
            strcpy(dataArray[ZERO][index].base32, convertDecTo32(num));
        }
    }

    else if (!strcmp(data_type, ".struct")) {
        /*if operand is a number*/
        if (isdigit(dataArray[ZERO][index].contents[ZERO])) {
            num = atoi(dataArray[ZERO][index].contents);
            if (num < ZERO) {
                num_neg = num & mask;
                strcpy(dataArray[ZERO][index].base32, convertDecTo32(num_neg));
            }
            else {
                strcpy(dataArray[ZERO][index].base32, convertDecTo32(num));
            }
        }
        /*if operand is a character*/
        else {
            num = (int)(dataArray[ZERO][index].contents[ZERO]);
            strcpy(dataArray[ZERO][index].base32, convertDecTo32(num));
        }
    }
}

 void saveBase32InstructionArray(instruction** instructionsArray, int IC, int index, char instruction[3]) {
    char* operands = malloc(MAX_STR_SIZE);
    char* token = operands;
    int num, num1, num2;
    int r1, r2; /*will be used for register index separation*/
    char array[2]; /*used to store individual register numbers as string*/
    unsigned int num_neg;
    int mask = 255; /*in binary this is 11111111 (8 one's - we only want the 8 rightmost bits)*/

    if (operands) {
        strcpy(operands, instructionsArray[ZERO][IC + index].operands);
    }
    if (!operands || operands[ZERO] == NULL_CHAR) {
        /*printf("no operands supplied to base32 conversion\n");*/
        operands = NULL;
    }
    if (index == ZERO) { /*first word*/
        num = getOpcodes32(instruction); /*get opcode*/
        if (num == -1) {
            printf("ERROR: non existent opcode\n");
            exit(ZERO);
        }
        num = num << 6; /*move num 6 bits left*/
        if (operands) {
            token = strtok(token, " \t\r,"); /*get first operand*/
            num1 = getAddressingType(token);  /*addressing type of origin operand*/
            num1 = num1 << 4; /*move num1 4 bits left*/
            token = strtok(NULL, " \t\r,"); /*get second operand*/
            if (!token) {
                num2 = ZERO;
                num1 = num1 >> 2; /*move num1 2 bits right*/
            }
            else {
                num2 = getAddressingType(token); /*addressing type of destination operand*/
                num2 = num2 << 2; /*move num2 2 bits left*/
            }
            num = num | num1; /*add num and num1*/
            num = num | num2; /*add num2 into the mix*/
        }
        strcpy(instructionsArray[ZERO][IC + index].base32, convertDecTo32(num));
    }

    /*not first word*/
    else if (index >= 1) {
        if (operands && !isalpha(operands[0])) { /*save actual number*/ 
            num = atoi(operands);
            if (num < ZERO) {
                num_neg = num & mask;
                num = num_neg;
            }
            num = num << 2; /*shift bits in num left by 2 - enables code A (absolute - ZEROZERO)*/
            strcpy(instructionsArray[ZERO][IC + index].base32, convertDecTo32(num));
        }
        else if (operands && operands[0] == 'r') { /*register addressing type */
            array[ZERO] = operands[1];
            strcpy(array + 1, "");
            r1 = atoi(array); /*get number of first register*/
            r1 = r1 & 15; /*get 4 leftmost bits of r1 (a precaution)*/

            if (index == 1) {/*first register*/
                r1 = r1 << 6; /*move r1 left 6 spaces*/
            }
            else {
                r1 = r1 << 2; /*move r1 left 2 spaces*/
            }

            /*if there is another register*/
            if (operands && strchr(operands, ',')) {
                token = strchr(operands, ',');
                token = strchr(token, 'r') + 1;
                array[ZERO] = token[ZERO];
                r2 = atoi(array); /*get number of second register*/

                r2 = r2 & 15; /*get 4 leftmost bits of r2 (a precaution)*/
                r2 = r2 << 2; /*move r2 left 2 spaces*/
                r2 = r2 | 960; /*mask with 1111 0000 00 to give space for r1*/
                r1 = r1 | 60;/*mask with 0000 1111 00 - gives space for r2*/
                r1 = r1 & r2; /*join r1 and r2 together*/
            }
            strcpy(instructionsArray[ZERO][IC + index].base32, convertDecTo32(r1));
        }
    }



    if (operands) {
        free(operands);
    }


}

 char* convertDecTo32(int num) {
    /*converts integers that in base 32 won't take up more than 2 characters*/
    char* result = malloc(3);
    char chars32[33] = { '!', '@', '#', '$', '%', '^', '&', '*', '<', '>', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n', 'o', 'p', 'q', 'r', 's', 't', 'u', 'v' };
    int num1, num2;
    unsigned int num_neg;
    int mask = 1023; /*in binary this is 1111111111 (10 one's)*/
    if (!result) {
        printf("Malloc error in line 651 firstIteration.c\n");
        exit(1);
    }


    if (num > 1023) {
        printf("ERROR: cannot convert to base 32 a number larger than 1023\n");
        exit(ZERO);
    }
    if (num < ZERO) {
        num_neg = num & mask;
        num = num_neg;
    }

    if (num > 32) {
        num1 = num % 32;
        num2 = num / 32;

        result[ZERO] = chars32[num2];
        result[1] = chars32[num1];
        result[2] = NULL_CHAR;
    }
    else {
        result[ZERO] = chars32[ZERO];
        result[1] = chars32[num];
        result[2] = NULL_CHAR;
    }
    return result;
}

 int getOpcodes32(char name[3]) {
    /*finds desired opcode in opcodes list and returns its index (its code) in base 32*/
    int index = ZERO;
    /*opcodes table*/
    char opcodes[16][4] = { "mov","cmp", "add", "sub", "not", "clr", "lea", "inc", "dec", "jmp", "bne",
                        "get", "prn", "jsr", "rts", "hlt" };

    while (index <= 15) {
        if (!strcmp(opcodes[index], name)) {
            return index;
        }
        ++index;
    }return -1;

}

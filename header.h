#ifndef MY_HEADER_FILE_ /*header guards - allows multiple files to include the same header file without compilation error*/
#define MY_HEADER_FILE_



#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <ctype.h>
#define MAX_STR_SIZE 80
#define BASE_ADDRESS 100
#define CHAR_HASH '#'
#define CHAR_DOT '.'
#define CHAR_COLON ':'
#define INT_MAX 2147483647
#define ZERO 0
#define NULL_CHAR '\0'
#define MAX_LABEL_LEN 30



/*preprocessor declarations*/
struct macro {
    char* name;
    char* contents;
};
typedef struct macro macro;

int callPreProcessor(char* filename);
int preProcessor(macro* macrosArray, int index, char filename[]);
int hasNewMacro(char str[], macro* macrosArray, int index, int flag);
void outputAMfile(FILE* filep, char str[80], macro* macrosArray, int len, int flag);
int countLineLength(char* filename);
int macroExists(macro* macrosArray, char* macro_name, int len);
int macroNameOK(char* macro_name);
void freeMacrosArray(macro* macrosArray, int len);

/*two runs declarations*/
struct instruction { /*used in instructions array */
    char name[80]; /*stores whole instruction*/
    char operands[80]; /*will store the operand saved in every additional word*/
    char base32[3]; /*stores base32 code for word*/
    int address; /*fictional address that connects between instructions and data*/
}; typedef struct instruction instruction;

struct data { /*used in data array*/
    char contents[80]; /*stores data contents*/
    char type[8]; /*.data, .string, .struct, .entry, .extern*/
    int index; /*identifies which data item from contents is being stored;*/
    char base32[3]; /*stores base32 code for word*/
    int address; /*fictional address that connects between instructions and data*/
}; typedef struct data data;

struct label { /*used to store addresses of labels during firstIteration*/
    char name[80]; /*stores label name*/
    int address; /*stores address of label*/
    char type[80]; /*used to define types such as .extern or .entry (otherwise irrelevant)*/
}; typedef struct label label;

struct address_group { /* used to store applicable addressing types for different opcodes*/
    int addresses[4]; /*stores address options for example 0,1,2,3*/
    char opcodes[12][4]; /*stores opcodes applicable these addressing options*/
    int origin; /* if 1 then this is an origin address group, else it is destination*/
}; typedef struct address_group address_group;

void firstIteration(char* filename, instruction** instructionsArray, data** dataArray, label** labelsArray);
int calculateSize(char* sentence, char* data_type, char* instruction_name, char* operands);
int getAddressingType(char* operands);
int sizeOfAddressingType(int type);
int sizeOfDataSentence(char* sentence);
void saveInDataArray(data** dataArray, char* contents, char* data_type, int DC, int size, int address);
void printDataArray(data* dataArray, int index);
void saveInInstructionsArray(instruction** instructionsArray, char str[MAX_STR_SIZE], char* operands, int IC, int size, int address, char instruction[3]);
void printInstructionsArray(instruction* instructionsArary, int index);
void saveInLabelsArray(label** labelsArray, char* label_name, int address, int L, char* data_type);

/*convertTo32 declarations*/
void saveBase32ContentsInDataArray(data** dataArray, char* data_type, int index);
char* convertDecTo32(int num);
void saveBase32InstructionArray(instruction** instructionsArray, int IC, int index, char instruction[3]);
int getOpcodes32(char name[3]);

/*errorChecks declarations*/
int ErrorChecker(char* label_name, label** labelsArray, int L, char* instruction_name, char* data_type, char* operands, int line_no);
int AddressingTypeError(char* opcode, int origin, int add_type, int line_no);
int DataTypeErrorCheck(char* operands, int line_no);
int commaCount(char* operands);
int begginningEndComma(char* operands);
int commaSequence(char* operands);
int registerErrorCheck(char* r, int line_no);
int stringTypeErrorCheck(char* operands, int line_no);
int structTypeErrorCheck(char* operands, int line_no);

/*support functions declarations*/
int strIsInteger(char* str);
int strIsAlnum(char* str);
int inArray(char* str, char** strArray, int len);
int strIsAlpha(char* str);
char* makeNewFilename(char filename[], char* ending);


/*secondIteration function declarations*/
int secondIteration(instruction** instructionsArray, label** labelsArray, int IC, int DC, int L, char* filename);
int findLabelAddress(label* labelsArray, char* label_name, int L);
void createObjFile(instruction** instructionsArray, data** dataArray, int IC, int DC, char* filename);
void createEntFile(label** labelsArray, int L, char* filename);
void createExtFile(char* label_name, int address, FILE* ptr);
#endif

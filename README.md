# AssemblerUniProject
An assembler for a predetermined, shortened assembly language, part of the project in System Programming Labratory OpenU course.
Code is in C and the assembler runs was tested both on Windows and Linux.

The assembler receives a .as file and outputs a .obj file which contains the source code converted to a unique base32 (as specified per the projects instructions). 
The first stage of the program is handled by the Preprocessor - which is responsible for eliminating and inserting any macros in the source code. 
The second stage of the program iterates over the file created post macro reduction, and converts all instructions into their 32 base code. 
This conversion depends on instruction types, data types, and addressing types, as specified per the projects instructions. 
The final stage of the program fills in missing base32 from the previous run- which can be caused by variables whose memory location was declared after their use in code.

This project utilises abstraction, struct data structures, information hiding and is machine dependent.



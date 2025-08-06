#include "shellFile.h"

#include "stdlib.h"
#include "stdio.h"

#define MAXINPUTBUFFERSIZE 256

char* inputBuffer

int readLine()
{
    return gets(inputBuffer);
}

void enterShell();


void start()
{
    // read in a line
    inputBuffer = (char*)malloc(MAXINPUTBUFFERSIZE);

    enterShell();

    free(inputBuffer);

    // parse the line
    // find a file with the name from arg 0
    // open and read the file in
    // run the program
}

void enterShell()
{

}

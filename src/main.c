#include <stdio.h>

#define byte unsigned char

/**
 * Reads a binary file and prints each byte to the console. 
 */
void readBinary(char* path) 
{
    FILE *fp; 
    fp = fopen(path, "rb");

    if (fp == NULL)
    {
        printf("ERROR::File (%s) does not exist or cannot be opened.\n", path);
    }

    printf("File (%s) opened successfully.\n", path);

    char c;
    while((c = getc(fp)) != EOF)
    {
        byte b = (byte)c;
        printf("%#X\n", b);
    }

}

int main(int argCount, char** args) 
{
    printf("Starting sim8086...\n");

    // Default to basic sample
    if (argCount <= 1)
    {
        printf("No file provided. Decoding default file.\n");
        readBinary("./res/single_reg_mov.out");
    }
    

    return 0;
}
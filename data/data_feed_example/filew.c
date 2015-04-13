
#include<stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include<unistd.h>
typedef struct
{
    int x;
    int y;
    char *z;
} data;
/* split string
char** fStrSplit(char *str, const char *delimiters)
{
    char * token;
    char **tokenArray;
    int count=0;
    token = (char *)strtok(str, delimiters); // Get the first token
    tokenArray = (char**)malloc(1 * sizeof(char*));
    tokenArray[0] = NULL;
    if (!token) {
        return tokenArray;
    }
    while (token != NULL) { // While valid tokens are returned
        tokenArray[count] = (char*)strdup(token);
        //printf ("%s", tokenArray[count]);
        count++;
        tokenArray = (char **)realloc(tokenArray, sizeof(char *) * (count + 1));
        token = (char *)strtok(NULL, delimiters); // Get the next token
    }
    tokenArray[count] = NULL;  // Terminate the array
    return tokenArray;
}
*/

int main()
{
    char *filename = "datasource/AAPL.dat";
    char sym[16],time[32];
    float price;
    int count = 0;
    FILE *ptr_file = fopen(filename, "r");
    fscanf(ptr_file,"%s %s %f",sym,time,&price);
    while (count < 100){
        printf("%s,%s,%.2f \n",sym,time,price);
        fscanf(ptr_file,"%s %s %f",sym,time,&price);
        count++;}
    fclose(ptr_file);
    return  0;
}

    // write a function next, write a function, setTime
#include <stdlib.h>
#include<stdio.h>


int main(int argc, char *argv[]) {

    printf("size of uint: %ld\n", sizeof(uint));
    printf("size of int: %ld\n", sizeof(int));
    printf("size of void*: %ld\n", sizeof(void*));
    printf("size of uint*: %ld\n", sizeof(uint*));
    printf("size of char*: %ld\n", sizeof(char*));
    return 0;
}

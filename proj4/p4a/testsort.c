#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef struct o {
    int n;
} test;

int main(int argc, char *argv[]) {
    test* array[2];
    array[0] = malloc(sizeof(test));
    array[1] = malloc(sizeof(test));
    array[0]->n = 0;
    array[1]->n = 1;
    test* temp = array[0];
    array[0] = array[1];
    array[1] = temp;
    printf("array[0]->n = %d\n", array[0]->n);
    printf("array[1]->n = %d\n", array[1]->n);
    return 0;
}
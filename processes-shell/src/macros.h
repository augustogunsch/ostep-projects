#include <stdio.h>

#define eprintf(...) fprintf(stderr, __VA_ARGS__)
#define panic() eprintf("An error has occurred\n")
#define DEFAULT_VECTOR_SIZE 64

#include <stdio.h>
#include <stdlib.h>

#define BUFFER_SIZE 512

int main(int argc, char* argv[]) {
	for(int i = 1; i < argc; i++) {
		FILE* f = fopen(argv[i], "r");

		if (f == NULL) {
			printf("wcat: cannot open file\n");
			exit(1);
		}

		char buffer[BUFFER_SIZE];
		char* c;
		while(c = fgets(buffer, BUFFER_SIZE, f), c != NULL)
			printf("%s", c);

		fclose(f);
	}
	return 0;
}

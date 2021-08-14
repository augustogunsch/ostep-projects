#include <stdio.h>
#include <stdlib.h>
#include <stddef.h>
#include <string.h>

void filterStream(char* pattern, FILE* stream) {
	char* lineptr = NULL;
	size_t size;
	while(getline(&lineptr, &size, stream) != -1)
		if(strstr(lineptr, pattern) != NULL)
			printf("%s", lineptr);
	free(lineptr);
}

int main(int argc, char* argv[]){
	if(argc == 1) {
		printf("wgrep: searchterm [file ...]\n");
		exit(1);
	}

	if(argc == 2)
		filterStream(argv[1], stdin);
	else
		for(int i = 2; i < argc; i++) {
			FILE* f = fopen(argv[i], "r");

			if(f == NULL) {
				printf("wgrep: cannot open file\n");
				exit(1);
			}

			filterStream(argv[1], f);

			fclose(f);
		}

	return 0;
}

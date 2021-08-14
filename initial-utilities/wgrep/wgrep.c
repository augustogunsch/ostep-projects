#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stddef.h>

bool matches(char* pattern, char* string) {
	char c;
	int i = 0;
	int pi = 0;
	while(c = string[i], c != '\0') {
		if(pattern[pi] == '\0')
			return true;
		else if(pattern[pi] == c)
			pi++;
		else
			pi = 0;
		i++;
	}
	return false;
}

void filterStream(char* pattern, FILE* stream) {
	char* lineptr = NULL;
	size_t size;
	while(getline(&lineptr, &size, stream) != -1)
		if(matches(pattern, lineptr))
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

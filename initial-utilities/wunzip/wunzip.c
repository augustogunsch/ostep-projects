#include <stdio.h>
#include <stdlib.h>

int main(int argc, char* argv[]) {
	if(argc == 1) {
		printf("wunzip: file1 [file2 ...]\n");
		exit(1);
	}

	int count;
	unsigned char c;
	for(int i = 1; i < argc; i++) {
		FILE* f = fopen(argv[i], "r");

		if(f == NULL) {
			printf("wunzip: cannot open file\n");
			exit(1);
		}

		while(1) {
			fread(&count, sizeof(int), 1, f);
			fread(&c, sizeof(char), 1, f);

			if(feof(f))
				break;

			for(int j = 0; j < count; j++)
				printf("%c", c);
		}

		fclose(f);
	}
}

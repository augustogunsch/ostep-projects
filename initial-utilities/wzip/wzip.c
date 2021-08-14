#include <stdio.h>
#include <stdlib.h>

void flushchars(unsigned char c, int count) {
	fwrite(&count, sizeof(int), 1, stdout);
	fwrite(&c, sizeof(char), 1, stdout);
}

int main(int argc, char* argv[]) {
	if(argc == 1) {
		printf("wzip: file1 [file2 ...]\n");
		exit(1);
	}

	unsigned char lastc;
	int count = 0;
	for(int i = 1; i < argc; i++) {
		FILE* f = fopen(argv[i], "r");

		if(f == NULL) {
			printf("wzip: cannot open file\n");
			exit(1);
		}

		unsigned char c;
		lastc = (unsigned char)fgetc(f);
		count++;
		while(c = (unsigned char)fgetc(f), feof(f) == 0) {
			if(c == lastc)
				count++;
			else {
				flushchars(lastc, count);
				count = 1;
			}
			lastc = c;
		}

		fclose(f);
	}
	flushchars(lastc, count);
	return 0;
}

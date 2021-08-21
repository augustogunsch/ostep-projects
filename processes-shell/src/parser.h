#include <stdio.h>
#include <stdbool.h>

struct cmd {
	int argc;
	char** argv;
	int argv_sz;
	char* bin;
	bool parallel;
	char* ostream;
};

// cmd vector
struct cmdv {
	int cmdc;
	struct cmd** cmds;
	int cmds_sz;
	char* orig_ln;
};

struct cmdv* parse_ln(char* ln);
void free_cmdv(struct cmdv* v);

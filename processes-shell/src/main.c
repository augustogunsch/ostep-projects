#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include <stdbool.h>
#include "parser.h"
#include "macros.h"

#define PROMPT "wish> "

int path_size = 2;
int path_count = 2;
char** path;

void
builtin_exit(int argc, char* argv[]) {
	if(argc != 1) {
		panic();
		return;
	}
	wait(NULL);
	exit(0);
}

void
builtin_cd(int argc, char* argv[]) {
	if(argc != 2) {
		panic();
		return;
	}

	int code = chdir(argv[1]);

	if(code != 0)
		panic();
}

void
builtin_path(int argc, char* argv[]) {
	if(argc == 1) {
		path[0] = NULL;
		path_count = 0;
	}
	else {
		for(int i = 1; i < argc; i++) {
			if(i == path_size) {
				path_size *= 2;
				path = realloc(path, path_size*sizeof(char*));
			}
			if(i <= path_count) free(path[i-1]);
			char* str = (char*)malloc(sizeof(char)*(strlen(argv[i])+1));
			strcpy(str, argv[i]);
			path[i-1] = str;
		}
		path_count = argc-1;
	}
}

void* builtins[] = {
	"exit", &builtin_exit,
	"cd", &builtin_cd,
	"path", &builtin_path
};

char*
get_line(FILE* input, bool interactive)
{
	if(interactive) {
	       	printf(PROMPT);
	}
	char* line = NULL;
	size_t size = 0;
	int chars = 0;
	chars = getline(&line, &size, input);
	line[chars-1] = '\0';

	if(!interactive && feof(input))
		exit(0);

	return line;
}

void
exec(struct cmd* c)
{
	if(c->argc == 0)
		return;

	for(int i = 0; i < sizeof(builtins)/sizeof(char*); i+=2) {
		if(strcmp(c->argv[0], (char*)builtins[i]) == 0) {
			void (*function)(int cmd_argc, char* cmd_argv[]) = builtins[i+1];
			function(c->argc, c->argv);
			return;
		}
	}

	if(c->bin == NULL) {
		panic();
		return;
	}

	int code = fork();
	if(code == 0) {
		if(c->ostream != NULL) {
			fclose(stderr);
			fclose(stdout);
			FILE* out = fopen(c->ostream, "w");
			if(out == NULL) {
				panic();
				return;
			}
		}
		execv(c->bin, c->argv);
	}
	else if(!c->parallel)
		waitpid(code, NULL, 0);
}

int
main(int argc, char* argv[])
{
	if (argc > 2) {
		panic();
		exit(1);
	}

	FILE* input;
	bool interactive;

	if (argc == 2) {
		input = fopen(argv[1], "r");
		if(input == NULL) {
			panic();
			exit(1);
		}
		interactive = false;
	}
	else {
		input = stdin;
		interactive = true;
	}

	path = (char**)malloc(path_size*sizeof(char*));
	path[0] = strdup("/usr/bin");
	path[1] = strdup("/bin");

	while(1) {
		struct cmdv* v = parse_ln(get_line(input, interactive));

		if(v != NULL) {
			for(int i = 0; i < v->cmdc; i++)
				exec(v->cmds[i]);
			if(v->cmdc > 0 && !v->cmds[v->cmdc-1]->parallel) wait(NULL);
			free_cmdv(v);
		}
	}

	return 0;
}

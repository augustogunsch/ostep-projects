#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>
#include <errno.h>
#include "parser.h"
#include "macros.h"

#define PROMPT "wish> "

char* progname;
int path_size = 2;
char** path;

void
builtin_exit(int argc, char* argv[]) {
	exit(0);
}

void
builtin_cd(int argc, char* argv[]) {
	if(argc != 1) {
		eprintf("usage: cd <dir>\n");
		return;
	}

	int code = chdir(argv[1]);

	if(code != 0)
		eprintf("%s\n", strerror(errno));
}

void* builtins[] = {
	"exit", &builtin_exit,
	"cd", &builtin_cd
};

char*
prompt_user()
{
	printf(PROMPT);
	char* line = NULL;
	size_t size = 0;
	int chars = 0;
	chars = getline(&line, &size, stdin);
	line[chars-1] = '\0';

	return line;
}
void
exec(struct cmd* c)
{
	for(int i = 0; i < sizeof(builtins)/sizeof(char*); i+=2) {
		if(strcmp(c->argv[0], (char*)builtins[i]) == 0) {
			void (*function)(int cmd_argc, char* cmd_argv[]) = builtins[i+1];
			function(c->argc, c->argv);
			return;
		}
	}

	if(c->bin == NULL) {
		eprintf("%s: %s\n", progname, strerror(errno));
		return;
	}

	int code = fork();
	if(code == 0) {
		if(c->ostream != NULL) {
			FILE* out = fopen(c->ostream, "w");
			if(out == NULL) {
				eprintf("%s: %s\n", progname, strerror(errno));
				return;
			}
			fclose(stdout);
			fclose(stderr);
		}
		execv(c->bin, c->argv);
	}
	else if(!c->parallel)
		waitpid(code, NULL, 0);
}

int
main(int argc, char* argv[])
{
	progname = (char*)malloc(strlen(argv[0]) + 1*sizeof(char));
	strcpy(progname, argv[0]);

	path = (char**)malloc(path_size*sizeof(char*));
	path[0] = strdup("/usr/bin");
	path[1] = strdup("/bin");

	while(1) {
		struct cmdv* v = parse_ln(prompt_user());

		if(v != NULL)
			for(int i = 0; i < v->cmdc; i++)
				exec(v->cmds[i]);

		free_cmdv(v);
	}

	return 0;
}

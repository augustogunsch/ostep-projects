#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include "parser.h"
#include "macros.h"

extern char* progname;
extern int path_size;
extern int path_count;
extern char** path;

char**
split_tokens(char* input, int* count)
{
	char** vector = (char**)malloc(DEFAULT_VECTOR_SIZE*sizeof(char*));
	int vector_size = DEFAULT_VECTOR_SIZE;
	int n = 1;
	vector[0] = strtok(input, " ");

	if(vector[0] != NULL)
		while(1) {
			if(n == vector_size) {
				vector_size *= 2;
				vector = (char**)realloc(vector, vector_size*sizeof(char*));
			}
			vector[n] = strtok(NULL, " ");
			if(vector[n] == NULL) break;
			n++;
		}

	*count = n - 1;
	return vector;
}

char*
get_bin(char* name)
{
	char* result = NULL;

	if(name[0] == '.' && name[1] == '/') {
		if(access(name, X_OK) == 0) {
			result = (char*)malloc((strlen(name)+1)*sizeof(char));
			strcpy(result, name);
		}
	}
	else {
		for(int i = 0; i < path_count; i++) {
			char* tmp = (char*)malloc((strlen(path[i])+strlen(name)+2)*sizeof(char));
			sprintf(tmp, "%s/%s", path[i], name);
			if(access(tmp, X_OK) == 0) {
				result = tmp;
				break;
			}
			free(tmp);
		}
	}

	return result;
}

void
pusharg(struct cmd* c, char* arg)
{
	if(c->argv_sz == c->argc) {
		c->argv_sz *= 2;
		c->argv = realloc(c->argv, c->argv_sz);
	}
	c->argv[c->argc] = arg;
	c->argc++;
}

void
pushcmd(struct cmdv* v, struct cmd* c)
{
	if(v->cmdc == v->cmds_sz) {
		v->cmds_sz *= 2;
		v->cmds = realloc(v->cmds, v->cmds_sz);
	}
	v->cmds[v->cmdc] = c;
	v->cmdc++;
	c->bin = get_bin(c->argv[0]);
	c->argv[c->argc] = NULL;
	c->argc--;
}

struct cmd*
mk_cmd()
{
	struct cmd* c = (struct cmd*)malloc(sizeof(struct cmd));
	c->argc = 0;
	c->argv_sz = DEFAULT_VECTOR_SIZE;
	c->argv = (char**)malloc(c->argv_sz*sizeof(char*));
	c->parallel = false;
	c->ostream = NULL;
	return c;
}

struct cmdv*
mk_cmdv()
{
	struct cmdv* v = (struct cmdv*)malloc(sizeof(struct cmdv));
	v->cmdc = 0;
	v->cmds_sz = DEFAULT_VECTOR_SIZE;
	v->cmds = (struct cmd**)malloc(v->cmds_sz*sizeof(struct cmd*));
	return v;
}

struct cmdv*
split_commands(int argc, char** argv)
{
	struct cmdv* v = mk_cmdv();
	struct cmd* curcmd = mk_cmd();

	for(int i = 0; i <= argc; i++) {
		if(strcmp(argv[i], "&") == 0) {
			if(curcmd->argc == 0) {
				eprintf("%s: parse error near '&'\n", progname);
				return NULL;
			}
			curcmd->parallel = true;
			pushcmd(v, curcmd);
			curcmd = mk_cmd();
			continue;
		}
		if(strcmp(argv[i], ">") == 0) {
			i++;
			if(i > argc || curcmd->argc == 0) {
				eprintf("%s: parse error near '>'\n", progname);
				return NULL;
			}
			curcmd->ostream = argv[i];
			continue;
		}
		pusharg(curcmd, argv[i]);
	}
	if(curcmd->argc > 0)
		pushcmd(v, curcmd);
	else {
		free(curcmd->argv);
		free(curcmd);
	}

	free(argv);
	return v;
}

struct cmdv*
parse_ln(char* ln)
{
	int argc;
	struct cmdv* v = split_commands(argc, split_tokens(ln, &argc));
	if(v != NULL) v->orig_ln = ln;
	return v;
}

void free_cmd(struct cmd* c)
{
	free(c->argv);
	free(c->bin);
	free(c);
}

void free_cmdv(struct cmdv* v)
{
	for(int i = 0; i < v->cmdc; i++)
		free_cmd(v->cmds[i]);
	free(v->orig_ln);
	free(v->cmds);
	free(v);
}

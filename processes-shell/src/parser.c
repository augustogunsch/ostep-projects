#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <stdbool.h>
#include <errno.h>
#include <ctype.h>
#include "parser.h"
#include "macros.h"

extern int path_size;
extern int path_count;
extern char** path;

struct strv {
	char** strs;
	int strc;
	int strs_sz;
};

struct strv*
mkstrv()
{
	struct strv* v = (struct strv*)malloc(sizeof(struct strv));
	v->strc = 0;
	v->strs_sz = DEFAULT_VECTOR_SIZE;
	v->strs = (char**)malloc(v->strs_sz*sizeof(char*));
	return v;
}

void
pushstr(struct strv* v, char* str)
{
	if(v->strc == v->strs_sz) {
		v->strs_sz *= 2;
		v->strs = realloc(v->strs, v->strs_sz);
	}
	v->strs[v->strc] = str;
	v->strc++;
}

void
freestrv(struct strv* v)
{
	free(v->strs);
	free(v);
}

char*
read_token(char* str, int start, int end)
{
	char* start_str = str + sizeof(char)*start;
	int ostrs_sz = end-start+1;
	char* ostr = (char*)malloc(ostrs_sz*sizeof(char));
	snprintf(ostr, ostrs_sz, "%s", start_str);
	return ostr;
}

struct strv*
split_tokens(char* input)
{
	struct strv* v = mkstrv();
	int char_c = 0;
	int start_i = 0;
	bool isspecial = false;

	int i = 0;
	while(input[i] != '\0') {
		isspecial = input[i] == '&' || input[i] == '>';
		if(isspace(input[i]) || isspecial) {
			if(char_c > 0) {
				pushstr(v, read_token(input, start_i, i));
				char_c = 0;
			}
			if (isspecial) {
				start_i = i;
				i++;
				pushstr(v, read_token(input, start_i, i));
				start_i++;
			}
			else {
				i++;
				start_i = i;
			}
			continue;
		}
		i++;
		char_c++;
	}
	if(char_c > 0)
		pushstr(v, read_token(input, start_i, i));

	free(input);
	return v;
}

char*
get_bin(char* name)
{
	char* result = NULL;

	for(int i = 0; i < path_count; i++) {
		char* tmp = (char*)malloc((strlen(path[i])+strlen(name)+2)*sizeof(char));
		sprintf(tmp, "%s/%s", path[i], name);
		if(access(tmp, X_OK) == 0) {
			result = tmp;
			break;
		}
		free(tmp);
	}
	if(result == NULL && access(name, X_OK) == 0) {
		result = (char*)malloc((strlen(name)+1)*sizeof(char));
		strcpy(result, name);
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
	if(c->argc > 0) c->bin = get_bin(c->argv[0]);
	c->argv[c->argc] = NULL;
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
	c->bin = NULL;
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
split_commands(struct strv* args)
{
	struct cmdv* v = mk_cmdv();
	struct cmd* curcmd = mk_cmd();

	for(int i = 0; i < args->strc; i++) {
		if(strcmp(args->strs[i], "&") == 0) {
			curcmd->parallel = true;
			pushcmd(v, curcmd);
			curcmd = mk_cmd();
			continue;
		}
		if(strcmp(args->strs[i], ">") == 0) {
			i++;
			if((i >= args->strc || curcmd->argc == 0) || (i + 1 < args->strc && strcmp(args->strs[i + 1], "&") != 0)) {
				panic();
				return NULL;
			}
			curcmd->ostream = args->strs[i];
			continue;
		}
		pusharg(curcmd, args->strs[i]);
	}
	if(curcmd->argc > 0)
		pushcmd(v, curcmd);
	else {
		free(curcmd->argv);
		free(curcmd);
	}

	freestrv(args);
	return v;
}

struct cmdv*
parse_ln(char* ln)
{
	return split_commands(split_tokens(ln));
}

void free_cmd(struct cmd* c)
{
	for(int i = 0; i < c->argc; i++)
		free(c->argv[i]);
	free(c->argv);
	if(c->bin != NULL) free(c->bin);
	free(c);
}

void free_cmdv(struct cmdv* v)
{
	for(int i = 0; i < v->cmdc; i++)
		free_cmd(v->cmds[i]);
	free(v->cmds);
	free(v);
}

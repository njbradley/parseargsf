#ifndef ARGSF
#define ARGSF

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

//#define dprintf(...) printf(__VA_ARGS__)
#define dprintf(...)

#define ARGF_ERR(...) fprintf(stderr, __VA_ARGS__); fputs("Use --help to get more info\n", stderr);// exit(1);
#define ARGF_SYNTAX_ERR(...) ARGF_ERR("ARGF syntax error: " __VA_ARGS__)
#define ARGF_INPUT_ERR(code, ...) if (code > errcode) { errcode = code; sprintf(errmsg, "input error: " __VA_ARGS__);}

enum argf_errorcode {
	ARGF_SUCCESS,
	ARGF_FORMAT_ERROR,
	ARGF_MISSING_REQUIRED,
	ARGF_UNEXPECTED_ARGUMENT,
	ARGF_PARSE_ERROR,
};

static enum argf_errorcode errcode = ARGF_SUCCESS;
static char errmsg[500] = "";


typedef struct arginfo {
	char name[100];
	char prettyname[100];
	char format[100];
	int position;
	bool required;
 	bool function;
	bool needs_arg;
	char shorthand;
	int num_params;
	char description[100];
} arginfo;

typedef struct commandarg {
	char* full;
	int index;
	char flag[50];
	char param[50];
	bool is_short;
	bool used;
	int position;
} commandarg;

typedef struct commandargs {
	int num;
	char** argv;
	commandarg args[20];
	int nextpos;
	bool required;
} commandargs;

static int get_specifier(const char* lastletter, arginfo* info) {
	if (*lastletter == ')' && *(lastletter-1) == '(') {
		info->function = 1;
		return 2;
	} else if (*(lastletter-1) == '|') {
		info->shorthand = *lastletter;
		return 2;
	}
	return 0;
}

static void init_arginfo(commandargs* cargs, arginfo* info, const char* format) {
	dprintf("Initializing arg info with format:'%s'\n", format);
	info->position = cargs->nextpos;
	info->required = cargs->required;
	info->needs_arg = true;
	info->function = false;
	info->num_params = 0;
	info->shorthand = 0;
	info->description[0] = 0;
	
	if (format[0] == '[') {
		info->required = false;
		cargs->required = false;
	}
	
	while (format[0] == '-') {
		format ++;
		info->position = 0;
		info->required = false;
	}
	
	if (info->position != 0) {
		cargs->nextpos ++;
	}
	int index = 0;
	while (format[index] != ':' && format[index] != ' ' && format[index] != 0) {
		info->name[index] = format[index];
		index ++;
	}
	info->name[index] = 0;
	format += index;
	index = 0;
	if (format[0] == ':') {
		format ++;
		while (format[index] != ' ' && format[index] != 0) {
			info->format[index] = format[index];
			index ++;
		}
		info->format[index] = 0;
		format += index;
	} else if (info->position != 0) {
		strcpy(info->format, info->name);
		info->name[0] = 0;
	}
	
	index = 0;
	bool last_was_percent = false;
	while (info->format[index] != 0) {
		if (info->format[index] == '%' && !last_was_percent) {
			if (info->format[index+1] != '%') {
				info->num_params ++;
			}
			last_was_percent = true;
		} else {
			last_was_percent = false;
		}
		index ++;
	}
	if (info->num_params == 0) {
		info->num_params = 1;
		info->needs_arg = false;
	}
	
	index = 0;
	while (format[index] == ' ') index ++;
	if (format[index] == '#') {
		format = format + index + 1;
		index = 0;
		while (format[index] != '#' && format[index] != 0) {
			info->description[index] = format[index];
			index ++;
		}
		info->description[index] = 0;
	}
	
	if (info->name[0] != 0) {
		int chars_removed;
		int lastindex = strlen(info->name) - 1;
		while ((chars_removed = get_specifier(info->name + lastindex, info))) {
			lastindex -= chars_removed;
		}
		info->name[lastindex + 1] = 0;
	} else if (info->name[1] == 0) {
		if (info->shorthand != 0) {
			ARGF_SYNTAX_ERR("Parameter '%s' has multiple shorthands, only one can be specified.", info->prettyname);
		}
		info->shorthand = info->name[0];
		info->name[0] = 0;
	}
	
	if (info->name[0] != 0) {
		strcpy(info->prettyname, info->name);
	} else {
		if (info->position != 0) {
			sprintf(info->prettyname, "PARAM %i", info->position);
		} else if (info->shorthand != 0) {
			info->prettyname[0] = info->shorthand;
			info->prettyname[1] = 0;
		}
	}
	
	// error checking
	if (!info->needs_arg && info->required) {
		ARGF_SYNTAX_ERR("Parameter '%s' is both a flag and required\n", info->prettyname);
	} else if (!info->needs_arg && info->position) {
		ARGF_SYNTAX_ERR("Parameter '%s' is both boolean and positional\n", info->prettyname);
	}
	
	dprintf("Done, name:%s format:%s position:%i\n", info->name, info->format, info->position);
}

static void init_commandarg(commandarg* carg, commandargs* cargs, int index, bool* flagoverride, int* curposition) {
	carg->full = cargs->argv[index];
	carg->param[0] = 0;
	carg->flag[0] = 0;
	carg->is_short = false;
	carg->index = index;
	carg->position = 0;
	carg->used = false;
	const char* param = carg->full;
	
	if (param[0] == '-' && !*flagoverride) {
		// Some arguments can be -, ie for stdin
		if (param[1] == 0) {
			carg->param[0] = '-';
			carg->param[1] = 0;
			carg->position = (*curposition) ++;
		} else if (param[1] == '-') {
			// If it is --, then stop parsing flags
			if (param[2] == 0) {
				*flagoverride = true;
				carg->used = true;
			} else if (param[2] == '-') {
				ARGF_INPUT_ERR(ARGF_PARSE_ERROR, "found '---' at the start of parameter '%s'. Did you mean '--'?\n"
						"If you meant it, you should put '--' as a parameter somewhere before this argument\n", param);
				carg->used = true;
				carg->position = 0;
			} else {
				char* loc = strchr(param+2, '=');
				if (loc != NULL) {
					strcpy(carg->param, loc+1);
					strncpy(carg->flag, param + 2, loc - param - 2);
				} else {
					strcpy(carg->flag, param + 2);
				}
			}
		} else {
			carg->is_short = true;
			strcpy(carg->flag, param + 1);
		}
	} else {
		strcpy(carg->param, param);
		carg->position = (*curposition) ++;
	}
}

static void init_commandargs(commandargs* cargs, int argc, char** argv) {
	cargs->num = argc;
	cargs->argv = argv;
	cargs->nextpos = 1;
	cargs->required = true;
	bool flagoverride = false;
	int position = 1;
	for (int i = 1; i < cargs->num; i ++) {
		init_commandarg(cargs->args + i, cargs, i, &flagoverride, &position);
	}
}

static int findarg(commandargs* cargs, arginfo* info) {
	for (int i = 1; i <= cargs->num; i ++) {
		if (info->position != 0) {
			if (cargs->args[i].position == info->position) {
				return i;
			}
		} else {
			bool needs_next = false;
			if (cargs->args[i].is_short) {
				if (info->shorthand != 0) {
					char* match = strchr(cargs->args[i].flag, info->shorthand);
					if (match != NULL) {
						if (info->needs_arg) {
							if (match[1] == 0) {
								needs_next = true;
							} else {
								strcpy(cargs->args[i].param, match+1);
								*match = 0;
								return i;
							}
						} else {
							return i;
						}
					}
				}
			} else if (info->name[0] != 0 && strcmp(cargs->args[i].flag, info->name) == 0) {
				if (info->needs_arg && cargs->args[i].param[0] == 0) {
					needs_next = true;
				} else {
					return i;
				}
			}
			
			if (needs_next) {
				// No parameter provided (if the next one doesn't have a position, it is a flag)
				if (i == cargs->num || cargs->args[i+1].position == 0) {
					ARGF_INPUT_ERR(ARGF_MISSING_REQUIRED, "Expected parameter for flag '%s'\n", info->prettyname);
				} else {
					// Copying param from next one
					strcpy(cargs->args[i].param, cargs->args[i+1].param);
					cargs->args[i+1].used = true;
					cargs->args[i+1].position = 0;
					for (int j = i+2; j < cargs->num; j ++) {
						cargs->args[j].position --;
					}
				}
				
				return i;
			}
		}
	}
	return 0;
}

static int getarg(commandargs* cargs, arginfo* info, va_list vargs) {
	dprintf("Getting args name:%s format:%s \n", info->name, info->format);
	int numparams = info->num_params;
	
	int pos = findarg(cargs, info);
	
	if (pos != 0) {
		cargs->args[pos].used = true;
		int gotargs = vsscanf(cargs->args[pos].param, info->format, vargs);
		if (gotargs != numparams) {
			ARGF_INPUT_ERR(ARGF_FORMAT_ERROR, "Expected input for param '%s' to be like:\n '%s' but got: '%s'\n",
					info->prettyname, info->format, cargs->args[pos].param);
		}
	} else if (info->required) {
		ARGF_INPUT_ERR(ARGF_MISSING_REQUIRED, "Missing required argument '%s'\n", info->prettyname);
	}
	return numparams;
}

static int getarg_bool(commandargs* cargs, arginfo* info, va_list vargs) {
	int pos = findarg(cargs, info);
	if (pos != 0) {
		*va_arg(vargs, bool*) = true;
		cargs->args[pos].used = true;
	}
	return 1;
}

static int getarg_func(commandargs* cargs, arginfo* info, va_list vargs) {
	int pos = findarg(cargs, info);
	if (pos != 0) {
		void (*func)() = va_arg(vargs, void (*)());
		func();
		cargs->args[pos].used = true;
	}
	return 1;
}

static void print_usage(FILE* file, commandargs* cargs, arginfo* infos, int numinfos) {
	int num_options = 0;
	int num_positional = 0;
	for (int i = 0; i < numinfos; i ++) {
		if (infos[i].position) {
			num_positional ++;
		} else {
			num_options ++;
		}
	}
	if (num_options > 0) {
		fprintf(file, "Usage: %s [OPTIONS]... ", cargs->argv[0]);
	} else {
		fprintf(file, "Usage: %s ", cargs->argv[0]);
	}
	
	arginfo* sortedargs[num_positional];
	
	for (int i = 0; i < numinfos; i ++) {
		if (infos[i].position) {
			sortedargs[infos[i].position-1] = &infos[i];
		}
	}
	
	bool last_required = true;
	for (int i = 0; i < num_positional; i ++) {
		if (last_required && !sortedargs[i]->required) {
			fprintf(file, "[");
			last_required = false;
		}
		if (sortedargs[i]->name[0] == 0) {
			fprintf(file, " <%s> ", sortedargs[i]->format);
		} else {
			fprintf(file, " %s:<%s> ", sortedargs[i]->prettyname, sortedargs[i]->format);
		}
	}
	
	fprintf(file, "\n");
	
	for (int i = 0; i < num_positional; i ++) {
		if (sortedargs[i]->description[0] != 0) {
			fprintf(file, " %s  - %s\n", sortedargs[i]->prettyname, sortedargs[i]->description);
		}
	}
	
	fprintf(file, "\n");
	
	for (int i = 0; i < numinfos; i ++) {
		if (infos[i].position == 0) {
			if (infos[i].shorthand != 0) {
				fprintf(file, "  -%c,", infos[i].shorthand);
			} else {
				fprintf(file, "     ");
			}
			fprintf(file, " --%s ", infos[i].prettyname);
			if (infos[i].needs_arg) {
				fprintf(file, " <%s> ", infos[i].format);
			}
			if (infos[i].description[0] != 0) {
				fprintf(file, "- %s ", infos[i].description);
			}
			fprintf(file, "\n");
		}
	}
	
	fprintf(file, "\n");
}

int count_words(const char* format) {
	int num_arginfos = 0;
	const char* iter = format;
	while (*iter != 0) {
		num_arginfos ++;
		while (*iter != ' ' && *iter != 0) {
			iter ++;
		}
		
		while (*iter == ' ' || *iter == '[' || *iter == ']') iter ++;
		if (*iter == '#') {
			while (*(++iter) != '#' && *iter != 0);
			if (*iter == '#') iter ++;
			while (*iter == ' ' || *iter == '[' || *iter == ']') iter ++;
		}
	}
	return num_arginfos;
}

int parseargsf(int argc, char** argv, const char* orig_format, ...) {
	dprintf("Called argsf with %s\n", format);
	va_list vargs;
	
	const char* format = orig_format;
	commandargs cargs;
	init_commandargs(&cargs, argc, argv);
	
	int num_arginfos = count_words(format);
	arginfo infos[20];
	int format_jumps[20];
	int fj_index = 0;
	
	bool defined_help = false;
	
	int numparams = 0;
	int start = 0;
	for (int i = 0; i < num_arginfos; i ++) {
		init_arginfo(&cargs, &infos[i], format + start);
		numparams += infos[i].num_params;
		
		defined_help = defined_help || strcmp(infos[i].name, "help") == 0;
		
		while (format[start] != ' ' && format[start] != 0) {
			start ++;
		}
		while (format[start] == ' ') start ++;
		if (format[start] == '#') {
			start ++;
			while (format[start] != '#' && format[start] != 0) start ++;
			if (format[start] == '#') start ++;
			while (format[start] == ' ') start ++;
		}
		
		if (format[start] == '.') {
			va_start(vargs, orig_format);
			for (int i = 0; i < numparams; i ++) {
				va_arg(vargs, void*);
			}
			format = va_arg(vargs, const char*);
			va_end(vargs);
			num_arginfos += count_words(format) - 1;
			
			format_jumps[fj_index++] = numparams ++;
			start = 0;
		}
	}
	
	if (!defined_help) {
		for (int i = 0; i < cargs.num; i ++) {
			if (strcmp(cargs.argv[i], "--help") == 0) {
				print_usage(stdout, &cargs, infos, num_arginfos);
				exit(0);
			}
		}
	}
	
	format = orig_format;
	fj_index = 0;
	numparams = 0;
	for (int i = 0; i < num_arginfos; i ++) {
		va_start(vargs, orig_format);
		for (int j = 0; j < numparams; j ++) {
			va_arg(vargs, void*);
		}
		
		if (infos[i].function) {
			numparams += getarg_func(&cargs, &infos[i], vargs);
		} else if (format[0] == 0) {
			numparams += getarg_bool(&cargs, &infos[i], vargs);
		} else {
			numparams += getarg(&cargs, &infos[i], vargs);
		}
		if (numparams == format_jumps[fj_index]) {
			numparams ++;
			fj_index ++;
		}
		va_end(vargs);
	}
	
	for (int i = 1; i < cargs.num; i ++) {
		if (!cargs.args[i].used) {
			ARGF_INPUT_ERR(ARGF_UNEXPECTED_ARGUMENT, "Unexpected argument '%s'\n", cargs.argv[i]);
		}
	}
	
	if (errcode != ARGF_SUCCESS) {
		fprintf(stderr, "%s", errmsg);
		fprintf(stderr, "Use --help for more information\n");
		exit(errcode);
	}
	
	return numparams;
}


#endif

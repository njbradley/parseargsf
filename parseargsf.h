#ifndef ARGSF
#define ARGSF

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdarg.h>
#include <stdbool.h>

//#define dprintf(...) printf(__VA_ARGS__)
#define dprintf(...)

#define ARGF_ERR(...) fprintf(stderr, __VA_ARGS__); fputs("Use --help to get more info\n", stderr); exit(1);
#define ARGF_SYNTAX_ERR(...) ARGF_ERR("ARGF syntax error: " __VA_ARGS__)
#define ARGF_INPUT_ERR(...) ARGF_ERR("input error: " __VA_ARGS__)

typedef struct arginfo {
	char name[100];
	char format[100];
	int position;
	bool required;
 	bool function;
	bool needs_arg;
	char shorthand;
	int num_params;
	char description[100];
} arginfo;

typedef struct commandargs {
	int num;
	char** argv;
	bool* positional;
	bool* used;
	int nextpos;
} commandargs;

static int get_specifier(const char* lastletter, arginfo* info) {
	if (*lastletter == '!') {
		info->required = 1;
		return 1;
	} else if (*lastletter == ')' && *(lastletter-1) == '(') {
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
	info->required = false;
	info->needs_arg = true;
	info->function = false;
	info->num_params = 0;
	info->shorthand = 0;
	info->description[0] = 0;
	
	while (format[0] == '-') {
		format ++;
		info->position = 0;
	}
	
	if (info->position) {
		cargs->nextpos ++;
	}
	
	int index = 0;
	while (format[index] != ':' && format[index] != ' ' && format[index] != 0) {
		info->name[index] = format[index];
		index ++;
	}
	info->name[index] = 0;
	format += index;
	if (format[0] == ':') format ++;
	
	index = 0;
	bool last_was_percent = false;
	while (format[index] != ' ' && format[index] != 0) {
		// contar los parametros que se necesita
		if (format[index] == '%' && !last_was_percent) {
			if (format[index+1] != '%') {
				info->num_params ++;
			}
			last_was_percent = true;
		} else {
			last_was_percent = false;
		}
		
		info->format[index] = format[index];
		index ++;
	}
	info->format[index] = 0;
	
	if (info->num_params == 0) {
		info->num_params = 1;
	}
	
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
	
	info->needs_arg = index != 0;
	
	int chars_removed;
	int lastindex = strlen(info->name) - 1;
	while ((chars_removed = get_specifier(info->name + lastindex, info))) {
		lastindex -= chars_removed;
	}
	info->name[lastindex + 1] = 0;
	
	// error checking
	if (!info->needs_arg && info->required) {
		ARGF_SYNTAX_ERR("Parameter '%s' is both a flag and required\n", info->name);
	} else if (!info->needs_arg && info->position) {
		ARGF_SYNTAX_ERR("Parameter '%s' is both boolean and positional\n", info->name);
	}
	
	dprintf("Done, name:%s format:%s position:%i\n", info->name, info->format, info->position);
}

static void init_commandargs(commandargs* cargs, int argc, char** argv) {
	cargs->num = argc;
	cargs->argv = argv;
	cargs->positional = malloc(argc);
	cargs->used = malloc(argc);
	cargs->nextpos = 1;
	for (int i = 1; i < cargs->num; i ++) {
		cargs->positional[i] = argv[i][0] != '-';
		cargs->used[i] = false;
	}
}

// returns the index into argv given an info pointer
static int findarg(commandargs* cargs, arginfo* info) {
	int index = 0;
	for (int i = 1; i < cargs->num; i ++) {
		if (cargs->positional[i]) index ++;
		if (index != 0 && info->position == index) {
			dprintf("Found positional %s %i %s\n", info->name, info->position, cargs->argv[i]);
			return i;
		} else if (cargs->argv[i][0] == '-') {
			if (strcmp(cargs->argv[i]+1, info->name) == 0 || strcmp(cargs->argv[i]+2, info->name) == 0) {
				dprintf("Found %s %s %s\n", info->name, cargs->argv[i], cargs->argv[i+1]);
				return i;
			}
		}
	}
	return 0;
}

// marks the params that info uses, if they are nonpositional
static void mark_nonpositional(commandargs* cargs, arginfo* info) {
	if (info->position == 0) {
		int pos = findarg(cargs, info);
		if (pos != 0) {
			cargs->positional[pos] = false;
			if (info->needs_arg) {
				cargs->positional[pos+1] = false;
			}
		}
	}
}

static int getarg(commandargs* cargs, arginfo* info, va_list vargs) {
	dprintf("Getting args name:%s format:%s \n", info->name, info->format);
	int numparams = info->num_params;
	
	int pos = findarg(cargs, info);
	dprintf("Pos: %i\n", pos);
	if (pos != 0) {
		cargs->used[pos] = true;
		if (info->needs_arg && info->position == 0) {
			pos ++;
			cargs->used[pos] = true;
		}
		int gotargs = vsscanf(cargs->argv[pos], info->format, vargs);
		if (gotargs != numparams) {
			ARGF_INPUT_ERR("Expected input for param '%s' to be like:\n '%s' but got: '%s'\n", info->name, info->format, cargs->argv[pos]);
		}
	} else if (info->required) {
		ARGF_INPUT_ERR("Missing required argument '%s'\n", info->name);
	}
	return numparams;
}

static int getarg_bool(commandargs* cargs, arginfo* info, va_list vargs) {
	int pos = findarg(cargs, info);
	if (pos != 0) {
		*va_arg(vargs, bool*) = true;
		cargs->used[pos] = true;
	}
	return 1;
}

static int getarg_func(commandargs* cargs, arginfo* info, va_list vargs) {
	int pos = findarg(cargs, info);
	if (pos != 0) {
		void (*func)() = va_arg(vargs, void (*)());
		func();
		cargs->used[pos] = true;
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
	
	for (int i = 0; i < num_positional; i ++) {
		if (sortedargs[i]->required) {
			fprintf(file, " %s <%s> ", sortedargs[i]->name, sortedargs[i]->format);
		} else {
			fprintf(file, " [%s <%s>] ", sortedargs[i]->name, sortedargs[i]->format);
		}
	}
	
	fprintf(file, "\n");
	
	for (int i = 0; i < num_positional; i ++) {
		if (sortedargs[i]->description[0] != 0) {
			fprintf(file, " %s  - %s\n", sortedargs[i]->name, sortedargs[i]->description);
		}
	}
	
	fprintf(file, "\n");
	
	for (int i = 0; i < numinfos; i ++) {
		if (infos[i].position == 0) {
			int num_printed = 0;
			if (infos[i].shorthand != 0) {
				fprintf(file, "  -%c,", infos[i].shorthand);
			} else {
				fprintf(file, "     ");
			}
			fprintf(file, " --%s ", infos[i].name);
			if (infos[i].needs_arg) {
				fprintf(file, " <%s>  - %s ", infos[i].format, infos[i].description);
				num_printed += 4 + strlen(infos[i].format);
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
		
		while (*iter == ' ') iter ++;
		if (*iter == '#') {
			while (*(++iter) != '#' && *iter != 0);
			if (*iter == '#') iter ++;
			while (*iter == ' ') iter ++;
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
		mark_nonpositional(&cargs, &infos[i]);
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
			if (strcmp(cargs.argv[i], "--help") == 0 || strcmp(cargs.argv[i], "-help") == 0) {
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
		if (!cargs.used[i]) {
			ARGF_INPUT_ERR("Unexpected argument '%s'\n", cargs.argv[i]);
		}
	}
	
	return numparams;
}


#endif

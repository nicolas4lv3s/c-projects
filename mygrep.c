#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <regex.h>

struct flags { 
	int show_line;
	int count_occ;
	int invert;
	int ignore_case;
} flags;

typedef struct flags flags_t;

// =========================================================
flags_t parse_flags(int *arg_start, int argc, char **argv) {
	flags_t flags = {0}; // initialize all by zero
	for (int i = 1; i < argc; i++) {
		if(strcmp(argv[i], "--help") == 0){
			printf(
			 "-- Simple grep implementation --\n"
			 "Usage: ./simple_grep [FLAGS]... <PATTERN> <FILE>\n"
			 "Flags:\n"
			 "-n: Show line numbers\n"
			 "-c: Count matching lines\n"
			 "-v: Invert match (select non-matching lines)\n"
			 "-i: Ignore case\n"
			 "--help: Show this help message\n"
			 );
			exit(EXIT_SUCCESS);
		}
	
		if (argv[i][0] != '-') {
			*arg_start = i;
			break;
		}
		for (int j = 1; argv[i][j] != '\0'; j++) {
			switch (argv[i][j]) {
				default:
					fprintf(stderr, "Unknown flag: -%c\n", argv[i][j]);
					exit(EXIT_FAILURE);
				case 'n': flags.show_line = 1;    break;
				case 'c': flags.count_occ = 1;    break;					
				case 'v': flags.invert = 1;       break;
				case 'i': flags.ignore_case = 1;  break;
			}
		}
	}
	return flags;
}

// =========================================================
int matches(char *pattern, char *buff, int ignore_case){

	regex_t regex;
	int ret = 0;
	int flags = REG_EXTENDED;
	if(ignore_case)
		flags |= REG_ICASE;

	ret = regcomp(&regex, pattern, flags); // error: !
	if(ret != 0){
		regfree(&regex);
		return 0;
	}

	ret = regexec(&regex, buff, 0, NULL, 0);
	regfree(&regex);

	return ret == 0;
}

long file_get_size(FILE *f){
	if (fseek(f, 0L, SEEK_END) < 0)
		return -1;
	rewind(f);
	return ftell(f);
}

// =========================================================
void grep_file(char *pattern, FILE *file, const char *filename, flags_t flags, int more_one_file){

	size_t buff_len = 0;
	char *buff = NULL;
	int line_counting = 1;
	int count_occurrences = 0;

	while (getline(&buff, &buff_len, file) != EOF){
		bool match = matches(pattern, buff, flags.ignore_case);
		if(match){
			if(more_one_file)
				printf("%s:", filename);
			count_occurrences++;
		}
		if (flags.invert)
			match = !match;

		if (!match || flags.count_occ){
			line_counting++;
			continue;
		}

		if(!flags.count_occ){
			if(flags.show_line)
				printf("%d: ", line_counting);

			fputs(buff, stdout);	
			line_counting++;
		}
	}
	if (flags.count_occ){
		printf("%d\n", count_occurrences);
	}
	free(buff);
}

// =========================================================
int main(int argc, char **argv) { // Grep Made by Me!

	if (argc < 2) {
		fprintf(stderr, "Usage: %s [flags]... <pattern> <filename>\n", argv[0]);
		return 1;
	}

	int arg_start = 0;
	flags_t flags = parse_flags(&arg_start, argc, argv);
	if(arg_start + 1 >= argc){
		exit(EXIT_FAILURE);
	}

	char *pattern = argv[arg_start];
	//char *filename = argv[arg_start + 1];
	//TODO: Suport multiple files

	int total_files = argc - (arg_start + 1);
	if(argc > arg_start + 1){ // more than one file
		for (int i = arg_start + 1; i < argc; i++){
			//open && close file each it.
			FILE *f = fopen(argv[i], "r"); 
			if(!f){
				return 1;
			}
			//TODO: replace strstr and strcasestr by regex
			grep_file(pattern, f, argv[i], flags, total_files > 1);
			fclose(f);
		}
	}

		
	return 0;
}

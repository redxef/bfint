/**
 * @author      
 * @file        main.c
 * @version     0.0.0-r0
 * @since       
 * 
 * @brief       A brief documentation about the file.
 *
 * A detailed documentation.
 */

#include <stdlib.h>
#include <stdio.h>
#include <stdint.h>
#include <assert.h>
#if defined __APPLE__ || defined __linux__ || defined __unix__
#define UNIX_LIKE
#include <unistd.h>
#define ISATTY isatty
#define FILENO fileno
#elif defined _WIN32
#include <io.h>
#define ISATTY _isatty
#define FILENO _fileno
#else
#error "Whatever strange OS you are running, it is crap."
#endif
#include <string.h>
#ifdef UNIX_LIKE
#define USING_OPTARG
#include <getopt.h>
#endif


void skip_loop(const char *prg, ssize_t *pc) {
        size_t brace_lvl = 0;
        do {
                switch(prg[*pc]) {
                        case '[':
                                brace_lvl++;
                                break;
                        case ']':
                                brace_lvl--;
                                break;        
                }
                (*pc)++;
        } while (brace_lvl);
}

void rewind_loop(const char *prg, ssize_t *pc) {
        size_t brace_lvl = 0;
        do {
                switch(prg[*pc]) {
                        case '[':
                                brace_lvl++;
                                break;
                        case ']':
                                brace_lvl--;
                                break;        
                }
                (*pc)--;
        } while (brace_lvl && *pc > 0);
}

int do_step(const char *prg, ssize_t *pc, ssize_t prg_size, char *tape, ssize_t *tc, ssize_t tape_size) {
        int c;
        switch (prg[*pc]) {
                case '+':
                        tape[*tc]++;
                        break;
                case '-':
                        tape[*tc]--;
                        break;
                case '<':
                        (*tc)--;
                        break;
                case '>':
                        (*tc)++;
                        break;
                case ',':
                        c = fgetc(stdin);
                        if (c != EOF)
                                tape[*tc] = c;
                        break;
                case '.':
                        fputc(tape[*tc], stdout);
                        fflush(stdout);
                        break;
                case '[':
                        if (!tape[*tc]) {
                                skip_loop(&prg[0], pc);
                                (*pc)--;
                        }
                        break;
                case ']':
                        rewind_loop(&prg[0], pc);
                        break;
                default:
                        break;
        }
        (*pc)++;

        if (*pc < 0) {
                fprintf(stderr, "Error: underflow occured in program counter.\n");
                return 1;
        }
        if (*pc >= prg_size) {
                fprintf(stderr, "Error: overflow occured in program counter.\n");
                return 1;
        }
        if (*tc < 0) {
                fprintf(stderr, "Error: underflow occured in tape counter.\n");
                return 1;
        }
        if (*tc >= tape_size) {
                fprintf(stderr, "Error: overflow occured in tape counter.\n");
                return 1;
        }
        return 0;
}

int read_prg_from_file(char **prg, ssize_t *prg_size, FILE *f) {
        int c;
        ssize_t pc = 0;
        char lst3[4] = { 0 };   /* used to detect eof */

        if (*prg == NULL) {
                *prg_size = 256;
                *prg = calloc(*prg_size, sizeof(char));
                assert(*prg != NULL);

                if (ISATTY(FILENO(f)))
                        printf("Enter program, end with 'EOF':\n");
                while ((c = fgetc(f)) != EOF) {
                        switch(c) {
                                case '+':
                                case '-':
                                case '<':
                                case '>':
                                case ',':
                                case '.':
                                case '[':
                                case ']':
                                        (*prg)[pc++] = c;
                                        break;
                        }
                        if (pc == *prg_size) {
                                (*prg_size) <<= 1;
                                *prg = realloc(*prg, *prg_size);
                                assert(*prg != NULL);
                        }
                        lst3[0] = lst3[1];
                        lst3[1] = lst3[2];
                        lst3[2] = c;
                        if (strcmp(lst3, "EOF") == 0)
                                break;
                }
                (*prg)[pc] = 0;

                /* read the newline which gets passed if a program is entered
                   interactively. */
                if (ISATTY(FILENO(f)))
                        fgetc(f);

                return 0;
        }
        return 1;
}

int parse_options(int argc, char **argv, char **prg, ssize_t *prg_size, char **tape, ssize_t *tape_size, char *flags) {
        int c;
        struct option lo[8];
        FILE *f;
        (void) tape;

        memset(lo, 0, sizeof(struct option) * 8);

        lo[0].name = "program";
        lo[0].has_arg = required_argument;
        lo[0].flag = NULL;
        lo[0].val = 'p';

        lo[1].name = "tape-size";
        lo[1].has_arg = required_argument;
        lo[1].flag = NULL;
        lo[1].val = 's';

        lo[2].name = "help";
        lo[2].has_arg = no_argument;
        lo[2].flag = NULL;
        lo[2].val = 'h';

        lo[3].name = "file";
        lo[3].has_arg = required_argument;
        lo[3].flag = NULL;
        lo[3].val = 'f';

        /*
         * p ... program, specifies a program.
         * s ... size of the tape, default is 30000
         * f ... file with bf sourcecode
         */
        while ((c = getopt_long(argc, argv, "p:s:h", lo, NULL)) != -1) {
                switch (c) {
                        case 'p':
                                *prg = optarg;
                                *prg_size = strlen(optarg);
                                break;
                        case 's':
                                *tape_size = atoi(optarg);
                                break;
                        case 'h':
                                printf("NAME\n");
                                printf("bfint\n");
                                printf("\n");
                                printf("SYNOPSIS\n");
                                printf("bfint [-p <program>] [-s <tape size>] [-h]\n");
                                printf("\n");
                                printf("DESCRIPTION\n");
                                printf("Used to interpret brainfuck programs.\n");
                                printf("\n");
                                printf("OPTIONS\n");
                                printf("-p <program>\n");
                                printf("--program <program>\n");
                                printf("Specify the brainfuck program as String.\n");
                                printf("\n");
                                printf("-s <tape size>\n");
                                printf("--tape-size <tape size>\n");
                                printf("Specify the size of the tape in bytes.\n");
                                printf("\n");
                                printf("-f <file>\n");
                                printf("--file <file>\n");
                                printf("Reads the program to be executed from the specified file.\n");
                                printf("\n");
                                printf("EXAMPLES\n");
                                printf("bfint --file copy.b\n");
                                printf("loads the source from copy.b and lets the user\n");
                                printf("interactively enter values.\n");
                                printf("\n");
                                printf("bfint --file - < ~/copy.b\n");
                                printf("If the string 'EOF' occures after the program source,\n");
                                printf("the user can specify input directly in the file\n");
                                printf("(after the EOF).\n");
                                printf("Example file content: '+[[-],.]EOFHELLO'\n");
                                return 1;
                                break;
                        case 'f':
                                if (strcmp("-", optarg) == 0)
                                        f = stdin;
                                else {
                                        f = fopen(optarg, "r");
                                }
                                if (f == NULL) {
                                        fprintf(stderr, "Error: failed to open file: %s\n", optarg);
                                }
                                *flags |= !read_prg_from_file(prg, prg_size, f) << 0;
                                break;
                        default:
                                break;
                }
        }
        return 0;
}

int check_program(char *prg, ssize_t prg_size) {
        ssize_t pc = 0;
        int brace = 0;
        while (prg[pc] && pc < prg_size) {
                if (prg[pc] == '[')
                        brace++;
                else if (prg[pc] == ']')
                        brace--;

                if (brace < 0)
                        return 0;
                pc++;
        }
        if (brace != 0)
                return 0;
        return 1;
}

int main(int argc, char **argv) {
        char *prg = NULL;       /* program */
        char *tape = NULL;      /* tape of memory */
        ssize_t prg_size = 0;
        ssize_t tape_size = 30000;
        ssize_t pc = 0;         /* program counter */
        ssize_t tc = 0;         /* tape counter */

        char flags = 0;

#ifdef USING_OPTARG
        if (parse_options(argc, argv, &prg, &prg_size, &tape, &tape_size, &flags) != 0)
                goto CLEANUP;
#endif

        flags |= !read_prg_from_file(&prg, &prg_size, stdin) << 0;
        assert(prg != NULL);

        if (!check_program(prg, prg_size)) {
                fprintf(stderr, "Error: invalid program.\n");
                goto CLEANUP;
        }

        /* finally init the tape */
        tape = calloc(tape_size, sizeof(char));
        assert(tape != NULL);

        while (prg[pc]) {
                if (do_step(prg, &pc, prg_size, tape, &tc, tape_size) != 0)
                        goto CLEANUP;
        }

        CLEANUP:
        free(tape);
        if (flags & 1 << 0)
                free(prg);

        return 0;
}

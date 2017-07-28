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
#ifndef _WIN32
#include <getopt.h>
#endif


void skip_loop(char *prg, size_t *pc) {
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

void rewind_loop(char *prg, size_t *pc) {
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
        } while (brace_lvl);
}

int main(int argc, char **argv) {
        char lst3[4] = { 0 };
        char *prg = NULL;       /* program */
        char *tape = NULL;      /* tape of memory */
        size_t prg_size = 0;
        size_t tape_size = 30000;
        size_t pc = 0;          /* program counter */
        size_t tc = 0;          /* tape counter */
        int c;                  /* getopt char, later signals if prg was entered
                                   interactively (c = 10) */

#ifndef _WIN32                  /* no one likes windows anyway */

        /*
         * p ... program, specifies a program.
         * s ... size of the tape, default is 30000
         * f ... file with bf sourcecode
         */
        while ((c = getopt(argc, argv, "p:s:h")) != -1) {
                switch (c) {
                        case 'p':
                                prg = optarg;
                                prg_size = strlen(optarg);
                                break;
                        case 's':
                                tape_size = atoi(optarg);
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
                                printf("Specify the brainfuck program as String.\n");
                                printf("\n");
                                printf("-s <tape size>\n");
                                printf("Specify the size of the tape in bytes.\n");
                                return 0;
                                break;
                        default:
                                break;
                }
        }
#endif

        if (prg == NULL) {
                prg_size = 256;
                prg = calloc(prg_size, sizeof(char));
                assert(prg != NULL);

                if (ISATTY(FILENO(stdin)))
                        printf("Enter program, end with 'EOF':\n");
                while ((c = fgetc(stdin)) != EOF) {
                        switch(c) {
                                case '+':
                                case '-':
                                case '<':
                                case '>':
                                case ',':
                                case '.':
                                case '[':
                                case ']':
                                        prg[pc++] = c;
                                        break;
                        }
                        if (pc == prg_size) {
                                prg_size <<= 1;
                                prg = realloc(prg, prg_size);
                                assert(prg != NULL);
                        }
                        lst3[0] = lst3[1];
                        lst3[1] = lst3[2];
                        lst3[2] = c;
                        if (strcmp(lst3, "EOF") == 0)
                                break;
                }
                prg[pc] = 0;
                pc = 0;
                /* read the newline which gets passed if a program is entered
                   interactively. */
                if (ISATTY(FILENO(stdin)))
                        fgetc(stdin);
                c = 10;
        }

        tape = calloc(tape_size, sizeof(char));
        assert(tape != NULL);

        while (pc >= 0 && prg[pc]) {
                switch (prg[pc]) {
                        case '+':
                                tape[tc]++;
                                break;
                        case '-':
                                tape[tc]--;
                                break;
                        case '<':
                                tc--;
                                break;
                        case '>':
                                tc++;
                                break;
                        case ',':
                                c = fgetc(stdin);
                                if (c != EOF)
                                        tape[tc] = c;
                                break;
                        case '.':
                                fputc(tape[tc], stdout);
                                fflush(stdout);
                                break;
                        case '[':
                                if (!tape[tc]) {
                                        skip_loop(&prg[0], &pc);
                                        pc--;
                                }
                                break;
                        case ']':
                                rewind_loop(&prg[0], &pc);
                                break;
                        default:
                                break;
                }
                pc++;
        }

        free(tape);
        if (c == 10)
                free(prg);

        return 0;
}

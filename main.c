#include <stdio.h>
#include <stdlib.h>

void
brainfuck(FILE *bf_file)
{
        int * instruction_pointer = nullptr;
        int * data_pointer = calloc(0, 30000);

        while ((*instruction_pointer = fgetc(bf_file)) != EOF) {
                switch (*instruction_pointer) {
                        case '>':
                                data_pointer++;
                                break;

                        case '<':
                                data_pointer--;
                                break;

                        case '+':
                                (*data_pointer)++;
                                break;

                        case '-':
                                (*data_pointer)--;
                                break;

                        case '.':
                                printf("%c", *data_pointer);
                                break;

                        case ',':
                                break;

                        case '[':
                                if (!data_pointer) {
                                        // Move forward
                                } else {

                                }
                                break;

                        case ']':
                                break;

                        default: break;
                }
        }

        free(data_pointer);
        data_pointer = nullptr;
        instruction_pointer = nullptr;
}

int
main(int argc, char * argv[])
{
        /* TODO: Implement correct arg parsing */
        FILE * bf_file = NULL;

        if (argc < 2) {
                fprintf(stderr, "ERROR: You must pass in a brainfuck file.\n");
                return 1;
        }

        bf_file = fopen(argv[1], "r");

        brainfuck(bf_file);

        return 0;
}

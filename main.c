#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lib/stack.h"

#define TAPE_LENGTH 30000

unsigned long
getTotalSystemMemory(void)
{
        const long pages = sysconf(_SC_PHYS_PAGES);
        const long page_size = sysconf(_SC_PAGE_SIZE);
        return pages * page_size;
}

void
brainfuck(FILE * bf_file, const size_t bf_file_size, int * returnVal)
{
        Stack stack;
        unsigned char * tape = calloc(
                TAPE_LENGTH, sizeof(unsigned char)
        );
        unsigned char * tape_pointer = tape;
        int instruction_pointer;
        fpos_t instruction_pointer_position;
        int depth = 0;

        init(&stack, bf_file_size);

        if (!tape_pointer) {
                perror("ERROR: Memory allocation failed!\n");
                fprintf(
                        stderr,
                        "Tried to allocate %d bytes out of %lu bytes.\n",
                        TAPE_LENGTH,
                        getTotalSystemMemory()
                );
                *returnVal = 1;
                goto cleanup;
        }

        while ((instruction_pointer = fgetc(bf_file)) != EOF) {
                switch (instruction_pointer) {
                case '>': /* Move tape pointer right */
                        tape_pointer++;
                        break;

                case '<': /* Move tape pointer left */
                        tape_pointer--;
                        break;

                case '+': /* Increment tape pointer value, wrap around via 0xFF */
                        *tape_pointer =  (*tape_pointer + 1) & 0xFF;
                        break;

                case '-': /* Decrement tape pointer value, wrap around via 0xFF */
                        *tape_pointer = (*tape_pointer - 1) & 0xFF;
                        break;

                case '.': /* Print tape pointer value to stdout*/
                        putchar(*tape_pointer);
                        break;

                case ',': /* Get tape pointer value from stdin */
                        *tape_pointer = getchar();
                        break;

                case '[': /* Start of loop */
                        if (!full(&stack)) {
                                fgetpos(bf_file, &instruction_pointer_position);
                                push(&stack, instruction_pointer_position);
                        } else {
                                fprintf(stderr, "ERROR: Tape overflow!\n");
                                *returnVal = 1;
                                goto cleanup;
                        }
                        if (!*tape_pointer) {
                                depth = 1;
                                while (depth > 0) {
                                        if (instruction_pointer == EOF) {
                                                perror("ERROR: Unmatched [");
                                                *returnVal = 1;
                                                goto cleanup;
                                        }
                                        instruction_pointer = fgetc(bf_file);
                                        depth += (instruction_pointer == '[');
                                        depth -= (instruction_pointer == ']');
                                }
                        }
                        break;

                case ']': /* End of loop */
                        if (get_top(&stack)) {
                                instruction_pointer_position = pop(&stack);
                                if (*tape_pointer) { /* Jump back, if value is non-zero */
                                        push(&stack, instruction_pointer_position);
                                        fsetpos(bf_file, &instruction_pointer_position);
                                }
                        } else {
                                printf("%d\n", get_top(&stack));
                                fprintf(stderr, "ERROR: Unmatched ]\n");
                                *returnVal = 1;
                                goto cleanup;
                        }
                        break;

                default:
                        break;
                }
        }

        *returnVal = 0;

cleanup:
        free(tape);
        tape = NULL;
}

int
main(const int argc, const char * argv[])
{
        FILE * bf_file = NULL;
        size_t bf_file_size;
        int returnVal = 0;

        /* TODO: Implement correct arg parsing */
        if (argc < 2) {
                perror("ERROR: You must pass in a brainfuck file.\n");
                return 1;
        }

        bf_file = fopen(argv[1], "r");

        if (bf_file == NULL) {
                perror("ERROR: Error opening file");
                return 1;
        }

        fseek(bf_file, 0L, SEEK_END);
        bf_file_size = ftell(bf_file);
        rewind(bf_file);

        brainfuck(bf_file, bf_file_size, &returnVal);

        fclose(bf_file);

        return returnVal;
}
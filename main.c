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
                /* Move tape pointer right */
                case '>':
                        tape_pointer++;
                        break;

                /* Move tape pointer left */
                case '<':
                        tape_pointer--;
                        break;

                /* Increment tape pointer value, wrap around upon overflow via 0xFF */
                case '+':
                        *tape_pointer =  (*tape_pointer + 1) & 0xFF;
                        break;

                /* Decrement tape pointer value, wrap around upon underflow via 0xFF */
                case '-':
                        *tape_pointer = (*tape_pointer - 1) & 0xFF;
                        break;

                /* Print tape pointer value to stdout*/
                case '.':
                        putchar(*tape_pointer);
                        break;

                /* Get tape pointer value from stdin */
                case ',':
                        *tape_pointer = getchar();
                        break;

                /* Start of loop */
                case '[':
                        if (full(&stack)) {
                                fprintf(stderr, "ERROR: Tape overflow!\n");
                                *returnVal = 1;
                                goto cleanup;
                        }
                        /* If the stack isn't full, get the position of the bracket
                         * and push it onto the stack
                         */
                        fgetpos(bf_file, &instruction_pointer_position);
                        push(&stack, instruction_pointer_position);

                        /* If the current cell is 0, move the instruction pointer
                         * forward to the matching bracket
                         */
                        if (*tape_pointer == 0) {
                                /* Keep track of the depth of the loops to ensure
                                 * only the matching closing bracket is used
                                 */
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

                /* End of loop */
                case ']':
                        if (get_top(&stack) == 0) {
                                fprintf(stderr, "ERROR: Unmatched ]\n");
                                *returnVal = 1;
                                goto cleanup;
                        }
                        instruction_pointer_position = pop(&stack);
                        if (*tape_pointer != 0) { /* Jump back if the value is non-zero */
                                push(&stack, instruction_pointer_position);
                                fsetpos(bf_file, &instruction_pointer_position);
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
        int return_val = 0;

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

        brainfuck(bf_file, bf_file_size, &return_val);

        fclose(bf_file);

        return return_val;
}
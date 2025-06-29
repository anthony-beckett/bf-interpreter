#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lib/stack.h"

#define TAPE_LENGTH 30000


void getNecessaryStackSize(FILE *, size_t *);
unsigned long getTotalSystemMemory(void);
void brainfsck(FILE *,  size_t, int *);


void
getNecessaryStackSize(FILE * bf_file, size_t * necessary_stack_size)
{
        char instruction_pointer;
        *necessary_stack_size = 0;

        while ((instruction_pointer = fgetc(bf_file)) != EOF)
                *necessary_stack_size += (instruction_pointer == '[');

        (*necessary_stack_size)++;
}

unsigned long
getTotalSystemMemory(void)
{
        const long pages = sysconf(_SC_PHYS_PAGES);
        const long page_size = sysconf(_SC_PAGE_SIZE);
        return pages * page_size;
}

void
brainfsck(FILE * bf_file, const size_t necessary_stack_size, int * returnVal)
{
        Stack stack;
        unsigned char * tape = calloc(
                TAPE_LENGTH, sizeof(unsigned char)
        );
        unsigned char * data_pointer = tape;
        int instruction_pointer;
        fpos_t instruction_pointer_position;
        int depth = 0;

        init(&stack, necessary_stack_size);

        if (!tape) {
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
                        data_pointer  = (data_pointer - tape + 1) % TAPE_LENGTH + tape;
                        break;

                /* Move tape pointer left */
                case '<':
                        data_pointer = (
                                data_pointer - tape - 1 + TAPE_LENGTH
                        ) % TAPE_LENGTH + tape;
                        break;

                /* Increment tape pointer value, wrap around upon overflow via 0xFF */
                case '+':
                        *data_pointer =  (*data_pointer + 1) & 0xFF;
                        break;

                /* Decrement tape pointer value, wrap around upon underflow via 0xFF */
                case '-':
                        *data_pointer = (*data_pointer - 1) & 0xFF;
                        break;

                /* Print tape pointer value to stdout*/
                case '.':
                        putchar(*data_pointer);
                        break;

                /* Get tape pointer value from stdin */
                case ',':
                        *data_pointer = getchar();
                        break;

                /* Start of loop */
                case '[':
                        if (full(&stack)) {
                                fprintf(stderr, "ERROR: Tape overflow!\n");
                                *returnVal = 1;
                                goto cleanup;
                        }

                        /* If the current cell is 0, move the instruction pointer
                         * forward to the matching bracket
                         */
                        if (*data_pointer == 0) {
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
                        } else {
                                /* If the stack isn't full, get the position of the bracket
                                 * and push it onto the stack
                                 */
                                fgetpos(bf_file, &instruction_pointer_position);
                                push(&stack, instruction_pointer_position);
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
                        if (*data_pointer != 0) { /* Jump back if the value is non-zero */
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
        size_t necessary_stack_size = 0;
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

        getNecessaryStackSize(bf_file, &necessary_stack_size);
        rewind(bf_file);

        brainfsck(bf_file, necessary_stack_size, &return_val);

        fclose(bf_file);

        return return_val;
}
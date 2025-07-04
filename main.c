#include <errno.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include "lib/stack.h"

#define TAPE_LENGTH 30000

void errorHandler(char *);
void getNecessaryStackSize(FILE *, size_t *);
unsigned long getTotalSystemMemory(void);
void brainfsck(FILE *,  size_t, int *);


void
errorHandler(char * msg)
{
        if (errno)
                perror(msg);
        else
                fprintf(stderr, "%s\n", msg);
}

void
getNecessaryStackSize(FILE * bf_file, size_t * necessary_stack_size)
{
        char instruction_pointer;
        *necessary_stack_size = 0;

        /* Iterate through the file, counting the number of 
         * opening brackets
         */
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
brainfsck(FILE * bf_file, const size_t necessary_stack_size, int * return_val)
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
                errorHandler("ERROR: Memory allocation failed!");
                fprintf(
                        stderr,
                        "Tried to allocate %d bytes out of %lu bytes.\n",
                        TAPE_LENGTH,
                        getTotalSystemMemory()
                );
                *return_val = 1;
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
                                errorHandler("ERROR: Tape overflow!");
                                *return_val = 1;
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
                                                errorHandler("ERROR: Unmatched [!");
                                                *return_val = 1;
                                                goto cleanup;
                                        }
                                        instruction_pointer = fgetc(bf_file);
                                        depth += (instruction_pointer == '[');
                                        depth -= (instruction_pointer == ']');
                                }
                        /* Otherwise, get the position of the bracket and push
                         * it onto the stack
                         */
                        } else {
                                fgetpos(bf_file, &instruction_pointer_position);
                                push(&stack, instruction_pointer_position);
                        }
                        break;

                /* End of loop */
                case ']':
                        if (get_top(&stack) == 0) {
                                errorHandler("ERROR: Unmatched ]!");
                                *return_val = 1;
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

        *return_val = 0;

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
                errorHandler("ERROR: You must pass in a brainfuck file.");
                return 1;
        }

        bf_file = fopen(argv[1], "r");

        if (bf_file == NULL) {
                errorHandler("ERROR: Error opening file");
                return 1;
        }

        getNecessaryStackSize(bf_file, &necessary_stack_size);
        rewind(bf_file);

        brainfsck(bf_file, necessary_stack_size, &return_val);

        fclose(bf_file);

        return return_val;
}

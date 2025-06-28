#ifndef BRAINFUCK_2024_STACK_H
#define BRAINFUCK_2024_STACK_H

#include <stdio.h>

#define MAX_STACK_SIZE 30000  /* Same as tape length should be enough */

/* Type Definition for Stack */
typedef struct {
        int top;
        size_t max_height;
        fpos_t data[MAX_STACK_SIZE];
} Stack;

void push(Stack* st, fpos_t value);
fpos_t pop(Stack* st);
void init(Stack* st, size_t max);
int full(Stack* st);
int get_top(Stack* st);
size_t get_height(Stack* st);
#endif /* BRAINFUCK_2024_STACK_H */
/*
 * stack.c
 *
 * An example (and probably not great) stack implimentation in ANSI C
 * Licensed under "Do what you want" by Leo Adamek (leo.adamek.me), 2012
 *
 * Notes:
 * This program is a demonstration of stacks in C.
 * The main function of this program is just a demonstration of the stack
 */

#include <stdio.h>

#include "stack.h"


/* Get current stack height and maximum height */
int
get_top(Stack* st){ return st->top; }

size_t
get_height(Stack* st){ return st->max_height; }


/* Add a value to the stack */
void
push(Stack* st, fpos_t value)
{
        st->data[st->top] = value;
        (st->top)++;
}

/* Retrieve a value from the stack */
fpos_t
pop(Stack* st)
{
        (st->top)--;
        return (st->data[st->top]);
}

/* Initialise a stack */
void
init(Stack* st, size_t max)
{
        st->top = 0;
        st->max_height = max;
}

/* check if a stack is full */
int
full(Stack* st)
{
        return ((size_t)(st->top) >= st->max_height);
}
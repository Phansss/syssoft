#ifndef STACK_H_
#define STACK_H_

#define MAX_STACK_SIZE	5

/*-- stack error codes --*/
enum err_code { ERR_NONE = 0, ERR_EMPTY, ERR_FULL, ERR_MEM, ERR_INIT, ERR_UNDEFINED };

typedef enum err_code err_code_t;

typedef long int element_t;

typedef void err_handler_func( const err_code_t );

typedef struct stack stack_t;


void stack_create( stack_t **, err_handler_func *err_handler ); /* cf. constructor: initialise the new stack */
void stack_free( stack_t ** ); /* cf. destructor: destroy the stack: free memory, etc. */
void stack_push( stack_t *, const element_t ); /* push element on the stack */
void stack_pop( stack_t * ); /* delete element from stack  */
void stack_top( const stack_t *, element_t * ); /* returns top of stack */
unsigned int stack_size( const stack_t * ); /* return number of elements on the stack */

#endif /*STACK_H_*/
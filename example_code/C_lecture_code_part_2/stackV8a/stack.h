#ifndef STACK_H_
#define STACK_H_

#define MAX_STACK_SIZE	10

enum err_code { ERR_NONE = 0, ERR_EMPTY, ERR_FULL, ERR_MEM, ERR_INIT, ERR_UNDEFINED };

typedef enum err_code err_code_t;

// don't use this anymore:
// typedef long int element_t;
typedef void * element_t;

// define CALLBACK functions
typedef void destroy_element_func(element_t);
typedef element_t copy_element_func(element_t);

typedef struct stack stack_t;

extern err_code_t stack_create( stack_t **, destroy_element_func *, copy_element_func * ); /* cf. constructor: initialise the new stack */
extern err_code_t stack_free( stack_t ** ); /* cf. destructor: destroy the stack: free memory, etc. */
extern err_code_t stack_push( stack_t *, const element_t ); /* push element on the stack */
extern err_code_t stack_pop( stack_t *); /* delete element from stack  */
extern err_code_t stack_top( const stack_t * , element_t * ); /* returns top of stack */
extern unsigned int stack_size( const stack_t * ); /* return number of elements on the stack */

#endif /*STACK_H_*/

/*
 * An implementation of a stack datastructure;
 * Elements on the stack have type long int;
 * - 1-file implementation
 * - Stack defined with typedef
 * - Manipulation of stack via operators
 * - Dynamic array implementation of stack
 * 
 */

#include <stdio.h>
#include <stdlib.h>

typedef long int element_t;

#define INIT_STACK_SIZE	3
#define INCREASE_STACK_SIZE	2

struct my_stack {
  element_t * data;
  int top;
  int maxSize;
};

typedef struct my_stack stack_t;

/*-- stack error codes --*/
enum err_code { ERR_NONE = 0, ERR_EMPTY, ERR_FULL, ERR_MEM, ERR_INIT, ERR_UNDEFINED };

typedef enum err_code err_code_t;

err_code_t stack_create( stack_t **); /* cf. constructor: alocates mem and initialise the new stack */
err_code_t stack_free( stack_t **); /* cf. destructor: destroy the stack: free memory, etc. */
err_code_t stack_push( stack_t * , const element_t ); /* push element on the stack */
err_code_t stack_pop( stack_t * ); /* delete element from stack  */
err_code_t stack_top( const stack_t * , element_t * ); /* returns top of stack */
unsigned int stack_size( const stack_t * ); /* return number of element_ts on the stack */

void err_handler( const err_code_t err );

int main( void )
{
  
  stack_t *s; // no memory is allocated yet!!!
  element_t value;
  err_code_t err;
  
  
  //initialize the stack before using it
  err_handler( stack_create(&s) );
  
  // read values from the command prompt and push them on the stack
  do
  {
    printf( "Give a value to push on the stack (negative value to quit): " );
    scanf( "%20ld", &value );
    if ( value >= 0 )
    {
      err = stack_push( s, value );
      err_handler( err );
    }
  } while ( value >= 0 );
  
  printf( "\nThe stack size is %d\n", stack_size(s) );
  
  printf( "\nThe stack values are:\n" );
  while ( stack_size(s) > 0)
  {
    err = stack_top(s, &value);
    err_handler( err );
    printf( "%ld\n", value );
    err_handler( stack_pop(s) );
  }
  
  //destroy the stack
  err_handler( stack_free(&s) );
  
  return 0;
}



err_code_t stack_create( stack_t **s )
{
  stack_t *t;
  t = (stack_t *) malloc ( sizeof(stack_t) );
  if (t == NULL )
  {
    *s = NULL;
    return ERR_MEM;
  }
  
  t->data = (element_t *) malloc ( sizeof(element_t) * INIT_STACK_SIZE );
  if (t->data == NULL )
  {
    free(t);
    *s = NULL;
    return ERR_MEM;
  }
  
  t->top = 0;	/* indicates empty stack */
  t->maxSize = INIT_STACK_SIZE;
  
  *s = t;
  return ERR_NONE;
}


err_code_t stack_free( stack_t **s )
{
    
  free( (*s)->data );
  free(*s);
  *s = NULL;
  return ERR_NONE;
}


err_code_t stack_push( stack_t * s, const element_t element )
{
  if (  s->top == s->maxSize ) { // reallocate memory
    s->data = (element_t *) realloc ( s->data,
				    sizeof(element_t) * (INCREASE_STACK_SIZE + s->maxSize) );
    if (s->data == NULL )
      return ERR_MEM;
    s->maxSize += INCREASE_STACK_SIZE;
  }
  
  s->data[s->top] = element;
  s->top++;
  return ERR_NONE;
}


err_code_t stack_pop( stack_t * s )
{
  if ( s->top == 0 )
    return ERR_EMPTY;
  
  s->top--;
  
  // TO DO: realloc stack is stack becomes to small
  
  return ERR_NONE;
}


err_code_t stack_top( const stack_t * s, element_t * element )
{
  if ( s->top == 0 )
    return ERR_EMPTY;
  *element = s->data[s->top-1];
  return ERR_NONE;
}


unsigned int stack_size( const stack_t * s )
{
  return ( s->top );
}


void err_handler( const err_code_t err )
{
  switch( err )
  {
    case ERR_NONE:
      break;
    case  ERR_EMPTY:
      printf( "\nCan't execute this operation while the stack is empty.\n" );
      break;
    case ERR_FULL:
      printf( "\nCan't execute this operation while the stack is full.\n" );
      break;
    case ERR_MEM:
      printf( "\nMemory problem appeared while executing this operation on the stack.\n" );
      break;
    case  ERR_INIT:
      printf( "\nStack initialisation problem.\n" );
      break;
    case ERR_UNDEFINED:
      printf( "\nUndefined problem appeared while executing this operation on the stack.\n" );
      break;
    default: // should never come here
      exit(0);
  }
}



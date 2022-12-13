
#include <stdlib.h>
#include "stack.h"

//#include "stack.h" // show effect of double reference for each .c file separately


struct stack {
  element_t data[MAX_STACK_SIZE];
  int top;
};


err_code_t stack_create( stack_t **s )
{
  #ifdef DEBUG
  printf( "\nstack_create() called\n" );
  #endif
  stack_t *t;
  t = (stack_t *) malloc ( sizeof(stack_t) );
  if (t == NULL )
  {
    *s = NULL;
    return ERR_MEM;
  }
  
  t->top = 0;	/* indicates empty stack */
  
  *s = t;
  return ERR_NONE;
}


err_code_t stack_free( stack_t **s )
{
  #ifdef DEBUG
  printf( "\nstack_free() called\n" );
  #endif
  free(*s);
  *s = NULL;
  return ERR_NONE;
}


err_code_t stack_push( stack_t * s, const element_t element )
{
  #ifdef DEBUG
  printf( "\nstack_push() called\n" );
  #endif
  if (  s->top == MAX_STACK_SIZE )
    return ERR_FULL;
  
  s->data[s->top++] = element;
  return ERR_NONE;
}


err_code_t stack_pop( stack_t * s )
{
  #ifdef DEBUG
  printf( "\nstack_pop() called\n" );
  #endif
  if ( s->top == 0 )
    return ERR_EMPTY;
  
  s->top--;
  return ERR_NONE;
}

err_code_t stack_top( const stack_t * s, element_t * element )
{
  #ifdef DEBUG
  printf( "\nstack_top() called\n" );
  #endif
  if ( s->top == 0 )
    return ERR_EMPTY;
  *element = s->data[s->top-1];
  return ERR_NONE;
}


unsigned int stack_size( const stack_t * s )
{
  #ifdef DEBUG
  printf( "\nstack_size() called\n" );
  #endif
  return ( s->top );
}







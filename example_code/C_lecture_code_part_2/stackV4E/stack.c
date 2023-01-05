
#include <stdlib.h>
#include "stack.h"


struct stack {
  element_t data[MAX_STACK_SIZE];
  int top;
  err_handler_func *err_handler; // callback function
};



void stack_create( stack_t **s, err_handler_func *err_handler )
{
  stack_t *t;
  
  t = (stack_t *) malloc ( sizeof(stack_t) );
  if (t == NULL )
  {
    *s = NULL;
    err_handler( ERR_MEM );
    return;
  }
  
  t->top = 0;	/* indicates empty stack */
  t->err_handler = err_handler;
  
  *s = t;
  return;
}


void stack_free( stack_t **s )
{
  free(*s);
  *s = NULL;
  return;
}


void stack_push( stack_t * s, const element_t element )
{
  if (  s->top == MAX_STACK_SIZE )
  {
    s->err_handler( ERR_FULL );
    return;
  }
  
  s->data[s->top++] = element;
  return;
}


void stack_pop( stack_t * s )
{
  if ( s->top == 0 )
  {
    s->err_handler( ERR_EMPTY );
    return;
  }
  
  s->top--;
  return;
}

void stack_top( const stack_t * s, element_t * element )
{
  if ( s->top == 0 )
  {
    s->err_handler( ERR_EMPTY );
    return;
  }
   
  *element = s->data[s->top-1];
  return;
}


unsigned int stack_size( const stack_t * s )
{
  return ( s->top );
}






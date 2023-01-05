
//#define DEBUG

#include <assert.h>
#include <stdlib.h>
#ifdef DEBUG
  #include <stdio.h>
#endif
#include "stack.h"

static void check_assert( const stack_t * );

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
  unsigned long numPushCalls = 0;
  numPushCalls++;
  printf("Entering function 'push' -- (#calls = %ld)\n", numPushCalls);
  #endif
  
  check_assert( s );  
  
  if (  s->top == MAX_STACK_SIZE )
    return ERR_FULL;
  
  s->data[s->top++] = element;
  return ERR_NONE;
}


err_code_t stack_pop( stack_t * s )
{
  #ifdef DEBUG
  unsigned long numPopCalls;
  numPopCalls++;
  printf("Entering function 'pop' -- (#calls = %ld)\n", numPopCalls);
  #endif
  
  check_assert( s ); 
  
  if ( s->top == 0 )
    return ERR_EMPTY;
  
  s->top--;
  return ERR_NONE;
}

err_code_t stack_top( const stack_t * s, element_t * element )
{
  #ifdef DEBUG
  unsigned long numTopCalls;
  numTopCalls++;
  printf("Entering function 'top' -- (#calls = %ld)\n", numTopCalls);
  #endif
  
  check_assert( s ); 
  
  if ( s->top == 0 )
    return ERR_EMPTY;
  *element = s->data[s->top-1];
  return ERR_NONE;
}


unsigned int stack_size( const stack_t * s )
{
  #ifdef DEBUG
  unsigned long numSizeCalls;
  numSizeCalls++;
  printf("Entering function 'stack size' -- (#calls = %ld)\n", numSizeCalls);
  #endif
  
  check_assert( s ); 
  
  return ( s->top );
}


static void check_assert( const stack_t *s )
{
  assert( (s->top >= 0) && (s->top <= MAX_STACK_SIZE) );
}






/*
 *  FILENAME: main.c
 *
 * Version 
 * An implementation of a stack datastructure;
 * Generic element type: type 'void *'
 *
 */
 

#include <assert.h>
#include <stdlib.h>
#include "stack.h"

#define MAX_STACK_SIZE	10

struct stack {
  element_t data[MAX_STACK_SIZE];
  int top;
  destroy_element_func *destroy_element; // callback function
  copy_element_func *copy_element; // callback function
};


static void check_assert( const stack_t * );

err_code_t stack_create( stack_t **s, destroy_element_func *destroy_element, copy_element_func *copy_element )
{
  stack_t *t;

  t = (stack_t *) malloc ( sizeof(stack_t) );
  if (t == NULL )
  {
    *s = NULL;
    return ERR_MEM;
  }
  
  t->top = 0;	/* indicates empty stack */
 
  t->destroy_element = destroy_element;
  t->copy_element = copy_element;
  
  *s = (stack_t *)t;
  return ERR_NONE;
}


err_code_t stack_free( stack_t **s )
{
  while ( (*s)->top != 0 ) {
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
    (*s)->top--; 
    (*s)->destroy_element( (*s)->data[(*s)->top] );// CLEANUP THE MEMORY ASSIGNED TO THE ELEMENT
    // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  }
  
  free(*s);
  *s = NULL;
  return ERR_NONE;
}

err_code_t stack_push( stack_t * s, const element_t element )
{
  check_assert( s );
  
  if (  s->top == MAX_STACK_SIZE )
    return ERR_FULL;
  
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  s->data[s->top++] = s->copy_element( element ); // CALL CALLBACK FUNC TO MAKE A DEEP COPY 
  
  //s->data[s->top++] = element; // NOT POSSIBLE IF = IS NOT DEFINED FOR THIS TYPE OF ELEMENT  
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  return ERR_NONE;
}


err_code_t stack_pop( stack_t * s )
{
  check_assert( s );
  
  if ( s->top == 0 )
    return ERR_EMPTY;
  
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  s->top--; 
  s->destroy_element( s->data[s->top] );// CALL CALLBACK FUN TO CLEANUP THE MEMORY ASSIGNED TO THE ELEMENT
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
  
  return ERR_NONE;
}


err_code_t stack_top( const stack_t * s, element_t * element )
{
  check_assert( s );
  
  if ( s->top == 0 )
    return ERR_EMPTY;
  
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!	///	
  *element = s->copy_element( s->data[s->top-1] ); // RETURN DEEP COPY
  //*element = s->data[s->top-1]; // RETURN JUST A POINTER
  // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!	
  
  return ERR_NONE;
}

unsigned int stack_size( const stack_t * s )
{
  check_assert( s );
  
  return ( s->top );
}

static void check_assert( const stack_t * s )
{
  assert( (s->top >= 0) && (s->top <= MAX_STACK_SIZE) ); 
}
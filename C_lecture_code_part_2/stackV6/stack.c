/*******************************************************************************
 *  FILENAME: stack.c
 *
 * Version
 * An implementation of a stack datastructure;
 * - Pointer list implementation of a stack
 *
 *******************************************************************************/

//#define DEBUG

#include <assert.h>
#include <stdlib.h>
#ifdef DEBUG
#include <stdio.h>
#endif
#include "stack.h"

#ifdef DEBUG
#define DEBUG_PRINT(...) 							\
do {					  					\
  printf("In %s - function %s at line %d: ", __FILE__, __func__, __LINE__);	\
  printf(__VA_ARGS__);								\
  } while(0)
#else
  #define DEBUG_PRINT(...) (void)0
#endif
  
  
err_code_t stack_create( stack_t ** stack )
{
  stack_t *t;
    
  DEBUG_PRINT( "\nstack_create() called\n" );
    
  t = (stack_t *) malloc ( sizeof(stack_t) );
  if (t == NULL )
  {
    *stack = NULL;
    return ERR_MEM;
  }
    
  t->top = NULL;	/* indicates empty stack */
  t->num_elements = 0;
    
  *stack = t;
  return ERR_NONE;
}
  
  
err_code_t stack_free( stack_t ** stack )
{
  DEBUG_PRINT( "\nstack_free() called\n" );
    
  node_t *temp;
  stack_t *t = *stack;
    
  // clean up memory
  while ( t->top != NULL )
  {
    temp = t->top->next;
    free( t->top );
    t->top = temp;
  }
    
  free(*stack);
  *stack = NULL;
  return ERR_NONE;
}
  
  
err_code_t stack_push( stack_t * stack , const element_t element )
{
  node_t *temp;
    
  DEBUG_PRINT( "\nstack_push() called\n" );
    
  temp = (node_t *) malloc( sizeof(node_t) );
  if ( temp == NULL )
    return ERR_MEM;
    
  temp->element = element;
  temp->next = stack->top;
  stack->top = temp;
    
  stack->num_elements++;
    
  return ERR_NONE;
}
  
  
err_code_t stack_pop( stack_t * stack )
{
  node_t *temp;
    
  DEBUG_PRINT( "\nstack_pop() called\n" );
    
  if ( stack->top == NULL )
    return ERR_EMPTY;
    
  temp = stack->top;
  stack->top = stack->top->next;
  free( temp );
  
  stack->num_elements--;
    
  return ERR_NONE;
}
  
  
err_code_t stack_top( const stack_t * stack, element_t * element )
{
    
  DEBUG_PRINT( "\nstack_top() called\n" );
   
  if ( stack->top == NULL )
    return ERR_EMPTY;
    
  *element = stack->top->element;
  return ERR_NONE;
}
  
  
unsigned int stack_size( const stack_t * stack )
{
  
  DEBUG_PRINT( "\nstack_size() called\n" );
    
  return ( stack->num_elements);
}
  
  
  
  
  
  
  
  
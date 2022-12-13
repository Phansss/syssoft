/*
*  FILENAME: main.c
*
* Version
* An implementation of a stack datastructure;
* Generic element type: type 'void *'
*
*/

#include <stdio.h>
#include <assert.h>
#include <stdlib.h>

#include "stack.h"

void destroy_element(element_t); 
element_t copy_element(element_t);

static void err_handler( const err_code_t err );

int main( void )
{
  
  stack_t * s; // 2nd stack: Stack stack2; etc.
  long int value;
  element_t element;
  err_code_t err;
  
  //initialize the stack before using it
  err_handler( stack_create(&s, &destroy_element, &copy_element) );
  
  // read values from the command prompt and push them on the stack
  do
  {
    printf( "Give a value to push on the stack (negative value to quit): " );
    scanf( "%20ld", &value );
    if ( value >= 0 )
    {
      err = stack_push( s, (element_t)&value );
      err_handler( err );
    }
  } while ( value >= 0 );
  
  printf( "\nThe stack size is %d\n", stack_size( s ) );
  
  printf( "\nThe stack values are:\n" );
  while ( stack_size( s ) > 0)
  {
    err = stack_top( s, &element);
    err_handler( err );
    printf( "%ld\n", *(long int *)element );
    // !!! include the following 'free' ONLY if top returns a deep copy to element
    free(element);   // avoid memory leak  ...
    err_handler( stack_pop( s ) );
  }
  
  //destroy the stack
  err_handler( stack_free( &s ) );
  
  return 0;
}

 

static void err_handler( const err_code_t err )
{
  switch( err ) {
    case ERR_NONE:
      break;
    case  ERR_EMPTY:
      printf( "\nCan't execute this operation while the stack is empty.\n" );
      break;
    case ERR_FULL:
      printf( "\nCan't execute this operation while the stack is full.\n" );
      break;
    case ERR_MEM:
      printf( "\nMemory problem occured while executing this operation on the stack.\n" );
      break;
    case ERR_INIT:
      printf( "\nStack initialization problem.\n" );
      break;
    case ERR_UNDEFINED:
      printf( "\nUndefined problem occured while executing this operation on the stack.\n" );
      break;
    default: // should never come here
      assert( 1==0 );
  }
}


void destroy_element(element_t element)  
{
  free( element );
}


element_t copy_element(element_t element) 
{
  long int *p;
  p = (long int *) malloc( sizeof(long int) );
  assert ( p != NULL );
  *p = *(long int *)element;
  return (element_t)p;
}




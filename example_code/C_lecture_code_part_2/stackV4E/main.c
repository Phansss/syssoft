/*
 * Version
 * An implementation of a stack datastructure;
 * callback error handler
 *
 */

#include <stdio.h>
#include <assert.h>
#include "stack.h"

void my_err_handler( const err_code_t err );

int main( void )
{
  
  stack_t *s; // 2nd stack: Stack stack2; etc.
  element_t value;
  
  //initialize the stack before using it
  stack_create( &s, &my_err_handler );
  
  // read values from the command prompt and push them on the stack
  do
  {
    printf( "Give a value to push on the stack (negative value to quit): " );
    scanf( "%20ld", &value );
    if ( value >= 0 )
    {
      stack_push( s, value );
    }
  } while ( value >= 0 );
  
  #ifdef DEBUG
  printf( "\nThe stack size is %d\n", stack_size( s ) );
  #endif
  
  printf( "\nThe stack values are:\n" );
  while ( stack_size( s ) > 0)
  {
    stack_top( s, &value);
    printf( "%ld\n", value );
    stack_pop( s );
  }
  
  //destroy the stack
  stack_free( &s );
  
  return 0;
}


void my_err_handler( const err_code_t err )
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




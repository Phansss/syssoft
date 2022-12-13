/*
 * Version
 * An implementation of a stack datastructure;
 * - Multiple file implementation (stack.c / stack.h)
 * - assert()
 *
 */

#include <stdio.h>
#include <assert.h>
//#include "stack.h" // show effect of double reference

#include "stack.h"

void err_handler( const err_code_t err );


int main( void )
{
  
  stack_t *s; // 2nd stack: Stack stack2; etc.
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
  
  #ifdef DEBUG
  printf( "\nThe stack size is %d\n", stack_size( s ) );
  #endif
  
  printf( "\nThe stack values are:\n" );
  while ( stack_size( s ) > 0)
  {
    err = stack_top( s, &value);
    err_handler( err );
    printf( "%ld\n", value );
    err_handler( stack_pop( s ) );
  }
  
  //destroy the stack
  err_handler( stack_free( &s ) );
  
  return 0;
}


void err_handler( const err_code_t err )
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







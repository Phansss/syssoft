
/*******************************************************************************
*  FILENAME: main.c
*
* Version V1
* An implementation of a stack datastructure;
* Elements on the stack have type long int;
* - 1-file implementation
* - Stack defined with typedef
* - Manipulation of stack via operators
* - Error handling
*
*******************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <assert.h>

#define MAX_STACK_SIZE	5 // define a constant
// ALT: const int MAX_STACK_SIZE = 5; 

typedef long int element_t;
// basic data types: int, char, float, double, void
// typequalifier: long, short, long long, signed, unsigned
	// ! size of a data type (in bytes) is system dependent - sizeof(int)
	// ! Is a char signed or unsigned? System dependent! 
// typedef: give a new name to a data type

struct stack
{
  element_t data[MAX_STACK_SIZE];
  int top;
};

typedef struct stack stack_t;

/*-- stack error codes --*/
enum err_code { ERR_NONE = 0, ERR_EMPTY, ERR_FULL, ERR_MEM, ERR_INIT, ERR_UNDEFINED };
// define a set of constant values that are related to each other

typedef enum err_code err_code_t;

/*
 * function prototypes
 */

err_code_t stack_create( void ); /* cf. constructor: initialize the new stack */
err_code_t stack_free( void ); /* cf. destructor: destroy the stack: free memory, etc. */
err_code_t stack_push( const element_t ); /* push element on the stack */
err_code_t stack_pop( void ); /* delete element from stack  */
err_code_t stack_top( element_t * ); /* returns top of stack */
unsigned int stack_size( void ); /* return number of elements on the stack */
err_code_t stack_print( void ); /* print out all elements on stack  */

void err_handler( const err_code_t err );

/*
 * global variables
 */

stack_t stack; // 2nd stack: Stack stack2; etc.
// struct stack stack


int main( void )
{
  element_t value;
  err_code_t err;
  
  //initialize the stack before using it
  err = stack_create();
  err_handler( err );
  // alt: err_handler( stack_create() );
  
  // read values from the command prompt and push them on the stack
  do
  {
    printf( "Give a value to push on the stack (negative value to quit): " );
    scanf( "%ld", &value );
    if ( value >= 0 )
    {
        err = stack_push( value );
        err_handler( err );
      //err_handler( stack_push( value ) );
    }
  } while ( value >= 0 );

  printf( "\nThe stack size is %d\n", stack_size() );

  //stack.top += MAX_STACK_SIZE+1;

  printf( "\nThe stack values are:\n" );
  err_handler( stack_print() );
  
  //destroy the stack
  err_handler( stack_free() );

  return 0;
}

err_code_t stack_create( void )
{
  stack.top = 0;	// indicates empty stack
  return ERR_NONE;
}

err_code_t stack_free( void )
{
  stack.top = 0; /* make stack empty */
  return ERR_NONE;
}

err_code_t stack_push( const element_t element )
{
  assert( (stack.top >= 0) && (stack.top <= MAX_STACK_SIZE) );
  if (  stack.top == MAX_STACK_SIZE )
    return ERR_FULL;

  stack.data[stack.top++] = element;
  return ERR_NONE;
}


err_code_t stack_pop( void )
{
  assert( (stack.top >= 0) && (stack.top <= MAX_STACK_SIZE) );
  if ( stack.top == 0 )
    return ERR_EMPTY;
  
  stack.top--;
  return ERR_NONE;
}


err_code_t stack_top( element_t * element )
{
  assert( (stack.top >= 0) && (stack.top <= MAX_STACK_SIZE) );
  if ( stack.top == 0 )
    return ERR_EMPTY;
  
  *element = stack.data[stack.top-1];
  return ERR_NONE;
}


unsigned int stack_size( void )
{
  /* try this one yourself */  
  
}

err_code_t stack_print( void )
{
    /* try this one yourself */ 
 
}


void err_handler( const err_code_t err )
{
  switch( err )
  {
    case ERR_NONE:
      break;
    case  ERR_EMPTY:
      printf( "\nCan't execute this operation while the stack is empty.\n" );
      exit(0);
      break;
    case ERR_FULL:
      printf( "\nCan't execute this operation while the stack is full.\n" );
      exit(0);
      break;
    case ERR_MEM:
      printf( "\nMemory problem appeared while executing this operation on the stack.\n" );
      exit(0);
      break;
    case  ERR_INIT:
      printf( "\nStack initialisation problem.\n" );
      exit(0);
      break;
    case ERR_UNDEFINED:
      printf( "\nUndefined problem appeared while executing this operation on the stack.\n" );
      exit(0);
      break;
    default: // should never come here
      assert( 1 == 0 );
  }
}

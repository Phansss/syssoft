/*
* Version
* An implementation of a stack datastructure;
* - 1-file implementation
* - Stack defined with typedef
* - Manipulation of stack via operators
* - Conditional compilation: DEBUG / STACK_SIZE / element TYPE
*
*/

#include <stdio.h>
#include <stdlib.h>

//#define STACK_SMALL
//#define DEBUG

#if defined(STACK_SMALL)
  #define MAX_STACK_SIZE	5
#elif defined (STACK_LARGE)
  #define MAX_STACK_SIZE	100
#elif defined (STACK_EXTRALARGE)
  #define MAX_STACK_SIZE	1000
#elif defined (STACK_SIZE)
  #define MAX_STACK_SIZE STACK_SIZE
#else
  #error "undefined stack size"
#endif

#define SHORT 1
#define INT 2
#define LONG 3
#define FLOAT 4
#define DOUBLE 5
#define CHAR 6

#if !defined(TYPE) // default data type
  typedef int element_t;
  #define FORMAT_SPEC "%d"
#elif TYPE == SHORT
  typedef short element_t;
  #define FORMAT_SPEC "%hd"
#elif TYPE == INT
  typedef int element_t;
  #define FORMAT_SPEC "%d"
#elif TYPE == LONG
  typedef long element_t;
  #define FORMAT_SPEC "%ld"
#elif TYPE == FLOAT
  typedef float element_t;
  #define FORMAT_SPEC "%f"
#elif TYPE == DOUBLE
  typedef double element_t;
  #define FORMAT_SPEC "%f"
#elif TYPE == CHAR
  typedef char element_t;
  #define FORMAT_SPEC "%1c"
#else
  #error "unacceptable type"
#endif


struct stack
{
  element_t data[MAX_STACK_SIZE];
  int top;
};

typedef struct stack stack_t;

/*-- stack error codes --*/
enum err_code { ERR_NONE = 0, ERR_EMPTY, ERR_FULL, ERR_MEM, ERR_INIT, ERR_UNDEFINED };

typedef enum err_code err_code_t;

err_code_t stack_create( stack_t ** ); /* cf. constructor: initialise the new stack */
err_code_t stack_free( stack_t ** ); /* cf. destructor: destroy the stack: free memory, etc. */
err_code_t stack_push( stack_t *, const element_t ); /* push element on the stack */
err_code_t stack_pop( stack_t * ); /* delete element from stack  */
err_code_t stack_top( const stack_t *, element_t * ); /* returns top of stack */
unsigned int stack_size( const stack_t *); /* return number of elements on the stack */

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
    printf( "\nGive a value to push on the stack (negative value to quit): " );
    scanf( FORMAT_SPEC, &value );
    if ( value >= 0 )
    {
      err = stack_push( s, value );
      err_handler( err );
    }
  } while ( value >= 0 );
  
  #ifdef DEBUG
  printf( "\nThe stack size is %d", stack_size( s ) );
  #endif
  
  printf( "\nThe stack values are:\n" );
  while ( stack_size( s ) > 0)
  {
    err = stack_top( s, &value);
    err_handler( err );
    printf( FORMAT_SPEC "\n", value );
    err_handler( stack_pop( s ) );
  }
  
  //destroy the stack
  err_handler( stack_free( &s ) );
  
  return 0;
}

err_code_t stack_create( stack_t **s )
{
  #ifdef DEBUG
  printf( "\nstack_create() called" );
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
  printf( "\nstack_free() called" );
  #endif
  free(*s);
  *s = NULL;
  return ERR_NONE;
}


err_code_t stack_push( stack_t * s, const element_t element )
{
  #ifdef DEBUG
  printf( "\nstack_push() called" );
  #endif
  if (  s->top == MAX_STACK_SIZE )
    return ERR_FULL;
  
  s->data[s->top++] = element;
  return ERR_NONE;
}


err_code_t stack_pop( stack_t * s )
{
  #ifdef DEBUG
  printf( "\nstack_pop() called" );
  #endif
  if ( s->top == 0 )
    return ERR_EMPTY;
  
  s->top--;
  return ERR_NONE;
}


err_code_t stack_top( const stack_t * s, element_t * element )
{
  #ifdef DEBUG
  printf( "\nstack_top() called" );
  #endif
  if ( s->top == 0 )
    return ERR_EMPTY;
  *element = s->data[s->top-1];
  return ERR_NONE;
}


unsigned int stack_size( const stack_t * s )
{
  #ifdef DEBUG
  printf( "\nstack_size() called" );
  #endif
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



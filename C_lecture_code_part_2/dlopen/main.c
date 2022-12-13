#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <assert.h>
#include "stack.h"


#define DLERRORTEST() 	do {				\
			    error_msg = dlerror(); 	\
			    if ( error_msg != NULL ) { 	\
			      printf("%s\n", error_msg); exit(1); }	\
			  } while(0)
  
static void err_handler( const err_code_t err );

err_code_t (*stack_create)( stack_t ** ); 
err_code_t (*stack_free)( stack_t ** ); 
err_code_t (*stack_push)( stack_t *, const element_t ); 
err_code_t (*stack_pop)( stack_t * ); 
err_code_t (*stack_top)( const stack_t *, element_t * ); 
unsigned int (*stack_size)( const stack_t *  ); 

int main(int argc, char **argv)
{
  void *handle;
  char *error_msg;
  stack_t *s;
  element_t value;
  err_code_t err;
  
  handle = dlopen("libstack.so", RTLD_LAZY);
  // I show error testing once, but better to use macro DLERRORTEST() for this 
  error_msg = dlerror();
  if ( error_msg != NULL )
  {
    printf("%s\n", error_msg);
    exit(1);
  }
  
  stack_create = dlsym( handle, "stack_create" );
  DLERRORTEST();
  
  stack_free = dlsym( handle, "stack_free" );
  DLERRORTEST();
  
  stack_pop = dlsym( handle, "stack_pop" );
  DLERRORTEST();
  
  stack_push = dlsym( handle, "stack_push" );
  DLERRORTEST();
  
  stack_top = dlsym( handle, "stack_top" );
  DLERRORTEST();
  
  stack_size = dlsym( handle, "stack_size" );	
  DLERRORTEST();
  
  //initialize the stack before using it
  stack_create( &s );
  
  // read values from the command prompt and push them on the stack
  do {
    printf( "Give a value to push on the stack (negative value to quit): " );
    scanf( "%ld", &value );
    if ( value >= 0 ) {
      err = stack_push( s, value );
      err_handler( err );
    }
  } while ( value >= 0 );
  
  printf( "\nThe stack size is %d\n", stack_size( s ) );
  
  printf( "\nThe stack values are:\n" );
  while ( stack_size( s ) > 0) {
    err = stack_top( s, &value);
    err_handler( err );
    printf( "%ld\n", value );
    err_handler( stack_pop( s ) );
  }
  
  stack_free( &s );
  
  if ( dlclose(handle) < 0 )
  {
    perror("Could not close library: ");
    exit(1);
  }
  
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

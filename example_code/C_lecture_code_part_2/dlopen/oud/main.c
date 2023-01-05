/*******************************************************************************
*  FILENAME: main.c
*
* Version
* An implementation of a stack datastructure;
* - Linking a shared library at runtime
*
*******************************************************************************/


/*------------------------------------------------------------------------------
		definitions (defines, typedefs, ...)
------------------------------------------------------------------------------*/

#include <stdio.h>
#include <stdlib.h>
#include <dlfcn.h>
#include <assert.h>
#include "stack.h"

#define DLERRORTEST() 	do {						\					
							error_msg = dlerror(); 	\
							if ( error_msg != NULL ) {  printf("%s\n", error_msg); exit(1); }	\															\
						} while(0)


/*------------------------------------------------------------------------------
		global variables
------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
		function prototypes
------------------------------------------------------------------------------*/

static void ErrHandler( const ErrCode err );

ErrCode (* Init)( Stack ** ); 
ErrCode (* Destroy)( Stack ** );
ErrCode (* Push)( Stack *, const Element ); 
ErrCode (* Pop)( Stack * ); 
ErrCode (* Top)( const Stack *, Element * ); 
unsigned int (* StackSize)( const Stack *  ); 

//----------------------------------------------------------------------------//
int main(int argc, char **argv)
//----------------------------------------------------------------------------//
{
	void *handle;
	char *error_msg;
	Stack *s;
	Element value;
	ErrCode err;

	
	handle = dlopen("./libstack.so", RTLD_LAZY);
	// I show error testing once, but better to use macro DLERRORTEST() for this 
	error_msg = dlerror();
	if ( error_msg != NULL )
	{
		printf("%s\n", error_msg);
		exit(1);
	}

	Init = dlsym( handle, "Init" );
	DLERRORTEST()
	
	Destroy = dlsym( handle, "Destroy" );
	DLERRORTEST()
	
	Pop = dlsym( handle, "Pop" );
	DLERRORTEST()
	
	Push = dlsym( handle, "Push" );
	DLERRORTEST()
	
	Top = dlsym( handle, "Top" );
	DLERRORTEST()
	
	StackSize = dlsym( handle, "StackSize" );	
	DLERRORTEST()
	
	//initialize the stack before using it
	Init( &s );

	// read values from the command prompt and push them on the stack
	do {
		printf( "Give a value to push on the stack (negative value to quit): " );
		scanf( "%ld", &value );
		if ( value >= 0 ) {
			err = Push( s, value );
			ErrHandler( err );
		}
	} while ( value >= 0 );

	printf( "\nThe stack size is %d\n", StackSize( s ) );

	printf( "\nThe stack values are:\n" );
	while ( StackSize( s ) > 0) {
		err = Top( s, &value);
		ErrHandler( err );
		printf( "%ld\n", value );
		ErrHandler( Pop( s ) );
	}

	//destroy the stack
	Destroy( &s );
	
	if ( dlclose(handle) < 0 )
	{
		perror("Could not close library: ");
		exit(1);
	}
	
	return 0;
}



//----------------------------------------------------------------------------//
static void ErrHandler( const ErrCode err )
//----------------------------------------------------------------------------//
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

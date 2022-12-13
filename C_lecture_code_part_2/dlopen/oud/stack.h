#ifndef STACK_H_
#define STACK_H_

/*------------------------------------------------------------------------------
		definitions (defines, typedefs, ...)
------------------------------------------------------------------------------*/

/*-- stack error codes --*/
enum errCode { ERR_NONE = 0, ERR_EMPTY, ERR_FULL, ERR_MEM, ERR_INIT, ERR_UNDEFINED };

typedef enum errCode ErrCode;

typedef long int Element;

typedef struct cell
			{
				Element element;
				struct cell *next;
			} Cell;

typedef struct stack {
	Cell *top;
	int numElements;
} Stack;

/*------------------------------------------------------------------------------
		global variables
------------------------------------------------------------------------------*/


/*------------------------------------------------------------------------------
		function prototypes
------------------------------------------------------------------------------*/

//extern ErrCode Init( Stack ** ); /* cf. constructor: initialise the new stack */
//extern ErrCode Destroy( Stack ** ); /* cf. destructor: destroy the stack: free memory, etc. */
//extern ErrCode Push( Stack *, const Element ); /* push element on the stack */
//extern ErrCode Pop( Stack * ); /* delete element from stack  */
//extern ErrCode Top( const Stack *, Element * ); /* returns top of stack */
//extern unsigned int StackSize( const Stack *  ); /* return number of elements on the stack */

#endif /*STACK_H_*/


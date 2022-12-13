#ifndef STACK_H_
#define STACK_H_


/*-- stack error codes --*/
enum err_code { ERR_NONE = 0, ERR_EMPTY, ERR_FULL, ERR_MEM, ERR_INIT, ERR_UNDEFINED };

typedef enum err_code err_code_t;

typedef long int element_t;

typedef struct node
{
  element_t element;
  struct node *next;
} node_t;

typedef struct stack {
  node_t *top;
  int num_elements;
} stack_t;

#endif /*STACK_H_*/


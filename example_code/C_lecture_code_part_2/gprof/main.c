/*
 *  FILENAME: main.c
 *
 * Version
 *
 */

#include <stdio.h>
#include <float.h>
#include <stdlib.h>
#include <time.h>
#include <math.h>

#define MAX_ELEMENTS 0x0FFFFFFF
#define MAX_SEARCH	250
#define DELAY 0x000F
//#define RAND_MIN DBL_MIN
//#define RAND_MAX DBL_MAX
#define RANGE_MIN 0
#define RANGE_MAX 1

typedef unsigned long index_t;

typedef enum {FALSE, TRUE} boolean_t;

void seed( void );
double double_rand(double , double);
void generate_numbers( void );
double get_number( void );
int double_compare( double, double );
boolean_t search_number( double, index_t * );

double data[MAX_ELEMENTS];

int main( void )
{
  double number;
  index_t index;
  index_t i;

  float ff;
  //
  
  seed();
  generate_numbers();
  for ( i = 0; i < MAX_SEARCH; i++)
  {
    number = get_number();
    if ( search_number( number, &index ) )
      printf("Number %f is found at index %ld (data[index] = %f)\n", number, index, data[index]);
    else
      printf("Number %f not found\n", number);
  }
  
  
  return 0;
}

int double_compare( double a, double b )
{
  if ( fabs(a - b) < FLT_EPSILON )
    return 0;
  else if (a < b)
    return 1;
  else return -1;
}

double get_number( void )
{
  return double_rand(RANGE_MIN, RANGE_MAX);
}


void generate_numbers( void )
{
  index_t i;
  
  for (i=0; i<MAX_ELEMENTS; i++)
  {
    data[i] = double_rand(RANGE_MIN, RANGE_MAX);
  }
}

// brute force search algo
boolean_t search_number( double number, index_t *index )
{	
  index_t i;
  for (i=0; i<MAX_ELEMENTS; i++)
  {
    if ( double_compare( number, data[i] ) == 0 )
    {
      *index = i;
      return TRUE;
    }
  }	
  *index = 0;
  return FALSE;
}


// return a rand number in [L,R].
double double_rand(double L, double R)
{
  long int i;
  double dummy;
  double temp = rand() / (double)RAND_MAX;
  // add some delay
  for (i = 0; i<DELAY; i++)
  {
    dummy = (long int)( ( (double)time(NULL) * (double)i ) / 13 ) % 1024; 
  }
  
  return (R-L)*temp + L;
}

// init random number generator with seed
void seed( void )
{
  srand(time(NULL));
}




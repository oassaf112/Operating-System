#include <assert.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/stat.h>
#include <unistd.h>

#define X_INSTRUCTIONS_NOT_NEEDED
#include "xis.h"
#include "xcpu.h"

#define TICK_ARG 1
#define IMAGE_ARG 2
#define QUANTUM_ARG 3

int main( int argc, char **argv ) {

  FILE *fp;
  struct stat fs;
  xcpu cs;
  unsigned char *mem;
  int ticks;

// Handle the command line input
  if( ( argc < QUANTUM_ARG ) || ( sscanf( argv[TICK_ARG], "%d", &ticks ) != 1 ) ||
      ( ticks < 0 )) {
    printf( "usage: xsim <ticks> <obj file> \n" );
    printf( "      <ticks> is number instructions to execute (0 = forever)\n" );
    printf( "      <image file> xis object file created by or xasxld\n" );
    return 1;
  }

//Step 1
 //Create a memory space for X simulation system
 //
//....



//Step 2
// Open the file specified on the command line

  //...

//Step 3
//Load the binary program into the memory
//If the file is not successfully, print out some error message and exit

//...

// Step 4
// close the file

// ...

//Step 5
// Initialize the cs context (registers)
  //...

//Step 6
// Make the cpu pointing to the memory space
  //...

//Step 7
// Let the CPU executing the code in mem, the cycle number is determined by the a command line input value
//...

  printf( "CPU ran out of time.\n" );
  return 0;
}



/* 
 * @copyright (c) 2008, Hedspi, Hanoi University of Technology
 * @author Huu-Duc Nguyen
 * @version 1.0
 */

#include <stdio.h>
#include <stdlib.h>

#include "reader.h"
#include "parser.h"

/******************************************************************/

int main() {
  if (compile("example3.kpl") == IO_ERROR) {
    printf("Can\'t read input file!\n");
    return -1;
  }
  return 0;
}

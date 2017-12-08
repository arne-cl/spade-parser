#include <stdio.h>
#include <string.h>
#include <stdlib.h>

typedef struct{
  int nedu;
  int *nw;
  int *bw, *ew;
  char ***edu;
} Dis;

Dis *readDis(FILE *f, int dim);

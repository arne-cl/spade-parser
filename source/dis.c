#include "dis.h"

Dis *readDis(FILE *f, int dim)
{
  int i, j, n, off;
  char buf[200];
  Dis *dis;

  if( (dis = (Dis *)malloc(sizeof(Dis)))==NULL )
    { fprintf(stderr, "Cannot allocate in readDis()\n"); exit(4); }

  dis->nedu = dim;

  if( (dis->edu = (char ***)calloc(dis->nedu+1, sizeof(char**)))==NULL )
    { fprintf(stderr, "Cannot allocate in readDis()\n"); exit(4); }
  if( (dis->nw = (int *)calloc(dis->nedu+1, sizeof(int)))==NULL )
    { fprintf(stderr, "Cannot allocate in readDis()\n"); exit(4); }
  if( (dis->bw = (int *)calloc(dis->nedu+1, sizeof(int)))==NULL )
    { fprintf(stderr, "Cannot allocate in readDis()\n"); exit(4); }
  if( (dis->ew = (int *)calloc(dis->nedu+1, sizeof(int)))==NULL )
    { fprintf(stderr, "Cannot allocate in readDis()\n"); exit(4); }

  for(i=1, off=0; i<=dim; i++)
    {
      fscanf(f, "%d", &n);
      dis->nw[i] = n;
      dis->bw[i] = off;
      dis->ew[i] = off + n;
      off += n;

      if( (dis->edu[i] = (char **)calloc(n, sizeof(char*)))==NULL )
	{ fprintf(stderr, "Cannot allocate in readDis()\n"); exit(4); }
      
      for(j=0; j<n; j++)
	{
	  fscanf(f, "%s", buf);
	  dis->edu[i][j] = strdup(buf);
	}
    }

  return dis;
}

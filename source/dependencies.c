#include "tree.h"
#include "dis.h"
#include "dtree.h"
#include "myalloc.h"

int debug = 0;

int training = 0;       /* regime: train or test; in training, eduboundaries==0 */
int eduboundaries = 1;  /* output: edu label or edu boundaries */
int classrel = 1;       /* relations: all 110 or only class relations(18) */

extern int cntalloc;
int allb;

int extractDependencies(Tree *tree, Dis *dis);
int getEdu4Lex(Dis *dis, char *lex, int b, int e, int *edub, int *edue);
Tree* getTree4Edu(Tree *tree, int db, int de);
void trimTree(Tree *tree, int db, int de);
Tree* getUpperParentNode(Tree *tree, char *lex);
void extractDTreeStructure(DTree *dtree);

int main(int argc, char **argv)
{
  FILE *f;
  Tree *tree[100000];
  Dis  *dis[100000];
  DTree *dtree[100000];
  int c, i, j, k, n, nt, ndis, ndt, offset, dim;
  int b, e, flag;
  char buf[200], rst[100];

  if(training && argc!=4)
    { fprintf(stderr, "Usage: %s file.MRG file.dis.edu file.dis.prc\n", argv[0]); exit(1); }
  if(!training && argc!=3)
    { fprintf(stderr, "Usage: %s file.MRG file.dis.edu\n", argv[0]); exit(1); }

  if( (f=fopen(argv[1], "r"))==NULL)
    { fprintf(stderr, "Cannot open file %s\n", argv[1]); exit(2); }
      
  nt = 0;
  c = fgetc(f);
  while( c=='\n' || c=='\t' || c==' ')
    c = fgetc(f);
  while( c=='(' )
    {
      tree[nt++] = readTree(f);
      
      c = fgetc(f);
      while( c=='\n' || c=='\t' || c==' ')
	c = fgetc(f);
    }
  fclose(f);
  /* printf("Done with %s (%d out of %d)\n", argv[1], nf++, argc-2); */

  if( (f=fopen(argv[2], "r"))==NULL)
    { fprintf(stderr, "Cannot open file %s\n", argv[2]); exit(2); }
  n = strlen(argv[2]);
  if( (argv[2][n-3]!='f' || argv[2][n-2]!='m' || argv[2][n-1]!='t') && 0 )
    { fprintf(stderr, "File %s needs extension .fmt\n", argv[2]); exit(2); }
  ndis = 0;
  while( fscanf(f, "%d", &dim)!=EOF )
    {
      if( dim>=0 )
	dis[ndis++] = readDis(f, dim);
      else
	dis[ndis++] = NULL;
    }
  fclose(f);	

  if( training ){
    if( (f=fopen(argv[3], "r"))==NULL)
      { fprintf(stderr, "Cannot open file %s\n", argv[3]); exit(2); }
    n = strlen(argv[3]);
    if( argv[3][n-3]!='p' || argv[3][n-2]!='r' || argv[3][n-1]!='c' )
      { fprintf(stderr, "File %s needs extension .prc\n", argv[3]); exit(2); }
    ndt = 0;
    while( fscanf(f, "%d", &offset)!=EOF )
      {
	if( offset>=0 )
	  {
	    fscanf(f, "%s", buf); /* Parent: or NULL */
	    if( !strcmp(buf, "Parent:") ){
	      fscanf(f, "%d", &b); fscanf(f, "%d", &e); fscanf(f, "%s", buf); /* b & e not used here */
	      if( strcmp(buf, "Root") )
		fscanf(f, "%s", buf);
	    }
	    fscanf(f, "%s", buf);
	    if( strcmp(buf, "(") )
	      { fprintf(stderr, "Keyword ( missing\n"); exit(4); }
	    dtree[ndt++] = readDTree(f, offset);
	  }
	else
	  {
	    dtree[ndt++] = NULL;
	    while( fscanf(f, "%s", buf)!=EOF )
	      if( !strcmp(buf, "---") )
		break;
	  }	 
      }
    fclose(f);	
  }

  if( nt==ndis && (ndis==ndt || !training) )
    ; /*fprintf(stderr, "%s: %d sentences\n", argv[1], nt); */
  else
    { fprintf(stderr, "%s: Unequal number of input elements: %d trees, %d dis, %d dtrees\n", argv[1], nt, ndis, ndt); exit(33); }

  for(i=0; i<nt; i++)
    {
      printf("%d %s:%d/%d\n", dis[i]->nedu, argv[1], i, nt);
      if( !training ){
	for(j=1; j<=dis[i]->nedu;j++){
	  for(k=0; k<dis[i]->nw[j]; k++)
	    printf("%s ", dis[i]->edu[j][k]);
	  printf("| ");
	}
	printf("\n");
      }
      flag = 0;
      if( dis[i]->nedu>1 )
	flag = extractDependencies(tree[i], dis[i]);
      if( !flag )
	printf("Depend:NONE");
      if( training ){
	  printf("\n");
	  if( dtree[i] && dtree[i]->node->span[1]> 1 )
	    extractDTreeStructure(dtree[i]);
	  else
	    if( !dtree[i]  )
	      printf("Struct:N/A");
	    else
	      printf("Struct:NONE");
      }
      printf("\n---\n");
    }

}
     
int extractDependencies(Tree *tree, Dis *dis)
{
  int findpt, findt, dir, db, de, lb, le, hb, he, pedu, pedub, pedue, bptlex, eptlex;
  int i, l1, l2, r1, r2;
  Tree *t, *ct, *pt, *tt;
  char *l, *h;
  char spanl[20], spanh[20], syn[100], rst[100], tlex[100], ptlex[100];

  for(i=1; i<=dis->nedu; i++){
      ct = deepCopy(tree, 0);
      db = dis->bw[i]; 
      de = dis->ew[i]; 

      t = getTree4Edu(ct, db, de); 
      updateLexHead(t); 
      pt = getUpperParentNode(tree, t->lex);
      if(!pt)
	return 0;
      
      for(tt=pt->child, findt=0, findpt=0; tt!=NULL; tt=tt->siblingR)
	{
	  if( !strcmp(tt->lex, t->lex) )
	    { 
	      l = tt->pos;
	      lb = tt->b;
	      le = tt->e; 
	      sprintf(spanl, "%d-%d", lb, le);
	      findt = 1; 
	    }
	  if( !strcmp(tt->lex, pt->lex) && findpt==0 )
	    {
	      h = tt->pos; 
	      if( findt==0 )
		dir = 'L';
	      else dir = 'R';
	      hb = tt->b; 
	      he = tt->e; 
	      sprintf(spanh, "%d-%d", hb, he);
	      findpt = 1;
	    }
	}
      if( !strcmp(pt->pos, "S1") )
	{ 
	  dir = 'T';
	  pt->pos = strdup("TOP"); 
	  h = pt->pos;
	  pt->lex = pt->pos;
	}

      getLex(tlex, t->lex); getLex(ptlex, pt->lex);
      bptlex = getBLex(pt->lex); eptlex = getELex(pt->lex);
      pedu = getEdu4Lex(dis, ptlex, bptlex, eptlex, &pedub, &pedue);

      h = pt->pos; /* take only parent info */
      if( strcmp(pt->pos, "TOP") )
	if( eduboundaries )
	  printf("%d{%d:%d}[%s/%s]<[%s]%d{%d:%d}[%s/%s] ", i, dis->bw[i], dis->ew[i], tlex, l, pt->pos, pedu, pedub, pedue, ptlex, h);
	else
	  printf("%d[%s/%s]<[%s]%d[%s/%s] ", i, tlex, l, pt->pos, pedu, ptlex, h);
  }

  return 1;
}

int getEdu4Lex(Dis *dis, char *lex, int b, int e, int *edub, int *edue)	      
{
  int i, j;

  for(i=1; i<=dis->nedu; i++)
    if( dis->bw[i]<=b && e<=dis->ew[i] )
      for(j=0; j<dis->nw[i]; j++)
	if( !strcmp(lex, dis->edu[i][j]) ){
	  *edub = dis->bw[i];
	  *edue = dis->ew[i];
	  return i;
	}

  *edub = *edue = -1;
  return -1;
}


Tree* getTree4Edu(Tree *tree, int db, int de)
{
  if( tree==NULL )
    return 0;

  trimTree(tree, db, de);
  while( 1 )
    {
      if( tree->child )
	if( tree->child->siblingR )
	  break;
        else
	  tree = tree->child;
      else
	break;
    }

  return tree;
}

void trimTree(Tree *tree, int db, int de)
{
  Tree *t, *tt, *ttt;

  if( tree->b >= db && tree->e <= de )
    return;
  if( tree->b >= de || tree->e <= db) /* some anomaly! */
    return;

  for(t=tree->child; t!=NULL;) 
    {
      if( db >= t->e && t->e>0 )/* trim to the left */
	{
	  tt = t;
	  tree->child = t->siblingR;
	  tree->b = t->siblingR->b;
	  t = t->siblingR;
	  t->siblingL = 0;
	  freeTree(tt);
	  if( t==NULL )
	    break;
	}
      else
	if( de <= t->b )/* trim to the right */
	  {
	    t->siblingL->siblingR = 0;
	    tree->e = t->siblingL->e;
	    for(tt=t; tt!=NULL; )
	      {
		ttt = tt;
		tt = tt->siblingR;
		freeTree(ttt);
	      }
	    break;
	  }
	else
	  t = t->siblingR;
    }
  
  for(t=tree->child; t!=NULL; t=t->siblingR)
    trimTree(t, db, de);
  
  if( tree->child )
    {
      tree->b = tree->child->b;
      for(t=tree->child; t->siblingR!=NULL; t=t->siblingR)
	;
      tree->e = t->e;
    }
}

Tree* getUpperParentNode(Tree *tree, char *lex)
{
  Tree *t, *rt;

  if( tree->child==NULL )
    return 0;

  for(t=tree->child; t!=NULL; t=t->siblingR)
    if( !strcmp(t->lex, lex) )
      return tree;

  for(t=tree->child; t!=NULL; t=t->siblingR)
    {
      rt = getUpperParentNode(t, lex);
      if( rt )
	return rt;
    }

  return 0;
}


void extractDTreeStructure(DTree *dtree)
{
  DTree *dt;
  char rel2par0[200], rel2par1[200];

  if( dtree==NULL )
    return;

  for(dt=dtree->child; dt!=NULL; dt=dt->sibling)
    extractDTreeStructure(dt);

  
  if( dtree->sibling ){
    if( classrel ){
      relClass(rel2par0, dtree->node->rel2par);
      relClass(rel2par1, dtree->sibling->node->rel2par);
    }
    else{
      strcpy(rel2par0, dtree->node->rel2par);
      strcpy(rel2par1, dtree->sibling->node->rel2par);
    }
    if( eduboundaries )
      printf("(%d{%d}:%s=%s:%d{%d},%d{%d}:%s=%s:%d{%d}) ", dtree->node->span[0], dtree->node->bw[0], dtree->node->status, rel2par0, dtree->node->span[1], dtree->node->ew[dtree->node->npieces-1], dtree->sibling->node->span[0], dtree->sibling->node->bw[0], dtree->sibling->node->status, rel2par1, dtree->sibling->node->span[1], dtree->sibling->node->ew[dtree->sibling->node->npieces-1]);
    else
      printf("(%d:%s=%s:%d,%d:%s=%s:%d) ", dtree->node->span[0], dtree->node->status, rel2par0, dtree->node->span[1], dtree->sibling->node->span[0], dtree->sibling->node->status, rel2par1, dtree->sibling->node->span[1]);
  }
}


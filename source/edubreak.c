#include "tree.h"

#define MAXRULE 50000

int LARALEX = 1;
int LARANOLEX = 1;
int L1R1LEX = 1;
int L1R1RHLEX = 2;
int L1R1LHLEX = 2;
int L1R1NOLEX = 1;
int L2L1LEX = 0;
int L2L1NOLEX = 0; /* context too loose */
int R1R2LEX = 0;
int R1R2RHLEX = 1;
int R1R2NOLEX = 0; /* context too loose */

char **whatrLARALEX[MAXRULE];
int whererLARALEX[MAXRULE], hmrLARALEX[MAXRULE], arrLARALEX[MAXRULE], nrLARALEX;

char **whatrLARANOLEX[MAXRULE];
int whererLARANOLEX[MAXRULE], hmrLARANOLEX[MAXRULE], arrLARANOLEX[MAXRULE], nrLARANOLEX;

char **whatrL1R1LEX[MAXRULE];
int whererL1R1LEX[MAXRULE], hmrL1R1LEX[MAXRULE], arrL1R1LEX[MAXRULE], nrL1R1LEX;

char **whatrL1R1RHLEX[MAXRULE];
int whererL1R1RHLEX[MAXRULE], hmrL1R1RHLEX[MAXRULE], arrL1R1RHLEX[MAXRULE], nrL1R1RHLEX;

char **whatrL1R1LHLEX[MAXRULE];
int whererL1R1LHLEX[MAXRULE], hmrL1R1LHLEX[MAXRULE], arrL1R1LHLEX[MAXRULE], nrL1R1LHLEX;

char **whatrL1R1NOLEX[MAXRULE];
int whererL1R1NOLEX[MAXRULE], hmrL1R1NOLEX[MAXRULE], arrL1R1NOLEX[MAXRULE], nrL1R1NOLEX;

char **whatrL2L1LEX[MAXRULE];
int whererL2L1LEX[MAXRULE], hmrL2L1LEX[MAXRULE], arrL2L1LEX[MAXRULE], nrL2L1LEX;

char **whatrL2L1NOLEX[MAXRULE];
int whererL2L1NOLEX[MAXRULE], hmrL2L1NOLEX[MAXRULE], arrL2L1NOLEX[MAXRULE], nrL2L1NOLEX;

char **whatrR1R2LEX[MAXRULE];
int whererR1R2LEX[MAXRULE], hmrR1R2LEX[MAXRULE], arrR1R2LEX[MAXRULE], nrR1R2LEX;

char **whatrR1R2NOLEX[MAXRULE];
int whererR1R2NOLEX[MAXRULE], hmrR1R2NOLEX[MAXRULE], arrR1R2NOLEX[MAXRULE], nrR1R2NOLEX;

char **whatrR1R2RHLEX[MAXRULE];
int whererR1R2RHLEX[MAXRULE], hmrR1R2RHLEX[MAXRULE], arrR1R2RHLEX[MAXRULE], nrR1R2RHLEX;
/*
char **whatr[MAXRULE];
int wherer[MAXRULE], hmr[MAXRULE], arr[MAXRULE], nr;
*/

int allb;

void doEduBreak(Tree *tree);
int readRules(FILE *f, char **whatrule[], int whererule[], int hmrule[], int arrule[], int threshold);
int searchEduBreakRules(char *ruleA[], char *ruleB[][3], int ar, int breakidx[]);
char* getLabLex(Tree *tree);
void writeEduBreakTree(Tree *tree);

int sameLabel(char *s1, char *s2);
int compactRules(char *rule[], char *ruleNL[], int ar);

int main(int argc, char **argv)
{
  FILE *f, *fLARALEX, *fLARANOLEX;
  FILE *fL1R1LEX, *fL1R1RHLEX, *fL1R1LHLEX, *fL1R1NOLEX;
  FILE *fL2L1LEX, *fL2L1NOLEX;
  FILE *fR1R2LEX, *fR1R2RHLEX, *fR1R2NOLEX;
  Tree *tree;
  int i, nt, c;

  if( (fLARALEX=fopen("../resource/rules.LARAlex.doit", "r"))==NULL && LARALEX ){
    fprintf(stderr, "Cannot open file rules.LARAlex.doit\n"); exit(33);
  }   
  if( (fLARANOLEX=fopen("../resource/rules.LARAnolex.doit", "r"))==NULL && LARANOLEX ){
    fprintf(stderr, "Cannot open file rules.LARAnolex.doit\n"); exit(33);
  }
  if( (fL1R1LEX=fopen("../resource/rules.L1R1lex.doit", "r"))==NULL && L1R1LEX ){
      fprintf(stderr, "Cannot open file rules.L1R1lex.doit\n"); exit(33);
  }
  if( (fL1R1RHLEX=fopen("../resource/rules.L1R1rhlex.doit", "r"))==NULL && L1R1RHLEX ){
      fprintf(stderr, "Cannot open file rules.L1R1rhlex.doit\n"); exit(33);
  }
  if( (fL1R1LHLEX=fopen("../resource/rules.L1R1lhlex.doit", "r"))==NULL && L1R1LHLEX ){
      fprintf(stderr, "Cannot open file rules.L1R1lhlex.doit\n"); exit(33);
  }
  if( (fL1R1NOLEX=fopen("../resource/rules.L1R1nolex.doit", "r"))==NULL && L1R1NOLEX ){
    fprintf(stderr, "Cannot open file rules.L1R1nolex.doit\n"); exit(33);
  }
  if( (fL2L1LEX=fopen("../resource/rules.L2L1lex.doit", "r"))==NULL && L2L1LEX ){
    fprintf(stderr, "Cannot open file rules.L2L1lex.doit\n"); exit(33);
  }
  if( (fL2L1NOLEX=fopen("../resource/rules.L2L1nolex.doit", "r"))==NULL && L2L1NOLEX ){
    fprintf(stderr, "Cannot open file rules.L2L1nolex.doit\n"); exit(33);
  }
  if( (fR1R2LEX=fopen("../resource/rules.R1R2lex.doit", "r"))==NULL && R1R2LEX ){
    fprintf(stderr, "Cannot open file rules.R1R2lex.doit\n"); exit(33);
  }
  if( (fR1R2RHLEX=fopen("../resource/rules.R1R2rhlex.doit", "r"))==NULL && R1R2RHLEX ){
    fprintf(stderr, "Cannot open file rules.R1R2rhlex.doit\n"); exit(33);
  }
  if( (fR1R2NOLEX=fopen("../resource/rules.R1R2nolex.doit", "r"))==NULL && R1R2NOLEX ){
    fprintf(stderr, "Cannot open file rules.R1R2nolex.doit\n"); exit(33);
  }

  if( LARALEX )
    nrLARALEX = readRules(fLARALEX, whatrLARALEX, whererLARALEX, hmrLARALEX, arrLARALEX, LARALEX);
  if( LARANOLEX ) 
    nrLARANOLEX = readRules(fLARANOLEX, whatrLARANOLEX, whererLARANOLEX, hmrLARANOLEX, arrLARANOLEX, LARANOLEX); 
  if( L1R1LEX )
    nrL1R1LEX = readRules(fL1R1LEX, whatrL1R1LEX, whererL1R1LEX, hmrL1R1LEX, arrL1R1LEX, L1R1LEX); 
  if( L1R1RHLEX )
    nrL1R1RHLEX = readRules(fL1R1RHLEX, whatrL1R1RHLEX, whererL1R1RHLEX, hmrL1R1RHLEX, arrL1R1RHLEX, L1R1RHLEX);
  if( L1R1LHLEX )
    nrL1R1LHLEX = readRules(fL1R1LHLEX, whatrL1R1LHLEX, whererL1R1LHLEX, hmrL1R1LHLEX, arrL1R1LHLEX, L1R1LHLEX);
  if( L1R1NOLEX )
    nrL1R1NOLEX = readRules(fL1R1NOLEX, whatrL1R1NOLEX, whererL1R1NOLEX, hmrL1R1NOLEX, arrL1R1NOLEX, L1R1NOLEX);
  if( L2L1LEX )
    nrL2L1LEX = readRules(fL2L1LEX, whatrL2L1LEX, whererL2L1LEX, hmrL2L1LEX, arrL2L1LEX, L2L1LEX);
  if( L2L1NOLEX )
    nrL2L1NOLEX = readRules(fL2L1NOLEX, whatrL2L1NOLEX, whererL2L1NOLEX, hmrL2L1NOLEX, arrL2L1NOLEX, L2L1NOLEX);
  if( R1R2LEX )
    nrR1R2LEX = readRules(fR1R2LEX, whatrR1R2LEX, whererR1R2LEX, hmrR1R2LEX, arrR1R2LEX, R1R2LEX);
  if( R1R2RHLEX )
    nrR1R2RHLEX = readRules(fR1R2RHLEX, whatrR1R2RHLEX, whererR1R2RHLEX, hmrR1R2RHLEX, arrR1R2RHLEX, R1R2RHLEX);
  if( R1R2NOLEX )
    nrR1R2NOLEX = readRules(fR1R2NOLEX, whatrR1R2NOLEX, whererR1R2NOLEX, hmrR1R2NOLEX, arrR1R2NOLEX, R1R2NOLEX);
  /*
  if( )
    nr = readRules(f, whatr, wherer, hmr, arr, );
    */

  for(i=1; i<argc; i++)
    {
      if( (f=fopen(argv[i], "r"))==NULL)
	{ fprintf(stderr, "Cannot open file %s\n", argv[i]); exit(2); }
      
      nt = 0;
      c = fgetc(f);
      while( c=='\n' || c=='\t' || c==' ')
	c = fgetc(f);
      while( c=='(' )
	{
	  tree = readTree(f);
	  doEduBreak(tree);
	  writeEduBreakTree(tree);
	  printf("<S>\n");
 
	  c = fgetc(f);
	  while( c=='\n' || c=='\t' || c==' ')
	    c = fgetc(f);
	}
      fclose(f);	
    }
}

void doEduBreak(Tree *tree)
{
  Tree *t;
  char *ruleA[100], *ruleB[100][3];
  int i, ar, breakidx[100], n;

  if( tree==NULL )
    return;

  if( tree->child==NULL )
    return;

  for(t=tree->child; t!=NULL; t=t->siblingR)
    doEduBreak(t);

  for(t=tree->child, ar=0; t!=NULL; t=t->siblingR)
    if( t->b!=t->e )
      ar += 1;
  if( ar<2 )
    return;

  ruleA[0] = getLabLex(tree);
  for(t=tree->child, ar=0; t!=NULL; t=t->siblingR){
    if( t->b!=t->e ){
      ruleA[ar+1] = getLabLex(t);
      ruleB[ar][0] = getLabLex(tree);
      ruleB[ar][1] = getLabLex(t);

      if( t->siblingR != NULL ){
	if( t->siblingR->b==t->siblingR->e && t->siblingR->siblingR )
	  t = t->siblingR;
	ruleB[ar][2] = getLabLex(t->siblingR);
      }
      else{
	ruleB[ar][2] = strdup("N/A"); 
      }
      ar += 1;
    }
  }

  n = searchEduBreakRules(ruleA, ruleB, ar, breakidx);
  if( n>0 )
    if( (tree->edubreak = (int *)calloc(sizeof(int), n))==NULL ){
      fprintf(stderr, "Cannot allocate in doEduBreak()\n"); exit(34); 
    }
    else
      for(i=0; i<n; i++)
	tree->edubreak[i] = breakidx[i];
}


int searchEduBreakRules(char *ruleA[], char *ruleB[][3], int ar, int breakidx[])
{
  int i, j, k, n;

  n = 0;
  for(i=0; i<20; i++)
    breakidx[i] = 0;

  for(i=0; i<nrLARALEX && LARALEX; i++){
    if( ar==arrLARALEX[i] ){
      for(k=0; k<=ar; k++)
	if( strcmp(ruleA[k], whatrLARALEX[i][k]) )
	  break;
      if( k>ar ){
	breakidx[n++] = whererLARALEX[i];
      }
    }
  }

  for(i=0; i<nrLARANOLEX && LARANOLEX; i++){
    if( ar==arrLARANOLEX[i] ){
      for(k=0; k<=ar; k++)
	if( !sameLabel(ruleA[k], whatrLARANOLEX[i][k]) )
	  break;
      if( k>ar ){
	breakidx[n++] = whererLARANOLEX[i];
      }
    }
  }

  for(i=0; i<nrL1R1LEX && L1R1LEX; i++){
    for(k=0; k<ar-1; k++){
      for(j=0; j<3; j++)
	if( strcmp(ruleB[k][j], whatrL1R1LEX[i][j]) )
	  break;
      if( j==3 )
	breakidx[n++] = (k+1);
    }
  }

  for(i=0; i<nrL1R1RHLEX && L1R1RHLEX; i++){
    for(k=0; k<ar-1; k++){
      for(j=0; j<3; j++){
	if( j==2 ){
	  if( strcmp(ruleB[k][j], whatrL1R1RHLEX[i][j]) )
	    break;
	}
	else
	  if( !sameLabel(ruleB[k][j], whatrL1R1RHLEX[i][j]) )
	    break;
      }
      if( j==3 )
	breakidx[n++] = (k+1);
    }
  }

  for(i=0; i<nrL1R1LHLEX && L1R1LHLEX; i++){
    for(k=0; k<ar-1; k++){
      for(j=0; j<3; j++){
	if( j==1 ){
	  if( strcmp(ruleB[k][j], whatrL1R1LHLEX[i][j]) )
	    break;
	}
	else
	  if( !sameLabel(ruleB[k][j], whatrL1R1LHLEX[i][j]) )
	    break;
      }
      if( j==3 )
	breakidx[n++] = (k+1);
    }
  }

  for(i=0; i<nrL1R1NOLEX && L1R1NOLEX; i++){
    for(k=0; k<ar-1; k++){
      for(j=0; j<3; j++)
	if( !sameLabel(ruleB[k][j], whatrL1R1NOLEX[i][j]) )
	  break;
      if( j==3 )
	breakidx[n++] = (k+1);
    }
  }

  for(i=0; i<nrL2L1LEX && L2L1LEX; i++){
    for(k=0; k<ar-1; k++){
      for(j=0; j<3; j++)
	if( strcmp(ruleB[k][j], whatrL2L1LEX[i][j]) )
	  break;
      if( j==3 )
	breakidx[n++] = (k+2);
    }
  }

  for(i=0; i<nrL2L1NOLEX && L2L1NOLEX; i++){
    for(k=0; k<ar-1; k++){
      for(j=0; j<3; j++)
	if( !sameLabel(ruleB[k][j], whatrL2L1NOLEX[i][j]) )
	  break;
      if( j==3 )
	breakidx[n++] = (k+2);
    }
  }

  for(i=0; i<nrR1R2LEX && R1R2LEX; i++){
    for(k=0; k<ar-1; k++){
      for(j=0; j<3; j++)
	if( strcmp(ruleB[k][j], whatrR1R2LEX[i][j]) )
	  break;
      if( j==3 )
	breakidx[n++] = (k+0);
    }
  }

  for(i=0; i<nrR1R2RHLEX && R1R2RHLEX; i++){
    for(k=0; k<ar-1; k++){
      for(j=0; j<3; j++)
	if( j==1 ){
	  if( strcmp(ruleB[k][j], whatrR1R2RHLEX[i][j]) )
	    break;
	}
	else{
	  if( !sameLabel(ruleB[k][j], whatrR1R2RHLEX[i][j]) )
	    break;
	}
      if( j==3 )
	breakidx[n++] = (k+0);
    }
  }

  for(i=0; i<nrR1R2NOLEX && R1R2NOLEX; i++){
    for(k=0; k<ar-1; k++){
      for(j=0; j<3; j++)
	if( !sameLabel(ruleB[k][j], whatrR1R2NOLEX[i][j]) )
	  break;
      if( j==3 )
	breakidx[n++] = (k+0);
    }
  }

  /*
  for(i=0; i<nr && ; i++){
    for(k=0; k<ar-1; k++){
      for(j=0; j<3; j++)
	if( !sameLabel(ruleB[k][j], whatr[i][j]) )
	  break;
      if( j==3 )
	breakidx[n++] = (k+?);
    }
  }
  */

  /* compacting possible duplicates */
  breakidx[n] = 0;
  for(i=0; i<n; i++)
    for(j=i+1; j<n; j++)
      if( breakidx[j]==breakidx[i] && breakidx[j]>0 )
	breakidx[j] = -breakidx[j];
  for(i=0; i<n;)
    if( breakidx[i]<0 ){
      for(j=i; j<n-1; j++)
	breakidx[j] = breakidx[j+1];
      n -= 1;
    }
    else
      i += 1;
    
  breakidx[n] = 0;
  return n;
}

int readRules(FILE *f, char **whatrule[], int whererule[], int hmrule[], int arrule[], int threshold)
{
  int n = 0, ar, hm, i, b;
  char buf[200], tmp[200];

  while( fscanf(f, "%d", &hm)!=EOF ){
    if( hm < threshold )
      break;
    hmrule[n] = hm;

    fscanf(f, "%s", buf);
    fscanf(f, "%s", tmp); /* -> */
    fscanf(f, "%d", &ar);
    arrule[n] = ar;
    
    if( (whatrule[n] = (char **)calloc(sizeof(char*), ar+1))==NULL ){
      fprintf(stderr, "Out of memory in readRules()\n"); exit(16); 
    }
    whatrule[n][0] = strdup(buf);
    for(i=1, b=0; i<=ar; ){
      if( fscanf(f, "%s", buf)!=EOF )
	if( !strcmp(buf, "|") ){
	  whererule[n] = i-1;
	  b = 1;
	}
	else
	  whatrule[n][i++] = strdup(buf);
      else
	break;
    }
    if( b==0 )
      whererule[n] = ar;
    n += 1;
  }

  return n;
}

char* getLabLex(Tree *tree)
{
  char buf[200], lex[100], *l;

  buf[0] = 0;
  if( tree ){
    strcpy(buf, tree->pos);
    strcat(buf, "[");
    getLex(lex, tree->lex);
    strcat(buf, lex);
    strcat(buf, "]");
  }
  else{ strcpy(buf, "N/A"); }
  
  l = strdup(buf);
  return l;
}

void writeEduBreakTree(Tree *tree)
{
  Tree *t;
  int i, k, w;

  if( tree==NULL )
    return ;

  if( tree->edubreak && tree->edubreak[0]>0 ){
    for(t=tree->child, k=1; t!=NULL; t=t->siblingR, k++){
      writeEduBreakTree(t);
      for(i=0; tree->edubreak[i]>0; i++)
	if( tree->edubreak[i]>0 )
	  if( k==tree->edubreak[i] && strcmp(t->pos, ".") ){ /* artificially ban break after . */
	    printf("\n");	
	    break;
	  }
    }
  }
  else
    for(t=tree->child, k=0; t!=NULL; t=t->siblingR, k++){
      writeEduBreakTree(t);
    }
  
  if( tree->child==NULL ){
    writeLeaves(tree, 1);
  }

}


int sameLabel(char *s1, char *s2)
{
  char ls1[200], ls2[200];
  int i, r=0;

  strcpy(ls1, s1);
  strcpy(ls2, s2);

  for(i=0; i<strlen(ls1); i++)
    if( ls1[i]=='[' ){ ls1[i] = 0; break; }

  for(i=0; i<strlen(ls2); i++)
    if( ls2[i]=='[' ){ ls2[i] = 0; break; }

  if( !strcmp(ls1, ls2) )
    r = 1;

  return r;
}



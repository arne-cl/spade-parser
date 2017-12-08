#include "tree.h"
#include "treelexhead.h"

extern int allb;

Tree* readTree(FILE *f)
{
  Tree *tree;
  int off = 0;
  char last[200];

  last[0] = 0;
  tree = readTree1(f, 0, 0, &off, last);
  updateSpans(tree);
  updateLexHead(tree);
  updateTreeText(tree);

  return tree;
}

Tree* readTree1(FILE *f, Tree *parent, Tree *siblingL, int *off, char last[])
{
  Tree *t;
  char buf[2000], lexid[100];
  int i, c;

  c = fgetc(f);  
  if( c==' ' ) /* S1 missing in Gold trees */
    strcpy(buf, "S1");
  else
    {
      i = 0;
      buf[i++] = c;
      while( c != ' ' )
	{
	  c = fgetc(f);  
	  buf[i++] = c;
	}
      buf[i-1] = 0;
      c = fgetc(f);
    }
  if( (t = (Tree *)malloc(sizeof(Tree)))==NULL )
    { fprintf(stderr, "Cannot allocate in readTree()\n"); exit(4); }
  else
    allb += sizeof(Tree);

  t->parent = parent;
  for(i=1; i<strlen(buf)-1; i++)
    if( (buf[i] == '-') || (buf[i] == '=') )
      buf[i] = 0;
  if( buf[0]=='(' )
    for(i=1; i<=strlen(buf); i++)
      buf[i-1] = buf[i];
  t->pos = strdup(buf);
  t->lex = NULL;
 
  while( c=='\n' || c=='\t' || c==' ')
    c = fgetc(f);
  if( c=='(' )
    {
      t->child = readTree1(f, t, 0, off, last);
      if( !strcmp(t->pos, "S1") || !strcmp(t->pos, "TOP") )
	{ t->siblingL = t->siblingR = 0; return t; }
      c = fgetc(f);
      while( c=='\n' || c=='\t' || c==' ')
	c = fgetc(f);
      if( c=='(' )
	t->siblingR = readTree1(f, t->parent, t, off, last);
      else
	t->siblingR = NULL;
    }
  else
    {
      i = 0; 
      buf[i++] = c;
      while( (c=fgetc(f))!=')' )
	buf[i++] = c;
      buf[i] = 0;
      t->child = 0;
      t->b = *off;
      if( increaseLeavesCount(t->pos, buf, last) )
	*off += 1;
      t->e = *off;
      if( !strcmp(buf, "-LRB-") )
	strcpy(buf, "(");
      if( !strcmp(buf, "-RRB-") )
	strcpy(buf, ")");
      if( !strcmp(buf, "-LCB-") )
	strcpy(buf, "{");
      if( !strcmp(buf, "-RCB-") )
	strcpy(buf, "}");
      sprintf(lexid, "[%d-%d]", t->b, t->e);
      strcat(buf, lexid);
      t->lex = strdup(buf);
      strcpy(last, t->lex); /* last is needed for double-quote mismatch */

      c = fgetc(f);
      while( c=='\n' || c=='\t' || c==' ')
	c = fgetc(f);
      if( c==')' )
	t->siblingR = 0;
      else
	{
	  while( c=='\n' || c=='\t' || c==' ')
	    c = fgetc(f);
	  if( c=='(' )
	      t->siblingR = readTree1(f, t->parent, t, off, last);
	}
    }
  
  t->siblingL = siblingL;
  return t;
}

int increaseLeavesCount(char *pos, char *lex, char *last)
{
  if( !strcmp(pos, "-NONE-") )
    return 0;

 if( !strcmp(lex, "'") && !strcmp(last, "'") )
   return 0;

 if( !strcmp(lex, "`") && !strcmp(last, "`") )
   return 0;
     
 return 1;
}

void updateSpans(Tree *tree)
{
  Tree *t;

  if( tree->child==NULL )
    return;

  for(t=tree->child; t!=NULL; t=t->siblingR)
    updateSpans(t);
      
  tree->b = tree->child->b;
  for(t=tree->child; t->siblingR!=NULL; t=t->siblingR)
    ;
  tree->e = t->e;
}


void updateTreeText(Tree *tree)
{
  char *leaf[500];
  int i, n;

  n = getLeaves(tree, leaf, 0);

  tree->ntext = n;
  if( (tree->text = (char **)calloc(tree->ntext, sizeof(char*)))==NULL ){
    fprintf(stderr, "Cannot allocate in updateText()\n");
    exit(22);
  }

  for(i=0; i<n; i++)
    tree->text[i] = leaf[i];

  updateRecTreeText(tree, tree->text);
}

int getLeaves(Tree *tree, char *leaf[], int i)
{
  Tree *t;

  if( tree==NULL )
    return i;

  if( tree->child==NULL && strcmp(tree->pos, "-NONE-") ){
    leaf[i] = tree->lex;
    return i+1;
  }
  else
    for(t=tree->child; t!=NULL; t=t->siblingR)
      i = getLeaves(t, leaf, i);

  return i;
}

void updateRecTreeText(Tree *tree, char **text)
{
  Tree *t; 

  if( tree==NULL )
    return;

  tree->text = text;

  for(t=tree->child; t!=NULL; t=t->siblingR)
    updateRecTreeText(t, text);
}

void updateLexHead(Tree *tree)
{
  Tree *t;

  if( tree==NULL )
    return;
  if( tree->child==NULL )
    return;


  for(t=tree->child; t!=NULL; t=t->siblingR)
    updateLexHead(t);
      
  tree->lex = lexHead(tree);
}

char* lexHead(Tree *tree)
{
  int found, i;
  Tree *tt;
  char *lex;

  if( !strcmp(tree->pos, "ADJP") )
    return lexSearchLeft(tree, ADJP, nADJP, &found);
      
  if( !strcmp(tree->pos, "ADVP") )
    return lexSearchRight(tree, ADVP, nADVP, &found);

  if( !strcmp(tree->pos, "CONJP") )
    return lexSearchRight(tree, CONJP, nCONJP, &found);

  if( !strcmp(tree->pos, "FRAG") )
    return lexSearchRight(tree, FRAG, nFRAG, &found);

  if( !strcmp(tree->pos, "INTJ") )
    return lexSearchLeft(tree, INTJ, nINTJ, &found);

  if( !strcmp(tree->pos, "LST") )
    return lexSearchRight(tree, LST, nLST, &found);

  if( !strcmp(tree->pos, "NAC") )
    return lexSearchLeft(tree, NAC, nNAC, &found);

  if( !strcmp(tree->pos, "PP") )
    return lexSearchRight(tree, PP, nPP, &found);

  if( !strcmp(tree->pos, "PRN") )
    return lexSearchLeft(tree, PRN, nPRN, &found);

  if( !strcmp(tree->pos, "PRT") )
    return lexSearchRight(tree, PRT, nPRT, &found);

  if( !strcmp(tree->pos, "QP") )
    return lexSearchLeft(tree, QP, nQP, &found);

  if( !strcmp(tree->pos, "RRC") )
    return lexSearchRight(tree, RRC, nRRC, &found);

  if( !strcmp(tree->pos, "S") )
    return lexSearchLeft(tree, S, nS, &found);

  if( !strcmp(tree->pos, "SBAR") )
    return lexSearchLeft(tree, SBAR, nSBAR, &found);

  if( !strcmp(tree->pos, "SBARQ") )
    return lexSearchLeft(tree, SBARQ, nSBARQ, &found);

  if( !strcmp(tree->pos, "SINV") )
    return lexSearchLeft(tree, SINV, nSINV, &found);

  if( !strcmp(tree->pos, "SQ") )
    return lexSearchLeft(tree, SQ, nSQ, &found);

  if( !strcmp(tree->pos, "UCP") )
    return lexSearchRight(tree, UCP, nUCP, &found);

  if( !strcmp(tree->pos, "VP") )
    return lexSearchLeft(tree, VP, nVP, &found);

  if( !strcmp(tree->pos, "WHADJP") )
    return lexSearchLeft(tree, WHADJP, nWHADJP, &found);

  if( !strcmp(tree->pos, "WHADVP") )
    return lexSearchRight(tree, WHADVP, nWHADVP, &found);

  if( !strcmp(tree->pos, "WHNP") )
    return lexSearchLeft(tree, WHNP, nWHNP, &found);

  if( !strcmp(tree->pos, "WHPP") )
    return lexSearchRight(tree, WHPP, nWHPP, &found);

  if( !strcmp(tree->pos, "NP") || !strcmp(tree->pos, "NX") || !strcmp(tree->pos, "X") )
    {
      for(tt=tree->child; tt->siblingR!=NULL; tt=tt->siblingR)
	;
      if( !strcmp(tt->pos, "POS") )
	return tt->lex;
      lex = lexSearchRight(tree, NP1, nNP1, &found); if( found ) return lex;
      lex = lexSearchLeft(tree, NP2, nNP2, &found);  if( found ) return lex;
      lex = lexSearchRight(tree, NP3, nNP3, &found); if( found ) return lex;
      lex = lexSearchRight(tree, NP4, nNP4, &found); if( found ) return lex;
      lex = lexSearchRight(tree, NP5, nNP5, &found); if( found ) return lex;
      return tt->lex;
      /* what about coordinated phrases? */
    }
}

char *lexSearchLeft(Tree *tree, char* list[], int nlist, int *found)
{
  Tree *t;
  int i;

  *found = 1;
  for(i=0; i<nlist; i++)
    for(t=tree->child; t!=NULL; t=t->siblingR)
      if( !strcmp(t->pos, list[i]) && t->b < t->e )
	return t->lex;
  
  *found = 0;
  return tree->child->lex; /*default lex */
}

char* lexSearchRight(Tree *tree, char* list[], int nlist, int *found)
{
  Tree *t, *tt;
  int i;

  *found = 1;
  for(tt=tree->child; tt->siblingR!=NULL; tt=tt->siblingR)
    ;

  for(i=0; i<nlist; i++)
    for(t=tt; t!=NULL; t=t->siblingL)
      if( !strcmp(t->pos, list[i]) && t->b < t->e )
	return t->lex;

  *found = 0;
  return tt->lex; /*default lex */
}

int nodeNumber(Tree *tree)
{
  Tree *t;
  int n=0;

  for(t=tree->child; t!=NULL; t=t->siblingR)
    n += nodeNumber(t);
  
  return n+1;
}

int leafNumber(Tree *tree)
{
  Tree *t;
  int n=0;

  for(t=tree->child; t!=NULL; t=t->siblingR)
    n += leafNumber(t);
  
  if( tree->child==NULL )
    return 1;

  return n;
}

int height(Tree *tree)
{
  Tree *t;
  int h, maxh;

  maxh = 0;
  for(t=tree->child; t!=NULL; t=t->siblingR)
    {
      h = height(t);
      if( maxh < h )
	maxh = h;
    }
  
  return maxh+1;
}

int countArity(Tree *tree, int c)
{
  Tree *t;
  int r=0, k;

  for(t=tree->child, k=0; t!=NULL; t=t->siblingR, k++)
    r += countArity(t, c);

  if( c<9 && k==c )
    r += 1;
  if( c>=9 && k>=c )
    r += 1;

  return r;
}

int hasPOSLeaf(Tree *tree, char *pos)
{
  Tree *t;
  
  if( tree==NULL )
    return 0;

  if( tree->child == NULL && !strcmp(tree->pos, pos) )
    return 1;

  for(t=tree->child; t!=NULL; t=t->siblingR)
    if( hasPOSLeaf(t, pos) )
      return 1;
 
  return 0;
} 

void writeTree(Tree *tree, int tab)
{
  Tree *t;
  int i, ar;

  if( tree==NULL )
    return;

  printf("(%s ", tree->pos);
  for(t=tree->child, ar=0; t!=NULL; t=t->siblingR){
    writeTree(t, tab+4); /* tab not used */
    ar += 1;
  }
    
  if( !tree->child ){
    if( tree->lex )
      printf("%s", tree->lex);
    else
      printf("%s", "no-lex");    
    if( tree->siblingR && tree->siblingR->child ){
      printf(")\n");
      for(i=0; i<tab; i++) printf(" ");    
    }
    else
      printf(") ");
  }
  else{
    printf(")\n");
    for(i=0; i<tab; i++) printf(" ");     
  }
}


void writeLEXPOSLeaves(FILE *f, Tree *tree)
{
  Tree *t;

  if( tree==NULL )
    return;

  if( tree->child == NULL )
    fprintf(f, "%s %s\n", tree->lex, tree->pos);

  for(t=tree->child; t!=NULL; t=t->siblingR)
    writeLEXPOSLeaves(f, t);
}

int writeLeaves(Tree *tree, int flag)
{
  Tree *t;
  char lex[100];
  int n = 0;

  if( tree==NULL )
    return;

  if( tree->child == NULL )
    if( strcmp(tree->pos, "-NONE-") ) /* bogus lex in Gold trees */
      {
	if( flag ){
	  getLex(lex, tree->lex);
	  printf("%s ", lex);
	}
	n = 1;
      }

  for(t=tree->child; t!=NULL; t=t->siblingR)
    n += writeLeaves(t, flag);

  return n;
}

int fwriteLeaves(FILE *f, Tree *tree, int flag)
{
  Tree *t;
  int n = 0;

  if( tree==NULL )
    return;

  if( tree->child == NULL )
    if( strcmp(tree->pos, "-NONE-") ) /* bogus lex in Gold trees */
      {
	if( flag )
	  fprintf(f, "%s ", tree->lex);
	n = 1;
      }

  for(t=tree->child; t!=NULL; t=t->siblingR)
    n += fwriteLeaves(f, t, flag);

  return n;
}


void writePOSLeaves(Tree *tree)
{
  Tree *t;

  if( tree==NULL )
    return;

  if( tree->child == NULL )
    printf("%s ", tree->pos);

  for(t=tree->child; t!=NULL; t=t->siblingR)
    writePOSLeaves(t);
}


void writePOSClusters(Tree *tree)
{
  Tree *t;
  int h;

  if( tree->child==NULL )
    return;

  h = height(tree);
  if( h>=2 && h<=3 )
    {
      writePOSLeaves(tree);
      printf(":: ");
    }
  else
    for(t=tree->child; t!=NULL; t=t->siblingR)
      writePOSClusters(t);
}

Tree* deepCopy(Tree *tree, Tree *siblingL)
{
  Tree *t;

  if( tree==NULL )
    return 0;

  if( (t = (Tree*)malloc(sizeof(Tree)))==NULL )
    { fprintf(stderr, "Cannot allocate in deepCopy()\n"); exit(4); }

  t->b = tree->b;
  t->e = tree->e;
  t->pos = strdup(tree->pos);
  t->lex = strdup(tree->lex);

  t->child = deepCopy(tree->child, 0);
  t->siblingR = deepCopy(tree->siblingR, t);
  t->siblingL = siblingL;

  return t;
}

void freeTree(Tree *tree)
{
  Tree *t;

  if( tree==NULL )
    return;

  for(t=tree->child; t!=NULL; t=t->siblingR)
    freeTree(t);

  allb -= sizeof(Tree);
  free(tree); 
}

int sameString(char *ts, char *ds)
{
  char tmpts[200];

  getLex(tmpts, ts);

  if( !strcmp(tmpts, ds) )
    return 1;

  return 0;
}

int sameCompactString(char *ts, char *ds)
{
  char *cts, *cds;
  char buf[2000];
  int i, r=0;

  if( (cts = (char *)calloc(strlen(ts)+1, sizeof(char)))==NULL )
    { fprintf(stderr, "Cannot allocate in sameCompactString()\n"); exit(11); }
  if( (cds = (char *)calloc(strlen(ds)+1, sizeof(char)))==NULL )
    { fprintf(stderr, "Cannot allocate in sameCompactString()\n"); exit(11); }
  *cts = 0;
  *cds = 0;

  for(i=0; i<=strlen(ts); )
    {
      sscanf(ts+i, "%s", buf);
      strcat(cts, buf);
      i += strlen(buf) + 1;
    }

  for(i=0; i<=strlen(ds); )
    {
      sscanf(ds+i, "%s", buf);
      strcat(cds, buf);
      i += strlen(buf) + 1;
    }

  if( !strcmp(cts, cds) )
    r = 1;

  free(cts);
  free(cds);

  return r;
}

int almostEqual(char *s1, char *s2)
{
  double ratio = 0.80;
  char buf1[200], buf2[200];
  int i, j, hits1=0, hits2=0, total1=0, total2=0;

  for(i=0; i<strlen(s1);){
    sscanf(s1+i, "%s", buf1);
    for(j=0; j<strlen(s2);){
      sscanf(s2+j, "%s", buf2);
      if( !strcmp(buf1, buf2) ){
	hits1 += 1;
	break;
      }
      j += strlen(buf2)+1;
    }
    i += strlen(buf1)+1;
    total1 += 1;
  }

  for(j=0; j<strlen(s2);){
    sscanf(s2+j, "%s", buf2);
    for(i=0; i<strlen(s1);){
      sscanf(s1+i, "%s", buf1);
      if( !strcmp(buf1, buf2) ){
	  hits2 += 1;
	  break;
      }
      i += strlen(buf1)+1;
    }
    j += strlen(buf2)+1;
    total2 += 1;
  }

  if( hits1/(double)total1 >= ratio && hits2/(double)total2 >= ratio )
    return 1;

  return 0;
}

void getLex(char *lex, char *fullex)
{
  int j;

  strcpy(lex, fullex);

  for(j=strlen(lex); j>0; j--)
    if( lex[j]=='[' )
      { lex[j] = 0; break; }

  if( !strcmp(lex, "Inc .") )
    strcpy(lex, "Inc.");
  
}

int getBLex(char *lex)
{
  int i, b = -1;

  for(i=strlen(lex); i>=0; i--)
    if( lex[i]=='[' )
      {
	sscanf(lex+i+1, "%d", &b);
	break;
      }
  return b;
}

int getELex(char *lex)
{
  int i, e = -1;

  for(i=strlen(lex); i>=0; i--)
    if( lex[i]=='-' )
      {
	sscanf(lex+i+1, "%d", &e);
	break;
      }
  return e;
}

int isPunctuation(char *s)
{
  if( s[0]=='.' || s[0]==',' || s[0]=='\'' || s[0]=='`' || 
      s[0]=='?' || s[0]=='!' || s[0]==':' || s[0]=='-' )
    return 1;

  return 0;
}

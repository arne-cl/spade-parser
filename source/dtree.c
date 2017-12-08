#include "dtree.h"
#include "myalloc.h"

int span;

DTree* readDTree(FILE *fti, int offset)
{
  DTree *tree;
  char *text = NULL;
  int pos=0, n=0, wc=0;

  span = 0;
  tree = readDTreeStruct(fti, NULL, &text, &pos, &wc, offset);
  updateText(tree, text);
  /*
  getSentenceTrees(tree, &n);
  getParagraphTrees(tree, &n);
  */
  updateMarks(tree);

  return tree;
}

DTree *readDTreeStruct(FILE *fti, DTree *lsib, char **text, int *pos, int *wc, int offset)
{
  DTree *tree;
  char buf[5];
  
  if( (tree = (DTree *) malloc(sizeof(DTree)))== NULL )
    { fprintf(stderr, "Out of memory\n"); exit(3); }

  tree->node = readNode(fti, text, pos, wc, offset);
  tree->child = NULL; 
  tree->sibling = NULL; 
  tree->bsibling = lsib;

  if( fscanf(fti, "%s", buf)==EOF )
    return tree;
  if( buf[0]==')' )
    tree->child = NULL;
  else /* ( */
    if( buf[0]=='(' )
      tree->child = readDTreeStruct(fti, NULL, text, pos, wc, offset);
    else
      { fprintf(stderr, "Keyword ( or ) missing for child: %s\n", buf); exit(4); }

  if( fscanf(fti, "%s", buf)==EOF || !strcmp(buf, "---") )
    return tree;
  if( buf[0]==')' )
    tree->sibling = NULL;
  else /* ( */
    if( buf[0]=='(' )
      tree->sibling = readDTreeStruct(fti, tree, text, pos, wc, offset);
    else
      { fprintf(stderr, "Keyword ( or ) missing for sibling: %s\n", buf); exit(4); }

  return tree;
}

Node *readNode(FILE *fti, char **text, int *pos, int *wc, int offset)
{
  char par, buf[500], bigbuf[30000], *newtext, *tbigbuf;
  Node *node;
  int i, j, k, len1=0, len2, nw;

  if( (node = (Node *) malloc(sizeof(Node)))== NULL )
    { fprintf(stderr, "Out of memory\n"); exit(3); }
  
  fscanf(fti, "%s", buf);
  node->status = strdup(buf);
  if( strcmp(buf, "Nucleus") && strcmp(buf, "Satellite") && strcmp(buf, "Root") )
    { fprintf(stderr, "Keyword Nucleaus/Satellite missing: %s\n", buf); exit(4); }

  fscanf(fti, "%s", buf);
  if( !strcmp(buf, "(span") )
    {
      fscanf(fti, "%d %d", &node->span[0], &node->span[1]);
      node->span[0] -= offset - 1; node->span[1] -= offset - 1; 
    }
  else /* leaf */
    if( !strcmp(buf, "(leaf") )
      { 
	fscanf(fti, "%d", &node->span[0]); 
	node->span[0] -= offset - 1; 
	node->span[1] = node->span[0];
	if( node->span[0] > span )
	  span =  node->span[0];
      }
    else
      { fprintf(stderr, "Keyword span/leaf missing: %s\n", buf); exit(4); }
  fscanf(fti, "%c", &par); 
  
  if( strcmp(node->status, "Root") )
    {
      fscanf(fti, "%s", buf);
      if( strcmp(buf, "(rel2par") )
	{ fprintf(stderr, "Keyword rel2par missing: %s\n", buf); exit(4); }
      fscanf(fti, "%s", buf);
      buf[strlen(buf)-1] = 0;  /* remove ) */
      node->rel2par = strdup(buf);
    }
  else
    node->rel2par = NULL; 
  
  if( node->span[1]==node->span[0] ) /*leaf*/
    {
      fscanf(fti, "%s", buf);
      if( strcmp(buf, "(text") )
	{ fprintf(stderr, "Keyword text missing\n"); exit(5); }
      fscanf(fti, "%s", buf);
      strcpy(bigbuf, buf+2);

      nw = 0;
      while( strcmp(buf+strlen(buf)-3, "_!)") )
	{
	  fscanf(fti, "%s", buf);
	  strcat(bigbuf, " ");
	  strcat(bigbuf, buf);
	  nw += 1;
	}
      bigbuf[strlen(bigbuf)-3] = 0; 
      /* remove blanks from begining and end */
      for(i=0; i<strlen(bigbuf); i++)
	if( bigbuf[i]!=' ' )
	  break;
      for(j=strlen(bigbuf)-1; j>=0; j--)
	if( bigbuf[j]!=' ' )
	  break;
      bigbuf[j+1]=0;
      tbigbuf = strdup(bigbuf+i);
      /*
      tokenizeE(&tbigbuf);
      */

      if( *text )
	len1 = strlen(*text);
      len2 = strlen(tbigbuf);
      if( (newtext = (char *) calloc(sizeof(char), len1+len2+2))==NULL )
	{ fprintf(stderr, "Cannot allocate for larger text.\n"); exit(12); }
      if( len1 )
	{
	  strcpy(newtext, *text);
	  strcat(newtext, " ");
	  free(*text);
	}
      strcat(newtext, tbigbuf);
      *text = newtext;
      free(tbigbuf);

      node->npieces = 1;
      node->btext = myIntPalloc(1);
      node->etext = myIntPalloc(1);
      node->btext[0] = *pos;
      *pos += len2-1; 
      node->etext[0] = *pos;
      *pos += 2;
      node->bw = myIntPalloc(1);
      node->ew = myIntPalloc(1);
      node->bw[0] = *wc;
      *wc += nw + 1;
      node->ew[0] = *wc;     
    }
    
  node->text = NULL;
  return node;
}

int updateText(DTree *tree, char *text)
{
  DTree *t;
  int i, j, npieces=0;

  if( tree->child==NULL )
    { 
      tree->node->text = text; 
      if( !strcmp(tree->node->status, "Nucleus") || 1 ) /* only nuclear rels || all */
	return 1;
      else /* Satellite */
	return 0;
    }

  for(t=tree->child; t!=NULL; t=t->sibling)
    if( updateText(t, text) ) 
      npieces += t->node->npieces;

  tree->node->npieces = npieces;
  tree->node->btext = myIntPalloc(npieces);
  tree->node->etext = myIntPalloc(npieces);
  tree->node->bw = myIntPalloc(npieces);
  tree->node->ew = myIntPalloc(npieces);
  
  for(t=tree->child, i=0; t!=NULL; t=t->sibling)
    if( !strcmp(t->node->status, "Nucleus") || 1 ) /* only nuclear rels || all */
      for(j=0; j<t->node->npieces; j++)
	{
	  tree->node->btext[i] = t->node->btext[j];
	  tree->node->etext[i] = t->node->etext[j];
	  tree->node->bw[i] = t->node->bw[j];
	  tree->node->ew[i] = t->node->ew[j];
	  i += 1;
	}
  if( i!=npieces )
    fprintf(stderr, "Error in updateText()\n");

  tree->node->text = text;

  if( !strcmp(tree->node->status, "Nucleus") || 1 ) /* only nuclear rels || all */
    return 1;
  else
    return 0;
}


void restoreSpans(DTree *tree)
{
  DTree *t;

  if( tree->child==NULL )
    return;

  for(t=tree->child; t->sibling!=NULL; t=t->sibling)
    restoreSpans(t);
  restoreSpans(t);

  tree->node->span[0] = tree->child->node->span[0];
  tree->node->span[1] = t->node->span[1];

}

void updateMarks(DTree *tree)
{
  DTree *t;
  int ar, cnt, cntP;
  char c;

  if( tree==NULL )
    return;

  for(t=tree->child; t!=NULL; t=t->sibling)
    updateMarks(t);

  if( tree->child )
    {
      for(t=tree->child, ar=0; t->sibling!=NULL; t=t->sibling, ar++)
	if( (t->node->mark==2 || t->sibling->node->mark==4) && t->sibling->node->mark==0 )
	  {
	    tree->node->mark = 4;
	    return;
	  }	

      for(t=tree->child, ar=0; t->sibling!=NULL; t=t->sibling, ar++)
	if( t->node->mark==0 && (t->sibling->node->mark==2 || t->sibling->node->mark==3) )
	  {
	    tree->node->mark = 3;
	    return;
	  }	

      for(t=tree->child, cnt=0, ar=0; t!=NULL; t=t->sibling, ar++)
	if( t->node->mark>0 )
	  cnt++;
      if( cnt==ar && ar>0 ) /* all children are marked > 0 */
	{
	  tree->node->mark = 2;
	  return;
	}	

      for(t=tree->child, ar=0; t->sibling!=NULL; t=t->sibling, ar++)
	if( t->node->mark==1 )
	  break;
      if( t->sibling==NULL && ar>0 && endSentence(t->node, 'A')>0 )
	{
	  tree->node->mark = 1;
	  return;
	}
      tree->node->mark = 0;
    }
  else
    {
      if( tree->sibling )
	c = tree->sibling->node->text[tree->sibling->node->btext[0]];
      else
	c = 'A';
      if( endSentence(tree->node, c) )
	if( tree->bsibling )
	  if( tree->bsibling->node->mark>0 )
	    tree->node->mark = 1;
	  else
	    tree->node->mark = 0;  
	else
	  tree->node->mark = 1;  
      else
	tree->node->mark = 0;  
    }
}

DTree *findParent(DTree *tree, DTree *ct)
{
  DTree *t, *rt;

  if( tree==NULL )
    return NULL;
  
  for(t=tree->child; t!=NULL; t=t->sibling)
    if( t==ct )
      return tree;
    else
      {
	rt = findParent(t, ct);
	if( rt )
	  return rt;
      }

  return NULL;
}

int replaceNodeKeepRel(DTree *tree, DTree *ct, DTree *rt)
{
  DTree *t;
  int r;

  if( tree==NULL )
    return 0;
  
  for(t=tree->child; t!=NULL; t=t->sibling)
    if( t==ct )
      {
	/* should keep the original relation, not just replace the node */
	if( t->bsibling )
	  t->bsibling->sibling = rt;
	else
	  tree->child = rt;
	return 1;
      }
    else
      {
	r = replaceNodeKeepRel(t, ct, rt);
	if( r )
	  return r;
      }

  return 0;
}

DTree *deepCopyDTree(DTree *dtree, DTree *bsibling)
{
  DTree *newtree;
  int i;
  
  if( dtree==NULL )
    return NULL;

  if( (newtree = (DTree *) malloc(sizeof(DTree)))== NULL )
    { fprintf(stderr, "Out of memory\n"); exit(3); }
  
  newtree->child = deepCopyDTree(dtree->child, 0);
  newtree->sibling = deepCopyDTree(dtree->sibling, newtree);
  newtree->bsibling = bsibling;

  if( (newtree->node = (Node *) malloc(sizeof(Node)))== NULL )
    { fprintf(stderr, "Out of memory\n"); exit(3); }
  newtree->node->status = strdup(dtree->node->status);
  if( dtree->node->rel2par )
    newtree->node->rel2par = strdup(dtree->node->rel2par);
  newtree->node->span[0] = dtree->node->span[0];
  newtree->node->span[1] = dtree->node->span[1];
  newtree->node->text = dtree->node->text; /* not deep copy on text */
  newtree->node->npieces = dtree->node->npieces;

  newtree->node->btext = myIntPalloc(newtree->node->npieces);
  newtree->node->etext = myIntPalloc(newtree->node->npieces);
  newtree->node->bw = myIntPalloc(newtree->node->npieces);
  newtree->node->ew = myIntPalloc(newtree->node->npieces);
  for(i=0; i<dtree->node->npieces; i++)
    {
      newtree->node->btext[i] = dtree->node->btext[i];
      newtree->node->etext[i] = dtree->node->etext[i];
      newtree->node->bw[i] = dtree->node->bw[i];
      newtree->node->ew[i] = dtree->node->ew[i];
    }

  return newtree;
}

int endSentence(Node *node, char nextC)
{
  int i, k, c;
  char buf[100];

  k = node->etext[node->npieces-1];
  if( c=quotingChar(node->text[k]) )
    if( c=='\'' )
      k -= 3;
    else
      k -= 2;

  if( node->text[k]!='.' )
    if( unambiguousEOSChar(node->text[k]) )
      return node->text[k];
    else
      return 0;

  if( node->text[k]=='.' )
    if( nextC>='A' && nextC <= 'Z' )
      return 1;
    else
      return 0;

  return 0;
}

int unambiguousEOSChar(char c)
{
  if( c=='>' || c=='!' || c=='?' )
    return 1;

  return 0;
}

char quotingChar(char c)
{
  if( c == '"' || c == '\'' || c == '`' )
    return c;

  return 0;
}


int endParagraph(Node *node)
{
  int k;

  k = node->etext[node->npieces-1];

  if( node->text[k]!='>' )
    return 0;
  else
    return 1;
}

void writeDTree(FILE *fto, DTree *tree, int tab)
{
  int i;
  DTree *dt;

  if( tree==NULL )
    return;

  for(i=0; i<tab; i++)
    fprintf(fto, " ");
  fprintf(fto, "( ");
  writeNode(fto, tree->node);
  if( tree->child )
    fprintf(fto, "\n");

  for(dt=tree->child; dt!=NULL; dt=dt->sibling)
    writeDTree(fto, dt, tab+2);

  if( tree->child )
    for(i=0; i<tab; i++)
      fprintf(fto, " ");
  fprintf(fto, " )\n");
  /* fprintf(fto, " )%d\n", tree->node->mark); */
}

void writeNode(FILE *fto, Node *node)
{
  int i, k;

  if( node==NULL )
    return;

  fprintf(fto, "%s ", node->status);
  if( node->span[1]!=node->span[0] )
    fprintf(fto, "(span %d %d) ", node->span[0], node->span[1]);
  else
    fprintf(fto, "(leaf %d) ", node->span[0]);

  if( strcmp(node->status, "Root") )
    fprintf(fto, "(rel2par %s) ", node->rel2par);

  if( node->span[1]==node->span[0] )
    {
      fprintf(fto, "(text _!");
      for(k=0; k<node->npieces; k++)
	{
	  for(i=node->btext[k]; i<=node->etext[k]; i++)
	    fprintf(fto, "%c", node->text[i]);
	  if( k<node->npieces-1 )
	    fprintf(fto, " ... ");
	}
      fprintf(fto, "_!)");
    }
}

int countLeaves(DTree *tree)
{
  DTree *t;
  int cnt=0;

  for(t=tree->child; t!=NULL; t=t->sibling)
    cnt += countLeaves(t);

  if(tree->child==NULL)
    cnt = 1;

  return cnt;
}

int treeHeight(DTree *tree)
{
  int max = 0, ch;
  DTree *t;

  if( tree == NULL )
    return 0;

  for(t=tree->child; t!=NULL; t=t->sibling)
    {
      ch = treeHeight(t);
      if( ch > max )
	max = ch;
    }

  return max + 1;

}

void relClass(char class[], char *rel)
{
  if( !rel ){ class[0] = 0; return; }

  if( !strcmp(rel+strlen(rel)-2, "-e") )
    rel[strlen(rel)-2] = 0;

  if( !strcmp(rel, "attribution") || !strcmp(rel, "attribution-n") ){
    strcpy(class, "Attribution"); return;
  }

  if( !strcmp(rel, "background") || !strcmp(rel, "circumstance")  ){
    strcpy(class, "Background"); return;
  }

  if( !strcmp(rel, "cause") || !strcmp(rel, "result") || 
      !strcmp(rel, "consequence-s") || !strcmp(rel, "consequence-n") ){
    strcpy(class, "Cause"); return;
  }
    
  if( !strcmp(rel, "comparison") || !strcmp(rel, "preference") || 
      !strcmp(rel, "analogy") || !strcmp(rel, "proportion") ){
    strcpy(class, "Comparison"); return;
  }
      
  if( !strcmp(rel, "condition") || !strcmp(rel, "hypothetical") || 
      !strcmp(rel, "contingency") || !strcmp(rel, "Otherwise") ){
    strcpy(class, "Condition"); return;
  }
      
  if( !strcmp(rel, "Contrast") || 
      !strcmp(rel, "concession") || !strcmp(rel, "antithesis") ){ 
    strcpy(class, "Contrast"); return;
  }
  
  if( !strcmp(rel, "elaboration-additional") || !strcmp(rel, "elaboration-general-specific") || 
      !strcmp(rel, "elaboration-part-whole") || !strcmp(rel, "elaboration-process-step") || 
      !strcmp(rel, "elaboration-object-attribute") || !strcmp(rel, "elaboration-set-member") || 
      !strcmp(rel, "example") || !strcmp(rel, "definition") ){
    strcpy(class, "Elaboration"); return;
  }

 if( !strcmp(rel, "purpose") || !strcmp(rel, "enablement") ){
    strcpy(class, "Enablement"); return;
  }

 if( !strcmp(rel, "Evaluation") || !strcmp(rel, "evaluation-n") || !strcmp(rel, "evaluation-s") ||
     !strcmp(rel, "interpretation-s") || !strcmp(rel, "interpretation-n") || !strcmp(rel, "Interpretation") || 
     !strcmp(rel, "conclusion") || !strcmp(rel, "comment") ){
    strcpy(class, "Evaluation"); return;
  }

 if( !strcmp(rel, "evidence") || !strcmp(rel, "explanation-argumentative") || 
     !strcmp(rel, "reason") || !strcmp(rel, "Reason") ){
    strcpy(class, "Explanation"); return;
  }
  
 if( !strcmp(rel, "List") || !strcmp(rel, "Disjunction") ){
    strcpy(class, "Joint"); return;
  }
  
 if( !strcmp(rel, "manner") || !strcmp(rel, "means") ){
    strcpy(class, "Manner-Means"); return;
  }
 
 if( !strcmp(rel, "Problem-Solution") || 
     !strcmp(rel, "Question-Answer") || !strcmp(rel, "question-answer-s") || 
     !strcmp(rel, "rhetorical-question") || !strcmp(rel, "Statement-Response") || 
     !strcmp(rel, "Topic-Comment") || !strcmp(rel, "Comment-Topic") ||
     !strcmp(rel, "comment") ){
    strcpy(class, "Topic-Comment"); return;
  }

 if( !strcmp(rel, "summary") || !strcmp(rel, "restatement") ){
    strcpy(class, "Summary"); return;
  }

 if( !strcmp(rel, "temporal-before") || !strcmp(rel, "temporal-after") || 
     !strcmp(rel, "temporal-same-time") || !strcmp(rel, "Temporal-Same-Time") || 
     !strcmp(rel, "Sequence") || !strcmp(rel, "Inverted-Sequence") ){
    strcpy(class, "Temporal"); return;
  }
   
 if( !strcmp(rel, "topic-shift") || !strcmp(rel, "topic-drift") ){
    strcpy(class, "Topic-Change"); return;
  }
   
 /* here? no class for you */
 strcpy(class, rel);

}

void nodeText(DTree *dtree, char bigbuf[])
{
  int i;
  char buf[200];

  bigbuf[0] = 0;
  for(i=dtree->node->btext[0]; i<=dtree->node->etext[dtree->node->npieces-1]; ){
    sscanf(dtree->node->text+i, "%s", buf);
    strcat(bigbuf, buf); strcat(bigbuf, " ");
    i += strlen(buf)+1;
  }
}

/***** Utility functions *****/

void getName(char fname[], char *allname)
{
  int i;

  for(i=strlen(allname); i>0; i--)
    if( allname[i]=='/' )
      break;

  strcpy(fname, allname+i+1);
}

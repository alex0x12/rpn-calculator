#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <stdarg.h>

#include "rpn_utils.h"

/* TOKEN */

static Token _opset[] =
{
  {"ln",FUNCTION,NONE,0},
  {"cos",FUNCTION,NONE,0},
  {"sin",FUNCTION,NONE,0},
  {"tan",FUNCTION,NONE,0},
  {"cot",FUNCTION,NONE,0},
  {"log",FUNCTION,NONE,0},
  {"exp",FUNCTION,NONE,0},
  {"min",FUNCTION,NONE,0},
  {"max",FUNCTION,NONE,0},
  {"sqrt",FUNCTION,NONE,0},

  {",",DELIM,NONE,0},

  {"p",CONSTANT,NONE,0},
  {"e",CONSTANT,NONE,0},

  {"(",LPAREN,NONE,0},
  {")",RPAREN,NONE,0},

  {"+",FLOATOP,LEFT,1},
  {"-",FLOATOP,LEFT,1},

  {"*",BINOP,LEFT,2},
  {"/",BINOP,LEFT,2},
  {"^",BINOP,RIGHT,3},

  {"%",UNAOP,RIGHT,4},
  {"!",UNAOP,RIGHT,4},

  // ADDITIONAL ALPHABET
  {"+%",BINOP,LEFT,1},
  {"-%",BINOP,LEFT,1},
  {"~",UNAOP,RIGHT,5}
};

static const size_t _opset_size = sizeof(_opset)/sizeof(_opset[0]);

static int _opcmp(const void* a, const void* b)
{
  Token* op_a=(Token*)a;
  Token* op_b=(Token*)b;
  return strcmp(op_a->sym,op_b->sym);
}

Token* create_token(const char* sym, token_t type, assoc_t assoc, int priority)
{
  Token* token = malloc(sizeof(Token));
  if(!token) return NULL;
  token->sym=strdup(sym);
  token->type=type;
  token->assoc=assoc;
  token->priority=priority;
  return token;
}

Token* find_token(const char* sym)
{
  if(!sym) return NULL;
  static bool sorted=false;
  if(!sorted)
  {
    qsort(_opset,_opset_size,sizeof(_opset[0]),_opcmp);
    sorted=true;
  }
  int l=0;
  int r=_opset_size-1;
  int m;
  while(l<=r)
  {
    m=l+((r-l)/2);
    char* item = _opset[m].sym;
    int rs=strcmp(sym,item);
    if(!rs)
    {
      Token* t = &_opset[m];
      return t;
    }
    else if(rs>0)
      l=m+1;
    else
      r=m-1;
  }
  return NULL;
}

Token* find_token_ch(const char ch)
{
  char s_ch[2]={0};
  s_ch[0]=ch;
  return find_token(s_ch);
}

void destroy_token(Token* token)
{
  if(!token) return;
  if(token->sym)
    free(token->sym);
  token->sym=NULL;
  free(token);
  token=NULL;
}

/* TOKEN */

/* STACK */

static int
_allocate(Stack** ptr, const Token* token ,Stack* next)
{
  (*ptr)=malloc(sizeof(Stack));
  (*ptr)->next=next;
  (*ptr)->token=create_token(token->sym,token->type,token->assoc,token->priority);
  return 0;
}

static int
_allocate_inplace(Stack** ptr, const char* sym, token_t type, assoc_t assoc, int priority ,Stack* next)
{
  (*ptr)=malloc(sizeof(Stack));
  (*ptr)->next=next;
  (*ptr)->token=create_token(sym,type,assoc,priority);
  return 0;
}

int
push(Stack** head,const Token* token)
{
   Stack* tmp=NULL;
   int rs=0;
   if(!head)
     rs=_allocate(head,token,tmp);
   else
   {
     rs=_allocate(&tmp,token,(*head));
     *(head)=tmp;
   }
   return rs;
}

int
push_inplace(Stack** head,const char* sym, token_t type, assoc_t assoc, int priority)
{
   Stack* tmp=NULL;
   int rs=0;
   if(!head)
     rs=_allocate_inplace(head,sym,type,assoc,priority,tmp);
   else
   {
     rs=_allocate_inplace(&tmp,sym,type,assoc,priority,(*head));
     *(head)=tmp;
   }
   return rs;
}

int
pop(Stack** head)
{
  if(!(*head)) return -1;
  Stack* tmp=(*head);
  (*head)=(*head)->next;
  destroy_token(tmp->token);
  free(tmp);
  tmp=NULL;
  return 0;
}

Token* top(Stack** head)
{
  if(!(*head)) return NULL;
  Token* t = (*head)->token;
  return t;
}

size_t
get_size(Stack** head)
{
  Stack* tmp=(*head);
  size_t size=0;
  while(tmp)
  {
    size++;
    tmp=tmp->next;
  }
  return size;
}

void destroy_stack(Stack** head)
{
  while((*head))
    pop(head);
}

/* STACK */

/* RPN */

void str_construct(char** dest, ...)
{
  va_list p;
  va_start(p,dest);
  char* arg;

  // for strlen to work properly
  if(!(*dest))
    *dest=calloc(1,1);
  while((arg=va_arg(p,char*))!=NULL)
  {
    size_t s=strlen(*dest)+strlen(arg)+1;
    char* t=realloc(*dest,s);
    *dest=t;
    strcat(*dest,arg);
  }
  va_end(p);
}

void str_append(char** dest, char* str)
{
  str_construct(dest,str," ",NULL);
}

/* RPN */

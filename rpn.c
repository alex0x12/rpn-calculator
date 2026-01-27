#include <stdio.h>
#include <ctype.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <getopt.h>

#include <math.h>

#include "rpn_utils.h"

static const unsigned char PRECISION_MAX=30;
static unsigned char PRECISION=3;
static char FORMAT[16] = "%0.*f";
static int BRIEF_FLAG = 0;

static double fact(double v)
{
  if(v<=1) return 1;
  return v * fact(v-1);
}

static void rpn_count(const Token* token, status_t status)
{
  static Stack *st=NULL;
  char* endptr=NULL;
  #ifdef DEBUG
  if(status!=HALT)
  {
    static int calls=0;
    printf("%s(%03d): - Token: %s\n",__func__,calls,token->sym);
    calls++;
  }
  #endif
  if(status==HALT)
  {
    if(!BRIEF_FLAG)
      printf("Res: %s\n",top(&st)->sym);
    else
      printf("%s\n",top(&st)->sym);
    destroy_stack(&st);
    st=NULL;
    return;
  }
  if(!token || !token->sym) return;

  token_t type = token->type;
  char* sym = token->sym;
  if(type==NUMBER)
  {
    push(&st,token);
    return;
  }
  if (type==CONSTANT)
  {
    push(&st,token);
    free(top(&st)->sym);
    char buffer[64];
    if(!strcmp(sym,"e"))
    {
      snprintf(buffer,64,"%.17g",M_E);
    }
    else if(!strcmp(sym,"p"))
    {
      snprintf(buffer,64,"%.17g",M_PI);
    }
    top(&st)->sym=strdup(buffer);
    return;
  }
  double rs;
  if(type==UNAOP)
  {
    rs=strtod(top(&st)->sym,&endptr);
    if(!strcmp(sym,"%"))
      rs/=100.0;
    else if(!strcmp(sym,"!"))
      rs=fact(rs);
    else if(!strcmp(sym,"~"))
      rs=-1*rs;
    else
      return;
  }
  else if(type==BINOP||type==FLOATOP)
  {
    double lop,rop;
    rop=strtod(top(&st)->sym,&endptr);
    pop(&st);
    lop=strtod(top(&st)->sym,&endptr);
    if(!strcmp(sym,"+"))
      rs=lop+rop;
    else if(!strcmp(sym,"-"))
      rs=lop-rop;
    else if(!strcmp(sym,"*"))
      rs=lop*rop;
    else if(!strcmp(sym,"/"))
      rs=lop/rop;
    else if(!strcmp(sym,"^"))
      rs=pow(lop,rop);
    else if(!strcmp(sym,"+%"))
        rs=lop*(1.0+rop/100.0);
    else if(!strcmp(sym,"-%"))
        rs=lop*(1.0-rop/100.0);
    else
      return;
  }
  else if(type==FUNCTION)
  {
    double sarg, farg;
    sarg=strtod(top(&st)->sym,&endptr);
    if(!strcmp(sym,"ln"))
    {
      rs=log(sarg);
    }
    else if(!strcmp(sym,"cos"))
    {
      rs=cos(sarg);
    }
    else if(!strcmp(sym,"sin"))
    {
      rs=sin(sarg);
    }
    else if(!strcmp(sym,"tan"))
    {
      rs=tan(sarg);
    }
    else if(!strcmp(sym,"cot"))
    {
      rs=cos(sarg)/sin(sarg);
    }
    else if(!strcmp(sym,"exp"))
    {
      rs=exp(sarg);
    }
    else if(!strcmp(sym,"sqrt"))
    {
      rs=sqrt(sarg);
    }
    else
    {
      pop(&st);
      farg=strtod(top(&st)->sym,&endptr);
      if(!strcmp(sym,"log"))
      {
        rs=log(sarg)/log(farg);
      }
      else if(!strcmp(sym,"min"))
      {
        rs=fmin(farg,sarg);
      }
      else if(!strcmp(sym,"max"))
      {
        rs=fmax(farg,sarg);
      }
    }
  }

  char** t=&top(&st)->sym;
  *t=realloc(*t,64);
  snprintf(*t,64,FORMAT,PRECISION,rs);
}

static bool check_priority(const Token* st_top, const Token* token)
{
  if(!st_top || !token) return true;
  int f = st_top->priority;
  int s = token->priority;
  if(f>s) return false;
  if(f==s)
  {
    switch(token->assoc)
    {
      case LEFT: return false;
      case RIGHT: return true;
      default: return true;
    }
  }
  return true;
}

static void rpn_parse(const Token* token, status_t status)
{
  #ifdef DEBUG
  if(status!=HALT)
  {
    static int calls=0;
    printf("%s(%03d): - Token: %s\n",__func__,calls,token->sym);
    calls++;
  }
  #endif

  static char* rs=NULL;
  static Stack* st=NULL;
  if(status==HALT)
  {
    while(get_size(&st))
    {
      rpn_count(top(&st),ACTIVE);
      str_append(&rs,top(&st)->sym);
      pop(&st);
    }
    destroy_stack(&st);
    if(rs)
    {
      if(!BRIEF_FLAG)
        printf("RPN: %s\n",rs);
      free(rs);
    }
    rs=NULL;
    st=NULL;
    rpn_count(NULL,HALT);
    return;
  }
  if(!token || !token->sym) return;
  switch(token->type)
  {
    case NUMBER:
    case CONSTANT:
      rpn_count(token,ACTIVE);
      str_append(&rs,token->sym);
      break;
    case UNAOP:
      if(strcmp(token->sym,"~"))
      {
        rpn_count(token,ACTIVE);
        str_append(&rs,token->sym);
      }
      else
        push(&st,token);
      break;
    case FUNCTION:
      push(&st,token);
      break;
    case LPAREN:
      push(&st,token);
      break;
    case FLOATOP:
    case BINOP:
    {
      while(get_size(&st) && !check_priority(top(&st),token))
      {
        rpn_count(top(&st),ACTIVE);
        str_append(&rs,top(&st)->sym);
        pop(&st);
      }
      push(&st,token);
      break;
    }
    case RPAREN:
      while(get_size(&st) && top(&st)->type!=LPAREN)
      {
        rpn_count(top(&st),ACTIVE);
        str_append(&rs,top(&st)->sym);
        pop(&st);
      }
      pop(&st);
      if(get_size(&st) && top(&st)->type==FUNCTION)
      {
        rpn_count(top(&st),ACTIVE);
        str_append(&rs,top(&st)->sym);
        pop(&st);
      }
      break;
    case DELIM:
      while(get_size(&st) && top(&st)->type!=LPAREN)
      {
        rpn_count(top(&st),ACTIVE);
        str_append(&rs,top(&st)->sym);
        pop(&st);
      }
      break;
    default:
      break;
  }

}

static void tokenize(const char* const input)
{

  char* arg = strdup(input);
  size_t len=strlen(arg);
  if(!len) return;
  size_t i=0;
  const Token* token=NULL;
  token_t prev_type=UNDEF;
  while(i<len)
  {
    if(isdigit(arg[i]))
    {
      char buffer[64]={0};
      size_t j=0;
      bool found_dot=false;
      while(isdigit(arg[i])||arg[i]=='.')
      {
        if(arg[i]=='.')
        {
          if(!found_dot)
            found_dot=true;
          else
          {
            i++;
            continue;
          }
        }
        buffer[j++]=arg[i++];
      }
      buffer[j]=0;

      prev_type=NUMBER;
      Token* tmp=create_token(buffer,NUMBER,NONE,0);
      rpn_parse(tmp,ACTIVE);
      destroy_token(tmp);
      continue;
    }
    else if(isalpha(arg[i]))
    {
      char buffer[64]={0};
      size_t j=0;
      while(isalpha(arg[i]))
        buffer[j++]=arg[i++];
      buffer[j]=0;

      token=find_token(buffer);
      if(token)
      {
        prev_type=token->type;
        rpn_parse(token,ACTIVE);
      }
      continue;
    }

    token=find_token_ch(arg[i]);

    if(!token)
    {
      i++;
      continue;
    }

    if(token->type==FLOATOP)
    {
      token_t cur_type=UNDEF;
      if(!prev_type)
      {
        cur_type=UNAOP;
      }
      else
      {
        switch(prev_type)
        {
          case LPAREN:
          case BINOP:
          case DELIM:
          case FLOATOP:
          	cur_type=UNAOP;
          	break;
          default:
            cur_type=BINOP;
            break;
        }
      }
      if(cur_type==UNAOP)
      {
        if(!strcmp(token->sym,"-"))
        {
          const Token* uminus = find_token("~");
          rpn_parse(uminus,ACTIVE);
        }
        prev_type=cur_type;
        i++;
        continue;
      }
      else
      {
        size_t j=i+1;
        const Token* inner_token=NULL;
        Stack* st = NULL;
        while(j<len)
        {
          if((inner_token=find_token_ch(arg[j])))
          {
            if(!strcmp(inner_token->sym,"%")&&!get_size(&st))
            {
                arg[j]=0x20;
                if(!strcmp(token->sym,"+"))
                    token=find_token("+%");
                else
                    token=find_token("-%");
                break;
            }
            switch (inner_token->type)
            {
              case BINOP:
              case FLOATOP:
                if(!get_size(&st))
                  goto tokenize_end;
                else
                  break;
              case LPAREN:
                push(&st,inner_token);
                break;
              case RPAREN:
                pop(&st);
                break;
            default:
              break;
            }
          }
          j++;
        }
      }
    }

    tokenize_end:
    prev_type=token->type;
    rpn_parse(token,ACTIVE);
    i++;
  }
  rpn_parse(token,HALT);
  free(arg);
}



int main(int argc, char** argv)
{
  if(argc<2)
  {
    fprintf(stderr,"Example: %s -p 5 -b -- '1+2' '2*2' 'log(2,8)'\n\n",argv[0]);
    fprintf(stderr,"-- is for splitting options and arguments.\n");
    fprintf(stderr,"-p, --precision {uint} is for precision.\n");
    fprintf(stderr,"-b, --brief is for brief output\n");
    fprintf(stderr,"You must provide at least 1 string!\n");
    return 1;
  }

  int opt;
  const struct option long_opts[]=
  {
    {"brief",no_argument,&BRIEF_FLAG,'b'},
    {"precision",required_argument,NULL,'p'},
    {NULL,0,NULL,0}
  };
  const char* const short_opts="bp:";
  while((opt=getopt_long_only(argc,argv,short_opts,long_opts,NULL))!=-1)
  {
    switch(opt)
    {
      case '?':
        return 1;
      case 'b':
        BRIEF_FLAG=1;
        break;
      case 'p':
        size_t i=0;
        while(isdigit(optarg[i]))
          i++;
        if(i<strlen(optarg))
        {
          fprintf(stderr,"--precision|-p: only non-negative numbers allowed!\n");
          return 1;
        }
        unsigned char tmp=atoi(optarg);
        PRECISION=tmp>PRECISION_MAX?PRECISION_MAX:tmp;
        break;
      default:
        break;
    }
  }
  size_t i=1;
  while(optind<argc)
  {
    if(!BRIEF_FLAG)
      printf("\nExpression %zu: [ %s ]\n",i++,argv[optind]);
    tokenize(argv[optind++]);
  }
  return 0;
}

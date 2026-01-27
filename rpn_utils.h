#ifndef RPN_UTILS_H
#define RPN_UTILS_H

typedef enum
{
  UNDEF,
  FUNCTION,
  DELIM,
  BINOP,
  UNAOP,
  FLOATOP,
  NUMBER,
  CONSTANT,
  LPAREN,
  RPAREN
} token_t;

typedef enum
{
  NONE,
  LEFT,
  RIGHT
} assoc_t;

typedef struct
{
  char* sym;
  token_t type;
  assoc_t assoc;
  int priority;
} Token;

Token* find_token(const char*  sym);
Token* find_token_ch(const char ch);
Token* create_token(const char* sym, token_t type, assoc_t assoc, int priority);
void destroy_token(Token* token);

/* STACK */
typedef struct node Stack;
struct node
{
  Token* token;
  Stack* next;
};

int          push_inplace(Stack** head, const char* sym, token_t type, assoc_t assoc, int priority);
int          push(Stack** head, const Token *token);
int          pop(Stack** head);
Token*       top(Stack** head);
size_t       get_size(Stack** head);
void         destroy_stack(Stack** head);

/* STACK */

/* RPN */

typedef enum
{
  HALT,
  ACTIVE
} status_t;

void str_append(char** dest, char* str);
void str_construct(char** dest, ...);

/* RPN */

#endif /* RPN_UTILS_H */

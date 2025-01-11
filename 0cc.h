#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// トークンの種類
typedef enum {
  TK_RESERVED,  // 記号
  TK_IDENT,     // 識別子
  TK_NUM,       // 整数トークン
  TK_EOF,       // 入力の終わりを表すトークン
} TokenKind;

typedef struct Token Token;

struct Token {
  TokenKind kind; // トークンの型
  Token *next;    // 次の入力トークン
  int val;        // kindがTK_NUMの場合、その数値
  char *str;      // トークン文字列
  int len;        // トークンの長さ
};

typedef enum {
  ND_ADD,
  ND_SUB,
  ND_MUL,
  ND_DIV,
  ND_MOD,
  ND_EQU,
  ND_NEQ,
  ND_LES,
  ND_LEQ,
  ND_ASSIGN,
  ND_NUM,
  ND_LVAR,
  ND_RETURN,
  ND_IF,
  ND_IFELSE,
  ND_WHILE,
  ND_FOR_INIT,
  ND_FOR_COND,
  ND_FOR_UPDT_STMT,
  ND_BLOCK,
  ND_FUNC_CALL,
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;                // ND_NUMの値, if/for/whileなどのlabel_index
  int offset;             // ND_LVAR
  Node *nodes[100];       // ND_BLOCKの中身
  char *func_name;        // ND_FUNC_CALL
};

typedef struct LVar LVar;

struct LVar {
  LVar *next;
  char *name;
  int len;
  int offset;
};

// 入力プログラム
extern char *user_input;

// 現在着目しているトークン
extern Token *token;

// stmtのASTの配列
extern Node *code[100];

// ローカル変数
extern LVar *locals;

// ラベルの unique identifier
extern int label_index;

// parse.c

void error(char *fmt, ...);
void error_at(char *loc, char *fmt, ...);

bool consume(char *op);
void expect(char *op);
int expect_number();
Token *consume_ident();
bool at_eof();
Token *new_token(TokenKind kind, Token *cur, char *str);
Token *tokenize(char *p);

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
void program();
Node *stmt();
Node *expr();
Node *assign();
Node *equality();
Node *relational();
Node *add();
Node *mul();
Node *primary();
Node *unary();

// codegen.c

void gen_lval(Node *node);
void gen_if(Node *node);
void gen_for_cond(Node *node);
void gen_for_updt_stmt(Node *node);
void gen(Node *node);

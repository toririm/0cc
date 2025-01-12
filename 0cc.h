#include <ctype.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// 関数の引数レジスタ
extern char *ARG_RGST[];

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
  ND_FUNC,
} NodeKind;

typedef struct Node Node;

struct Node {
  NodeKind kind;
  Node *lhs;
  Node *rhs;
  int val;                // ND_NUMの値, if/for/whileなどのlabel_index
  int offset;             // ND_LVARの変数, ND_FUNCの総変数のベースポインタからのオフセット
  Node *stmts[100];       // ND_BLOCK, ND_FUNCの中身
  Node *args[7];          // ND_FUNC, ND_FUNC_CALLの引数, 6 + NULL
  char *name;             // ND_FUNC, ND_FUNC_CALLの関数名
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

// funcのASTの配列
extern Node *code[100];

// ローカル変数
// パーサは関数ごとのパースに対して独立なので
// 一時変数としてグローバルに定義しても問題ない
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
bool is_alnum(char c);
LVar *find_lvar(Token *tok);
LVar *new_lvar(char *name, int len);
char *strcopy_n(char *src, int n);
Token *new_token(TokenKind kind, Token *cur, char *str);
Token *tokenize(char *p);

Node *new_node(NodeKind kind, Node *lhs, Node *rhs);
Node *new_node_num(int val);
Node *new_node_lvar(Token *tok);
void program();
Node *func();
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
void gen_func_call(Node *node);
void gen(Node *node);
void gen_func(Node *node);

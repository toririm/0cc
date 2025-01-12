#include "0cc.h"

// エラーを報告するための関数
// printfと同じ引数を取る
void error(char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// エラー箇所を報告する
void error_at(char *loc, char *fmt, ...) {
  va_list ap;
  va_start(ap, fmt);

  int pos = loc - user_input;
  fprintf(stderr, "%s\n", user_input);
  fprintf(stderr, "%*s", pos, " "); // pos個の空白を出力
  fprintf(stderr, "^ ");
  vfprintf(stderr, fmt, ap);
  fprintf(stderr, "\n");
  exit(1);
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進めて
// 真を返す。それ以外の場合には偽を返す。
bool consume(char *op) {
  if (
      token->kind != TK_RESERVED
      || strlen(op) != token->len
      || memcmp(token->str, op, token->len)
    )
    return false;
  token = token->next;
  return true;
}

// 次のトークンが期待している記号のときには、トークンを1つ読み進める。
// それ以外の場合にはエラーを報告する。
void expect(char *op) {
  if (
      token->kind != TK_RESERVED
      || strlen(op) != token->len
      || memcmp(token->str, op, token->len)
    )
    error_at(token->str, "'%c'ではありません", *op);
  token = token->next;
}

// 次のトークンが数値の場合、トークンを1つ読み進めてその数値を返す。
// それ以外の場合にはエラーを報告する。
int expect_number() {
  if (token->kind != TK_NUM)
    error_at(token->str, "数ではありません");
  int val = token->val;
  token = token->next;
  return val;
}

// 次のトークンが識別子の場合、トークンを1つ読み進めてそのトークンを返す。
// それ以外の場合にはNULLポインタを返す。
Token *consume_ident() {
  if (token->kind != TK_IDENT)
    return NULL;
  Token *cur = token;
  token = token->next;
  return cur;
}

bool at_eof() {
  return token->kind == TK_EOF;
}

bool is_alnum(char c) {
  return ('a' <= c && c <= 'z') ||
          ('A' <= c && c <= 'Z') ||
          ('0' <= c && c <= '9') ||
          (c == '_');
}

LVar *find_lvar(Token *tok) {
  for (LVar *var = locals; var; var = var->next)
    if (var->len == tok->len && !memcmp(tok->str, var->name, var->len))
      return var;
  return NULL;
}

LVar *new_lvar(char *name, int len) {
  LVar *lvar = calloc(1, sizeof(LVar));
  lvar->next = locals;
  lvar->name = name;
  lvar->len = len;
  lvar->offset = locals->offset + 8;
  locals = lvar;
  return lvar;
}

/*
  srcのn文字+ヌル文字\0のn+1のcharを新たに確保し、先頭のポインタを返す
*/
char *strcopy_n(char *src, int n) {
  char *rtn = calloc(n + 1, sizeof(char));
  strncpy(rtn, src, n);
  rtn[n] = '\0';
  return rtn;
}

// 新しいトークンを作成してcurに繋げる
// このあとに cur->len を指定してやる必要がある
Token *new_token(TokenKind kind, Token *cur, char *str) {
  Token *tok = calloc(1, sizeof(Token));
  tok->kind = kind;
  tok->str = str;
  cur->next = tok;
  return tok;
}

// 入力文字列pをトークナイズしてそれを返す
Token *tokenize(char *p) {
  Token head;
  head.next = NULL;
  Token *cur = &head;

  while (*p) {
    // 空白文字をスキップ
    if (isspace(*p)) {
      p++;
      continue;
    }

    if (strncmp(p, "return", 6) == 0 && !is_alnum(p[6])) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 6;
      p += 6;
      continue;
    }

    if (strncmp(p, "if", 2) == 0 && !is_alnum(p[2])) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 2;
      p += 2;
      continue;
    }

    if (strncmp(p, "else", 4) == 0 && !is_alnum(p[4])) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 4;
      p += 4;
      continue;
    }

    if (strncmp(p, "while", 5) == 0 && !is_alnum(p[5])) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 5;
      p += 5;
      continue;
    }

    if (strncmp(p, "for", 3) == 0 && !is_alnum(p[3])) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 3;
      p += 3;
      continue;
    }

    if (
      !memcmp(p, "==", 2)
      || !memcmp(p, "!=", 2)
      || !memcmp(p, "<=", 2)
      || !memcmp(p, ">=", 2)
    ) {
      cur = new_token(TK_RESERVED, cur, p);
      cur->len = 2;
      p+=2;
      continue;
    }

    if (strchr("+-*/%()<>=;{},", *p)) {
      cur = new_token(TK_RESERVED, cur, p++);
      cur->len = 1;
      continue;
    }

    if (isdigit(*p)) {
      char *head = p;
      cur = new_token(TK_NUM, cur, p);
      cur->val = strtol(p, &p, 10);
      cur->len = p - head;
      continue;
    }

    if (is_alnum(*p)) {
      cur = new_token(TK_IDENT, cur, p);
      while (is_alnum(*p))
        p++;
      cur->len = p - cur->str;
      continue;
    }

    error_at(p, "トークナイズできません");
  }

  new_token(TK_EOF, cur, p);
  return head.next;
}

/*
program    = func*
func       = ident "(" (ident ("," ident)* )? ")" "{" stmt* "}"
stmt       = expr ";"
           | "{" stmt* "}"
           | "return" expr ";"
           | "if" "(" expr ")" stmt ("else" stmt)?
           | "while" "(" expr ")" stmt
           | "for" "(" expr? ";" expr? ";" expr? ")" stmt
expr       = assign
assign     = equality ("=" assign)?
equality   = relational ("==" relational | "!=" relational)*
relational = add ("<" add | "<=" add | ">" add | ">=" add)*
add        = mul ("+" mul | "-" mul)*
mul        = unary ("*" unary | "/" unary | "%" unary)*
unary      = ("+" | "-")? primary
primary    = num
           | ident ("(" (expr ("," expr)* )? ")")?
           | "(" expr ")"
*/

Node *new_node(NodeKind kind, Node *lhs, Node *rhs) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = kind;
  node->lhs = lhs;
  node->rhs = rhs;
  return node;
}

Node *new_node_num(int val) {
  Node *node = calloc(1, sizeof(Node));
  node->kind = ND_NUM;
  node->val = val;
  return node;
}

Node *new_node_lvar(Token *tok) {
  Node *node = new_node(ND_LVAR, NULL, NULL);
    
  LVar *lvar = find_lvar(tok);
  if (!lvar) {
    lvar = new_lvar(tok->str, tok->len);
  }
  node->offset = lvar->offset;

  return node;
}

void program() {
  int i = 0;
  while (!at_eof()) {
    locals = calloc(1, sizeof(LVar));
    code[i++] = func();
  }
  code[i] = NULL;
}

Node *func() {
  Token *tok;

  tok = consume_ident();
  if (!tok) error("関数の識別子が不正です\n");
  expect("(");

  Node *node = new_node(ND_FUNC, NULL, NULL);
  node->name = strcopy_n(tok->str, tok->len);

  int arg_idx = 0;
  while (!consume(")")) {
    if (arg_idx > 0) expect(",");

    tok = consume_ident();
    if (!tok) error("関数の引数が不正です\n");

    node->args[arg_idx++] = new_node_lvar(tok);
  }
  node->args[arg_idx] = NULL;

  expect("{");
  int stmt_idx = 0;
  while (!consume("}"))
    node->stmts[stmt_idx++] = stmt();
  node->stmts[stmt_idx] = NULL;

  node->offset = locals->offset;

  return node;
}

Node *stmt() {
  Node *node;

  if (consume("if")) {
    expect("(");
    Node *cond = expr();
    expect(")");
    Node *if_node = stmt();
    node = new_node(ND_IF, cond, if_node);
    // "if" の識別番号をvalに保存する
    node->val = label_index++;

    if (consume("else")) {
      Node *else_node = stmt();
      node->rhs = new_node(ND_IFELSE, node->rhs, else_node);
      node->rhs->val = node->val;
    }

    return node;
  }

  if (consume("while")) {
    expect("(");
    Node *cond = expr();
    expect(")");
    Node *while_node = stmt();
    node = new_node(ND_WHILE, cond, while_node);
    node->val = label_index++;
    return node;
  }

  if (consume("for")) {
    expect("(");
    Node *init = NULL;
    Node *cond = NULL;
    Node *updt = NULL;
    if (!consume(";")) {
      init = expr();
      expect(";");
    }
    if (!consume(";")) {
      cond = expr();
      expect(";");
    }
    if (!consume(")")) {
      updt = expr();
      expect(")");
    }
    Node *for_node = stmt();

    Node *for_updt_stmt = new_node(ND_FOR_UPDT_STMT, updt, for_node);
    Node *for_cond = new_node(ND_FOR_COND, cond, for_updt_stmt);
    node = new_node(ND_FOR_INIT, init, for_cond);
    node->val = for_cond->val = for_updt_stmt->val = label_index++;

    return node;
  }

  if (consume("{")) {
    node = new_node(ND_BLOCK, NULL, NULL);
    int i = 0;
    // ブロック内の stmts を保存する
    while (!consume("}")) {
      node->stmts[i++] = stmt();
    }
    node->stmts[i] = NULL;
    return node;
  }


  if (consume("return")) {
    node = new_node(ND_RETURN, expr(), NULL);
  } else {
    node = expr();
  }
  
  expect(";");
  return node;
}

Node *expr() {
  return assign();
}

Node *assign() {
  Node *node = equality();
  if (consume("=")) {
    return new_node(ND_ASSIGN, node, assign());
  }
  return node;
}

Node *equality() {
  Node *node = relational();

  for (;;) {
    if (consume("=="))
      node = new_node(ND_EQU, node, relational());
    else if (consume("!="))
      node = new_node(ND_NEQ, node, relational());
    else
      return node;
  }
}

Node *relational() {
  Node *node = add();

  for (;;) {
    if (consume("<"))
      node = new_node(ND_LES, node, add());
    else if (consume("<="))
      node = new_node(ND_LEQ, node, add());
    else if (consume(">"))
      node = new_node(ND_LES, add(), node);
    else if (consume(">="))
      node = new_node(ND_LEQ, add(), node);
    else
      return node;
  }
}

Node *add() {
  Node *node = mul();

  for (;;) {
    if (consume("+"))
      node = new_node(ND_ADD, node, mul());
    else if (consume("-"))
      node = new_node(ND_SUB, node, mul());
    else
      return node;
  }
}

Node *mul() {
  Node *node = unary();

  for (;;) {
    if (consume("*"))
      node = new_node(ND_MUL, node, unary());
    else if (consume("/"))
      node = new_node(ND_DIV, node, unary());
    else if (consume("%"))
      node = new_node(ND_MOD, node, unary());
    else
      return node;
  }
}

Node *primary() {
  if (consume("(")) {
    Node *node = expr();
    expect(")");
    return node;
  }

  Token *tok = consume_ident();
  if (tok) {
    Node *node;

    if (consume("(")) {
      node = new_node(ND_FUNC_CALL, NULL, NULL);
      node->val = label_index++;
      node->name = strcopy_n(tok->str, tok->len);
      int i = 0;
      while (!consume(")")) {
        if (i > 0) expect(",");
        node->args[i++] = expr();
      }
      node->args[i] = NULL;
      return node;
    }

    return new_node_lvar(tok);
  }

  return new_node_num(expect_number());
}

Node *unary() {
  if (consume("+"))
    return unary();
  if (consume("-"))
    return new_node(ND_SUB, new_node_num(0), unary());
  return primary();
}

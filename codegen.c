#include "0cc.h"

// スタックのトップに変数のアドレスを置く
void gen_lval(Node *node) {
  if (node->kind != ND_LVAR)
    error("代入の左辺値が変数ではありません");
  
  printf("  mov rax, rbp\n");
  printf("  sub rax, %d\n", node->offset);
  printf("  push rax\n");
}

void gen_if(Node *node) {
  if (node->kind != ND_IFELSE) {
    printf("  je .Lend%d\n", node->val);
    gen(node);
    return;
  }
  
  printf("  je .Lelse%d\n", node->val);
  gen(node->lhs);
  printf("  jmp .Lend%d\n", node->val);
  printf(".Lelse%d:\n", node->val);
  gen(node->rhs);
}

void gen_for_cond(Node *node) {
  if (node->kind != ND_FOR_COND)
    error("不正なASTです\n");
  
  if (node->lhs) {
    gen(node->lhs);
    printf("  pop rax\n");
    printf("  cmp rax, 0\n");
    printf("  je .Lend%d\n", node->val);
  }
  
  // ND_FOR_UPDT_STMT の処理を委譲する
  gen_for_updt_stmt(node->rhs);
}

void gen_for_updt_stmt(Node *node) {
  if (node->kind != ND_FOR_UPDT_STMT)
    error("不正なASTです\n");
  
  gen(node->rhs);
  if (node->lhs)
    gen(node->lhs);
  printf("  jmp .Lbegin%d\n", node->val);
}

void gen_func_call(Node *node) {

  int i = 0;
  for (; node->args[i] && i < 6; i++) {
    gen(node->args[i]);
  }
  i--;
  while (i >= 0) printf("  pop %s\n", ARG_RGST[i--]);

  /*
    x86-64のABIのため
    RSPが16の倍数になるように調整
  */

  // 現在のRSPを保存
  printf("  mov rax, rsp\n");
  // 16バイトアラインメントをチェック
  printf("  and rax, 15\n");
  printf("  jz .Lcall%d\n", node->val);
  // アラインメントが必要な場合
  printf("  sub rsp, 8\n");
  printf("  call %s\n", node->name);
  printf("  add rsp, 8\n");
  printf("  jmp .Lend%d\n", node->val);
  // アラインメントが不要な場合
  printf(".Lcall%d:\n", node->val);
  printf("  call %s\n", node->name);
  printf(".Lend%d:\n", node->val);
  printf("  push rax\n");
}

void gen(Node *node) {
  switch (node->kind) {
    case ND_NUM:
      printf("  push %d\n", node->val);
      return;
    case ND_LVAR:
      gen_lval(node);
      // ここでスタックトップのアドレスのメモリに保存されている値を取り出す
      printf("  pop rax\n");
      printf("  mov rax, [rax]\n");
      printf("  push rax\n");
      return;
    case ND_ASSIGN:
      gen_lval(node->lhs);
      gen(node->rhs);

      printf("  pop rdi\n");
      printf("  pop rax\n");
      printf("  mov [rax], rdi\n");
      printf("  push rdi\n");
      return;
    case ND_RETURN:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  mov rsp, rbp\n");
      printf("  pop rbp\n");
      printf("  ret\n");
      return;
    case ND_IF:
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      gen_if(node->rhs);
      printf(".Lend%d:\n", node->val);
      return;
    case ND_WHILE:
      printf(".Lbegin%d:\n", node->val);
      gen(node->lhs);
      printf("  pop rax\n");
      printf("  cmp rax, 0\n");
      printf("  je .Lend%d\n", node->val);
      gen(node->rhs);
      printf("  jmp .Lbegin%d\n", node->val);
      printf(".Lend%d:\n", node->val);
      return;
    case ND_FOR_INIT:
      if (node->lhs)
        gen(node->lhs);
      printf(".Lbegin%d:\n", node->val);

      // ND_FOR_COND の処理を委譲する
      gen_for_cond(node->rhs);

      printf(".Lend%d:\n", node->val);
      return;
    case ND_BLOCK:
      int i = 0;
      // stmts を取り出す
      while (node->stmts[i]) {
        gen(node->stmts[i++]);

        printf("  pop rax\n");
      }
      return;
    case ND_FUNC_CALL:
      gen_func_call(node);
      return;
  }

  gen(node->lhs);
  gen(node->rhs);

  // 奥に入っているのが第2項: rdi
  // 手間に入っているのが第1項: rax
  printf("  pop rdi\n");
  printf("  pop rax\n");

  switch (node->kind) {
    case ND_ADD:
      printf("  add rax, rdi\n");
      break;
    case ND_SUB:
      printf("  sub rax, rdi\n");
      break;
    case ND_MUL:
      printf("  imul rax, rdi\n");
      break;
    case ND_DIV:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      break;
    case ND_MOD:
      printf("  cqo\n");
      printf("  idiv rdi\n");
      printf("  mov rax, rdx\n");
      break;
    case ND_EQU:
      printf("  cmp rax, rdi\n");
      printf("  sete al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_NEQ:
      printf("  cmp rax, rdi\n");
      printf("  setne al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LES:
      printf("  cmp rax, rdi\n");
      printf("  setl al\n");
      printf("  movzb rax, al\n");
      break;
    case ND_LEQ:
      printf("  cmp rax, rdi\n");
      printf("  setle al\n");
      printf("  movzb rax, al\n");
      break;
  }

  printf("  push rax\n");
}

void gen_func(Node *node) {
  if (node->kind != ND_FUNC)
    error("ND_FUNC expected but found %d\n", node->kind);
  
  printf("%s:\n", node->name);

  // プロローグ
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, %d\n", node->offset);

  // 関数の引数のレジスタの値を変数に保存
  int arg_idx = 0;
  Node *arg;
  while (arg = node->args[arg_idx]) {
    gen_lval(arg);
    printf("  pop rax\n");
    printf("  mov [rax], %s\n", ARG_RGST[arg_idx++]);
  }

  Node *stmt;
  for (int i = 0; stmt = node->stmts[i]; i++) {
    gen(stmt);

    printf("  pop rax\n");
  }

  // エピローグ
  // 変数で確保しておいたRSPをRBPと同じ場所に戻す
  printf("  mov rsp, rbp\n");
  // popでRSPがリターンアドレスを指し、RBPを関数呼び出し前のRBPに戻す
  printf("  pop rbp\n");
  printf("  ret\n");
}

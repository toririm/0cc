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

void gen_func(Node *node) {
  /*
    x86-64のABIのため
    rspが16の倍数になるように調整
  */

  // 引数の数, 第3引数, 第1引数として使われるRAX, RDX, RDIを一旦退避
  printf("  push rax\n");
  printf("  push rdx\n");
  printf("  push rdi\n");

  // RSP % 16
  printf("  mov rdi, 16\n");
  printf("  mov rax, rsp\n");
  printf("  cqo\n");
  printf("  idiv rdi\n");
  // 商->RAX, 余り->RDX

  // if (RSP % 16)
  printf("  cmp rdx, 0\n");
  printf("  je .Lelse%d\n", node->val);
  // スタックに退避していたRAX, RDX, RDIを戻す
  printf("  pop rdi\n");
  printf("  pop rdx\n");
  printf("  pop rax\n");
  // popやpushは8byte単位で動かすのでズレている場合は決めうちで8下げる
  // RSP -= 8;
  printf("  sub rsp, 8\n");
  printf("  call %s\n", node->func_name);
  printf("  add rsp, 8\n");
  printf("  push rax\n");
  printf("  jmp .Lend%d\n", node->val);

  // else
  printf(".Lelse%d:\n", node->val);
  // スタックに退避していたRAX, RDX, RDIを戻す
  printf("  pop rdi\n");
  printf("  pop rdx\n");
  printf("  pop rax\n");
  printf("  call %s\n", node->func_name);
  printf("  push rax\n");

  // end if
  printf(".Lend%d:\n", node->val);
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
      while (node->nodes[i]) {
        gen(node->nodes[i++]);

        printf("  pop rax\n");
      }
      return;
    case ND_FUNC_CALL:
      gen_func(node);
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

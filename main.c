#include "0cc.h"

char *user_input;
Token *token;
Node *code[100];
LVar *locals;

int main(int argc, char **argv) {
  if (argc != 2) {
    fprintf(stderr, "引数の個数が正しくありません\n");
    return 1;
  }

  user_input = argv[1];
  locals = calloc(1, sizeof(LVar));
  token = tokenize(user_input);
  program();

  // アセンブリの前半部分を出力
  printf(".intel_syntax noprefix\n");
  printf(".globl main\n");
  printf("main:\n");

  // プロローグ
  // 26 * 8 = 208, 変数26個分の領域を確保する
  printf("  push rbp\n");
  printf("  mov rbp, rsp\n");
  printf("  sub rsp, 208\n");

  for (int i = 0; code[i]; i++) {
    gen(code[i]);

    // 式の評価結果のスタックを取り除く
    printf("  pop rax\n");
  }

  // エピローグ
  // 変数で確保しておいたRSPをRBPと同じ場所に戻す
  printf("  mov rsp, rbp\n");
  // popでRSPがリターンアドレスを指し、RBPを関数呼び出し前のRBPに戻す
  printf("  pop rbp\n");
  printf("  ret\n");
  return 0;
}

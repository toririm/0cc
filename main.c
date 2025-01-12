#include "0cc.h"

char *ARG_RGST[] = { "rdi", "rsi", "rdx", "rcx", "r8", "r9" };

char *user_input;
Token *token;
Node *code[100];
LVar *locals;
int label_index = 0;

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

  printf(".globl ");
  for (int i = 0; code[i]; i++) {
    if (i > 0) printf(", ");
    printf("%s", code[i]->name);
  }
  printf("\n");

  for (int i = 0; code[i]; i++) {
    gen_func(code[i]);
  }

  return 0;
}

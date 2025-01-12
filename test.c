#include <stdio.h>

int foo() {
    printf("foo() called!\n");
}

int bar() {
    printf("bar() called!\n");
    return 127;
}

int plus(int x, int y) {
    printf("plus called! %d + %d = %d\n", x, y, x + y);
    return x + y;
}

int plus6(int a, int b, int c, int d, int e, int f) {
    int sum = a + b + c + d + e + f;
    printf("plus6 called! %d+%d+%d+%d+%d+%d=%d\n", a, b, c, d, e, f, sum);
    return sum;
}

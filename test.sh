#!/bin/bash
assert() {
  expected="$1"
  input="$2"

  ./0cc "$input" > tmp.s
  cc -o tmp tmp.s
  ./tmp
  actual="$?"

  if [ "$actual" = "$expected" ]; then
    echo "$input => $actual"
  else
    echo "$input => $expected expected, but got $actual"
    exit 1
  fi
}

assert 0 "main(){0;}"
assert 42 "main () {   42;}"
assert 21 "main( ) {5+20-4;}"
assert 41 "main(){ 12 + 34 - 5 ; }"
assert 47 "main(){5+6*7;}"
assert 15 "main(){5*(9-6);}"
assert 4 "main(){ (3+5)/2; }"
assert 15 "main(){ -3*-5; }"
assert 1 "main(){ -(2+3)/-5; }"
assert 10 "main(){ - - - (-10); }"
assert 2 "main(){+ + 2;}"
assert 1 "main(){+ - 1 + 2;}"
assert 1 "main(){0 < 1;}"
assert 1 "main(){3*5+1==4*4*(2 - 1 * 2 >= 0);}"
assert 1 "main(){1 <= 2;}"
assert 1 "main(){ 1<=1; }"
assert 1 "main(){0!=1;}"
assert 6 "main(){a=3; b = 2; a * b;}"
assert 100 "main(){ z = 99; z + 1;}"
assert 9 "main(){foo = 7; bar=2; fizz=foo+bar;fizz;}"
assert 5 "main(){return 5;}"
assert 6 "main(){foo=2; bar = 3; return foo * bar; return 3;}"
assert 1 "main(){if (1 + 3 == 4) 3 - 2;}"
assert 0 "main(){if (35 < 1) return 1; else return 0;}"
assert 2 "main(){0; if (0 == 0) return 2;}"
assert 3 "main(){if (0) return 1; if (2) return 3; return 4;}"
assert 3 "main(){if (0) return 1; else if (2) return 3; else return 4;}"
assert 1 "main(){if (4 != 4) return 3; else if (2) return 1; else return 0;}"
assert 5 "main(){i = 0; while (i < 5) i = i + 1;}"
assert 5 "main(){for (i = 0; i < 5; i = i + 1) i;}"
assert 5 "main(){i = 0; for (;i<5;i=i+1) i;}"
assert 5 "main(){i = 0; for (;i<5;) i = i + 1;}"
# assert 5 "main(){i = 0; for (;;) {if (i<5) {i=i+1;} else return i;} 0;}"
assert 45 "main(){sum = 0; for (i=0;i<10;i=i+1) {sum=sum+i;} return sum;}"
assert 128 "foo(){0;} bar(){127;} main(){foo(); bar() + 1;}"
assert 129 "foo(){0;} bar(){127;} main(){foo(); 1 + bar() + 1;}"
assert 1 "main(){5 % 4;}"
assert 1 "main(){a=6; if (a%3==0) {if(a%5==0)return 0;else return 1;} else return 2;}"
assert 0 "main(){a=15; if (a%3==0) {if(a%5==0)return 0;else return 1;} else return 2;}"
assert 2 "main(){a=7; if (a%3==0) {if(a%5==0)return 0;else return 1;} else return 2;}"
assert 5 "plus(x, y){return x + y;} main(){plus(1 + 1, 6 / 2);}"
assert 4 "plus(x, y){return x + y;} main(){plus(plus(plus(1, 1), 1), 1);}"
assert 21 "plus6(a,b,c,d,e,f){return a+b+c+d+e+f;} main(){plus6(1, 2, 3, 4, 5, 6);}"
assert 1 "plus(x, y){return x + y;} main(){plus(1, 0);}"
assert 24 "plus(x, y){return x + y;} plus6(a,b,c,d,e,f){return a+b+c+d+e+f;} main(){plus6(plus(1, 0), plus(2, 3), 3, 4, 5, 6);}"
assert 4 "foo(x, y){return 2 * x + y;} main(){foo(1, 2);}"
assert 10 "double(x){return 2 * x;} main(){double(5);}"
# assert 6 "loop(n){if(n==0)return 0; return n + loop(n-1);} main(){loop(3);}"

echo OK

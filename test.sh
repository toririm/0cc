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

assert 0 "0;"
assert 42 "42;"
assert 21 "5+20-4;"
assert 41 " 12 + 34 - 5 ;"
assert 47 "5+6*7;"
assert 15 "5*(9-6);"
assert 4 "(3+5)/2;"
assert 15 "-3*-5;"
assert 1 "-(2+3)/-5;"
assert 10 " - - - (-10);"
assert 2 "+ + 2;"
assert 1 "+ - 1 + 2;"
assert 1 "0 < 1;"
assert 1 "3*5+1==4*4*(2 - 1 * 2 >= 0);"
assert 1 "1 <= 2;"
assert 1 " 1<=1;"
assert 1 "0!=1;"
assert 6 "a=3; b = 2; a * b;"
assert 100 " z = 99; z + 1;"
assert 9 "foo = 7; bar=2; fizz=foo+bar;fizz;"
assert 5 "return 5;"
assert 6 "foo=2; bar = 3; return foo * bar; return 3;"

echo OK

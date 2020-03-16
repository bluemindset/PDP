
#include <stdio.h>

#define func(x) _Generic((x), int: func_int, char: func_char)(x);

void func_int (int x)
{
  printf("%s\t%d\n", __func__, x);
}

void func_char (char x)
{
  printf("%s\t%c\n", __func__, x);
}

int main(void)
{
  int i = 5;
  char c = 'A';

  func(i);
  func(c);
}
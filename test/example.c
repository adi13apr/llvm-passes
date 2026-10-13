int add(int a, int b) { return a + b; }

int main(void) {
  int x = add(2, 3);
  if (x > 4)
    x = x * 2;
  return x;
}
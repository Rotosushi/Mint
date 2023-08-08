
typedef int(*Fn)(int,int);

int f(int a, int b) {
  return a + b;
}

Fn add = f;

int main() {
  return add(1, 2);
}
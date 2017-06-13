/*#include <stdint.h>
#include <string.h>

#define MOZ_INIT_OUTSIDE_CTOR \
  __attribute__((annotate("moz_ignore_ctor_initialization")))

#define MOZ_IS_CLASS_INIT \
  __attribute__((annotate("moz_is_class_init")))


// Should be ignored since it doesn't have CTOR
struct Obj {
  ~Obj() {};
};

// Just the declaration
int do_something_with_obj(const Obj& o) {
};

//Function with declaration and definition
void InitByRef(int& var) {
  var = 2;
}

// This should be skipped since there is no CTOR
struct TestStruct {
  int a;
  int b;
};

// This should trigger an unintialized error message
struct TestStruct2 {
  int b;
  TestStruct2() { // expected-error {{'b' is not correctly initialized in the constructor of 'TestStruct2'}}
  }
};

class Test {
  int a;
  int b;
  int c;
  int d = 2;
  int *e;
  int *f;
  int g;
  int h;
  int x;
  MOZ_INIT_OUTSIDE_CTOR int z; // should be ignored
  TestStruct2 zt; // will be ignored since it's not a primitive

  void InitPtrByPtr(int **var);
  void InitPtr(int *&var);
  void InitByMemset(int *var);
  void InitByMemset(int& var);
  int Init(int *var1, int& var2, int& var3, int*& var4, int **var5, int& var6);
  Test();
};

void Test::InitPtrByPtr(int **var) {
  *var = 0;
}

void Test::InitPtr(int *&var) {
  var = 0;
}

void Test::InitByMemset(int *var) {
  memset(var, 0, sizeof(int));
}

void Test::InitByMemset(int& var) {
  memset(&var, 0, sizeof(var));
}

int Test::Init(int *var1, int& var2, int& var3, int*& var4, int **var5, int& var6) {
  *var1 = 2;
  InitByRef(var2);
  InitByMemset(var3);
  InitPtr(var4);
  InitPtrByPtr(var5);
  InitByMemset(&var6);
  return 3;
}

Test::Test() : d(2) { // expected-error {{'x' is not correctly initialized in the constructor of 'Test'}}
  h = Init(&a, b, c, e, &f, g);

  // Test to see if we ignore default generated constructor
  TestStruct p;
}

struct Test2 {
  int h;
  int f;
  Test2();
};

Test2::Test2() : f(2) {
  h = do_something_with_obj(Obj());
}

class Test3 {
  int a = 2;
  int b;
  void Init(int& var);
  Test3();
};

void Test3::Init(int &var) {
  InitByRef(var);
}

Test3::Test3() {
  Init(b);
}

class TestIf {
  int a;
  TestIf();
};

TestIf::TestIf() {
  int p = 1;
  if (p > 2) {
    a = 0;
  } else {
    a = 1;
  }
}

class TestFor {
  int a;
  TestFor();
};

TestFor::TestFor() { // expected-error {{'a' is not correctly initialized in the constructor of 'TestFor'}}
  for (int i = 0; i < 10; i++) {
    if (!i)
      a = 0;
  }
}

class TestWhile {
  int a;
  TestWhile();
};

TestWhile::TestWhile() {
  int i = 1;
  while (i < 10) {
    if (i % 5)
      a = 0;
    else
      a = 1;
    i++;
  }
}

class TestDoWhile {
  int a;
  TestDoWhile();
};

TestDoWhile::TestDoWhile() {
  int i = 0;
  do {
    if (!i)
      a = 0;
     else
      a = 1;
    i = 1;
  } while(0);
}

class TestSwitchOk {
  int a;
  TestSwitchOk();
};

TestSwitchOk::TestSwitchOk() {
  int i = 0;
  switch (i) {
    case 0:
      a = 0;
      break;

    case 1:
    case 2:
      a = 1;
      break;
  }
}

class TestSwitchErr {
  int a;
  TestSwitchErr();
};

TestSwitchErr::TestSwitchErr() { // expected-error {{'a' is not correctly initialized in the constructor of 'TestSwitchErr'}}
  int i = 0;
  switch (i) {
    case 0:
      a = 0;
      break;

    case 1:
    case 2:
      break;
  }
}

class TestTernaryOk {
  int a;
  TestTernaryOk();
};

TestTernaryOk::TestTernaryOk() {
  int i = (1 == 1) ? (a = 1) : (a = 0);
}
*/
class TestTernaryErr {
  int a;
  TestTernaryErr();
};

TestTernaryErr::TestTernaryErr() { // expected-error {{'a' is not correctly initialized in the constructor of 'TestTernaryErr'}}
  int i = (1 == 1) ? (a = 1) : 0;
}
/*
class TestTransitiveRefOk {
  int a;
  TestTransitiveRefOk();
};

TestTransitiveRefOk::TestTransitiveRefOk() {
  int &ref_to_a = a;
  ref_to_a = 0;
}

class TestTransitiveRefErr {
  int a;
  TestTransitiveRefErr();
};

TestTransitiveRefErr::TestTransitiveRefErr() { // expected-error {{'a' is not correctly initialized in the constructor of 'TestTransitiveRefErr'}}
  int &ref_to_a = a;
}

class TestInitFunc {
  int a;
  int b;
public:
  TestInitFunc() : a(0) {}
  MOZ_IS_CLASS_INIT void Init();
};

void TestInitFunc::Init() {
  b = 0;
}

class TestDelegatingCtor {
  int a, b;
  TestDelegatingCtor(int a1, int b1) : a(a1), b(b1) {}

  explicit TestDelegatingCtor(int a1) : TestDelegatingCtor(a1, 0) {}
};

class TestBogusDelegatingCtor {
  int a, b;
  TestBogusDelegatingCtor(int a1, int b1) // expected-error {{'b' is not correctly initialized in the constructor of 'TestBogusDelegatingCtor'}}
      : a(a1) {}

  explicit TestBogusDelegatingCtor(int a1) : TestBogusDelegatingCtor(a1, 0) {}
};*/
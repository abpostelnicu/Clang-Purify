#include <stddef.h>

static __attribute__((always_inline)) bool AssertAssignmentTest(bool expr) {
  return expr;
}

#define UNLIKELY(x) (__builtin_expect(!!(x), 0))
#define CRASH() do { } while(0)
#define CHECK_ASSIGNMENT(expr) !AssertAssignmentTest(!(expr))
#define ASSERT(expr) \
do { \
  if (UNLIKELY(!CHECK_ASSIGNMENT(expr))) { \
    CRASH();\
  } \
}	while(0) \


class TestOverloading {
public:
  int value;
  // different operators
  explicit operator bool() const { return (bool)value; }
  TestOverloading& operator=(const int _val) { value = _val; return *this; }
};

void TestOverloadingFunc() {
  TestOverloading p;
  p.value = 2;
  
  ASSERT(p);
  
  ASSERT(p = 3);
  
  p = 3;
}

void TestSimple() {
  int p = 2;
  
  ASSERT(p = 2);
}

class TestOne {
	int var1 = 0;	// initialized inline
	int var2;     // initialized in constructor initialization list
	int var3;     // initialized by return value
	int var4;     // initialized in if/else statement
	int var5;     // initialized only in if-then -> warning message triggered
	int var6;     // initialized in InitVariableWithCond

	int Initialize();
	void InitVariable(int &var);
	void InitVariableWithCond(int &var);

public:
	TestOne() : var2(0) {
		var3 = Initialize();
	}
};

int TestOne::Initialize() {
	bool something = true;
	if (something) {
		var4 = 0;
		InitVariable(var5);
	} else {
		var4 = 1;
	}

	InitVariableWithCond(var6);
	return 0;
}

void TestOne::InitVariable(int &var) {
	var = 2;
}

void TestOne::InitVariableWithCond(int &var) {
	bool something = true;

	if (something)
		var = 0;
	else
		var = 2;
}

class NewOverload {
  int a;
  void *operator new(size_t size) {
    return (void*)0xFF;
  }
};

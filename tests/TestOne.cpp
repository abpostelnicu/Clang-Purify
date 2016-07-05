#include <stddef.h>

static inline void AssertImpl(bool expr) {
  
}

#define ASSERT_CHECK_ATTRIB(expr) AssertImpl(!(expr))

#define ASSERT(expr) \
  ASSERT_CHECK_ATTRIB(expr) \


void TestAssertAttrib() {
  int p;
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

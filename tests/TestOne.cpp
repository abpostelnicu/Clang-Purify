class TestOne {
	int var1 = 0;	// initialized inline
	int var2;		// initialized in constructor initialization list
	int var3;		// initialized by return value
	int var4;		// initialized in if/else statement	
	int var5;		// initialized only in if-then -> warning message triggered
	int var6;		// initialized in InitVariableWithCond

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

class Base {
public:
  virtual void fo() {
  }
};

class BaseOne : public Base {
public:
  virtual void fo() {
    
  }
};

class BaseSecond : public Base {
public:
  virtual void fo() {
    
  }
};

class Deriv : public BaseOne, public BaseSecond {
public:
  int func() {
    return 0;
  }
  void fo() {
    func();
    BaseSecond::fo();
    BaseOne::fo();
  }
};

class DerivSimple : public Base {
public:
  void fo() {
    
  }
};

class BaseVirtualOne : public virtual Base {
  
};

class BaseVirtualSecond: public virtual Base {
  
};

class DerivVirtual : public BaseVirtualOne, public BaseVirtualSecond {
public:
  void fo() {
    BaseVirtualSecond::fo();
  }
};

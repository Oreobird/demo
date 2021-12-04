#include <tvm/runtime/packed_func.h>

void MyAdd(TVMArgs args, TVMRetValue* rv) {
  // automatically convert arguments to desired type.
  int a = args[0];
  int b = args[1];
  // automatically assign value return to rv
  *rv = a + b;
}

void CallPacked() {
  PackedFunc myadd = PackedFunc(MyAdd);
  // get back 3
  int c = myadd(1, 2);
}


// register a global packed function in c++
TVM_REGISTER_GLOBAL("myadd")
.set_body(MyAdd);

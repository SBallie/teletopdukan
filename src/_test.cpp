// TODO: probably should leave c++ to compiling for the full app target??
// - though if we could use lambdas and a few other things that may be interesting?
// - but it may just make the hardware interface more opaque
// TODO: Possibly have a common system.h and then systemc.h and
#include <system.h>
#include <systemcpp.h>

int addi(int a, int b) { return 42; }
long addl(long a, long b) { return 52; }
long long addll(long long a, long long b) { return 62; }

// call from C++ with p
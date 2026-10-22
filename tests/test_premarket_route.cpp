#include "../src/symbol_codes.h"
#include <cassert>
int main(){
 assert(premarketRequestCode("usBRK.B","BRK.B.N")=="usBRK.B.N");
 assert(premarketRequestCode("usBRK.A","BRK.A.N")=="usBRK.A.N");
 assert(premarketRequestCode("usBRK.B","")=="usBRK.B");
 assert(premarketRequestCode("usBRK.B","BRK.A.N")=="usBRK.B");
 assert(premarketRequestCode("usAAPL","AAPL.OQ")=="usAAPL");
 assert(premarketRequestCode("usBMNU","BMNU.AM")=="usBMNU");
 assert(canonicalWatchSymbol("usBRK.B.N")=="usBRK.B");
 assert(premarketRequestCode("hk00700","00700")=="hk00700");
}

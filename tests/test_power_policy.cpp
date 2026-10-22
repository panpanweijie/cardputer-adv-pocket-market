#include "power_policy.h"
#include <cassert>
int main(){assert(backgroundRefreshMs(5000)==60000);assert(backgroundRefreshMs(300000)==300000);assert(idleYieldMs(false,false,false)==10);assert(idleYieldMs(false,true,false)==2);assert(idleYieldMs(true,false,false)==20);assert(idleYieldMs(true,false,true)==2);}

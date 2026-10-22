#include "boundary_feedback.h"
#include <cassert>
#include <iostream>
int main(){BoundaryAttemptLatch l;assert(!l.update(-1,false));assert(l.update(-1,true));for(int i=0;i<100;++i)assert(!l.update(-1,true));assert(!l.update(0,true));assert(l.update(-1,true));assert(l.update(1,true));l.reset();assert(l.update(1,true));assert(!l.update(1,false));assert(l.update(1,true));BoundaryPulse p;assert(!p.active(0));p.trigger(1,100);assert(p.active(100)&&p.offset(100)==0);assert(p.offset(210)==3);assert(!p.active(320)&&p.offset(320)==0);p.trigger(-1,400);assert(p.offset(510)==-3);p.reset();assert(!p.active(520));p.trigger(1,0xfffffff0u);assert(p.active(20));std::cout<<"PASS one feedback per held boundary attempt, rearm, reversal, 220ms pulse, 3px bounce, reset and rollover\n";}

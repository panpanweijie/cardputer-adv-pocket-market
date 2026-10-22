#include "motion_deck.h"
#include <cassert>
#include <iostream>
int main(){MotionDeck d;uint32_t t=1000;auto step=[&](float deg,float gyro=0){float r=deg/57.2957795f;d.update(-std::sin(r),std::cos(r),0,0,gyro,0,t+=20,22);};
d.reset(10,t);step(0);for(int i=0;i<40;++i)step(18,8);float right=d.position;assert(right>10&&!d.uprightRest);
for(int i=0;i<12;++i)step(0,-8);for(int i=0;i<40;++i)step(-18,-8);assert(d.position<right-.3f&&!d.uprightRest);
// Pausing for 200ms during reversal must not freeze or change neutral.
d.reset(10,t);step(0);for(int i=0;i<40;++i)step(12,6);float neutral=d.ny;for(int i=0;i<10;++i)step(4);assert(!d.uprightRest&&d.ny==neutral);float before=d.position;for(int i=0;i<40;++i)step(-12,-6);assert(d.position<before-.2f);
// Standing locks, but small movement wakes quickly and must not immediately re-lock.
d.reset(7,t);step(0);for(int i=0;i<60;++i)step(0,.8f);assert(d.uprightRest);for(int i=0;i<5;++i)step(3,4);assert(!d.uprightRest);for(int i=0;i<20;++i)step(-5,-4);assert(!d.uprightRest);float start=d.position;for(int i=0;i<20;++i)step(-10,-4);assert(d.position<start);
std::cout<<"PASS handheld left/right reversal, short pause preserves neutral, small movement unlock and cooldown\n";}

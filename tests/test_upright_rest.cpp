#include "motion_deck.h"
#include <cassert>
#include <iostream>
int main(){MotionDeck d;uint32_t t=1000;d.reset(7,t);d.update(0,0,1,0,0,0,t+=20,22);
// A stand after a different startup neutral: preserve first stable selected card for a minute.
for(int i=0;i<3000;++i)d.update(.01f*std::sin(i*.1f),.995f,.1f,.4f,.8f,.2f,t+=20,22);
assert(d.uprightRest&&d.selected(22)==7&&std::fabs(d.position-7)<.01f&&d.velocity==0);
d.nudge(1,22);for(int i=0;i<100;++i)d.update(0,.995f,.1f,0,.8f,0,t+=20,22);assert(d.selected(22)==8);
for(int i=0;i<1000;++i)d.update(0,.995f,.1f,0,.8f,0,t+=20,22);assert(d.selected(22)==8&&d.velocity==0);
// A deliberate pick-up/rotation unlocks; maintaining a new tilt then controls the deck.
for(int i=0;i<12;++i)d.update(-.3f,.94f,.1f,0,18,0,t+=20,22);assert(!d.uprightRest);
// Long presses still slide while standing, release stops at selected card.
d.reset(21,t);d.update(0,1,0,0,0,0,t+=20,22);for(int i=0;i<30;++i)d.update(0,1,0,0,0,0,t+=20,22);d.keyboardHeld=true;d.keyboardSpeed=4;
for(int i=0;i<60;++i)d.update(0,1,0,0,0,0,t+=20,22);assert(d.position>23);d.endKeyboardSlide();for(int i=0;i<100;++i)d.update(0,1,0,0,0,0,t+=20,22);float stopped=d.position;
for(int i=0;i<300;++i)d.update(0,1,0,0,.8f,0,t+=20,22);assert(std::fabs(d.position-stopped)<.01f);
std::cout<<"PASS standing static hold, stale neutral, noise, key navigation, deliberate unlock, cyclic hold and release\n";}

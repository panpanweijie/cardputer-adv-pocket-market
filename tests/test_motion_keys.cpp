#include <cassert>
#include <vector>
#include <cmath>
#include "motion_keys.h"
#include "motion_deck.h"
struct Keys{bool left=false,right=false;std::vector<char>word;};
int main(){
 Keys k;assert(motionKeyDirection(k)==0);k.word={','};assert(motionKeyDirection(k)==-1);k.word={'/'};assert(motionKeyDirection(k)==1);k.left=true;assert(motionKeyDirection(k)==0);
 MotionKeyHold h;assert(h.update(1,100).tap==1);assert(h.update(1,449).speed==0);assert(h.update(1,450).speed==4);assert(h.update(1,1650).speed==8);
 auto released=h.update(0,1670);assert(released.endedSlide&&released.speed==0);assert(!h.update(0,1700).endedSlide);
 assert(h.update(-1,1800).tap==-1);assert(h.update(-1,2150).speed==-4);auto reversed=h.update(1,2170);assert(reversed.endedSlide&&reversed.tap==1&&reversed.speed==0);
 h.reset();assert(h.update(1,UINT32_MAX-100).tap==1);assert(h.update(1,249).speed==4); // millis wrap
 MotionDeck d;h.reset();uint32_t t=1000;d.reset(10,t);d.update(0,0,1,0,0,0,t,50);
 bool opposing=true;auto step=[&](int dir){t+=20;auto c=h.update(dir,t);if(c.endedSlide)d.endKeyboardSlide();if(c.tap){d.velocity=0;d.nudge(c.tap,50);}d.keyboardHeld=dir!=0;d.keyboardSpeed=c.speed;d.update(opposing?-.31f:0,0,opposing?.95f:1,0,0,0,t,50);};
 for(int i=0;i<100;++i)step(-1);assert(d.position<3); // keyboard wins against an opposing positive tilt
 float before=d.position;opposing=false;step(0);float target=std::round(before);assert(d.manualTarget==target&&d.keyboardSpeed==0);
 for(int i=0;i<100;++i)step(0);assert(std::fabs(d.position-target)<.06f); // no queued extra cards or inertial run-away
 d.reset(49,t);h.reset();for(int i=0;i<150;++i)step(1);assert(d.position>50&&d.selected(50)>=0&&d.selected(50)<50);float lastTarget=std::round(d.position);step(0);for(int i=0;i<100;++i)step(0);assert(std::fabs(d.position-lastTarget)<.06f); // wrap during hold, stop on release
}

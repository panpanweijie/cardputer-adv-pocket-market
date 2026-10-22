#pragma once
#include <cstdint>
#include <cmath>
class BoundaryAttemptLatch{
 int latched=0;
 public:
 void reset(){latched=0;}
 bool update(int direction,bool blocked){if(!direction||!blocked){latched=0;return false;}if(latched==direction)return false;latched=direction;return true;}
};
class BoundaryPulse{
 uint32_t started=0;bool running=false;
 public:
 int direction=0;
 void reset(){running=false;direction=0;}
 void trigger(int dir,uint32_t now){direction=dir;started=now;running=true;}
 bool active(uint32_t now)const{return running&&uint32_t(now-started)<220;}
 int offset(uint32_t now)const{return active(now)?direction*int(std::lround(3*std::sin(uint32_t(now-started)*3.14159265f/220))):0;}
};

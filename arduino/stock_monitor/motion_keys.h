#pragma once
#include <cstdint>
struct MotionKeyCommand {int tap=0;float speed=0;bool endedSlide=false;};
class MotionKeyHold {
 int direction=0;uint32_t pressed=0;bool sliding=false;
 public:
 void reset(){direction=0;pressed=0;sliding=false;}
 MotionKeyCommand update(int next,uint32_t now){
  MotionKeyCommand out;
  if(next!=direction){out.endedSlide=sliding;direction=next;pressed=now;sliding=false;out.tap=next;}
  if(!direction)return out;
  uint32_t held=now-pressed;
  if(held>=350){sliding=true;float ramp=(held-350)/1200.f;if(ramp>1)ramp=1;out.speed=direction*(4+4*ramp);}
  return out;
 }
};
template<class Keys>int motionKeyDirection(const Keys&keys){
 bool left=keys.left,right=keys.right;
 for(char c:keys.word){left=left||c==','||c==';';right=right||c=='/'||c=='.';}
 return left==right?0:left?-1:1;
}

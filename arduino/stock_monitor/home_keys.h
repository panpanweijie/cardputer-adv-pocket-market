#pragma once
#include <cstdint>
class HomeKeyRepeat {
 int direction=0;uint32_t pressed=0,last=0;bool repeating=false;
 public:
 void reset(){direction=0;pressed=last=0;repeating=false;}
 int update(int next,uint32_t now){
  if(next!=direction){direction=next;pressed=last=now;repeating=false;return next;}
  if(!direction)return 0;
  uint32_t held=now-pressed;if(held<350)return 0;
  uint32_t interval=held<1500?120:65;
  if(!repeating||now-last>=interval){last=now;repeating=true;return direction;}
  return 0;
 }
};
template<class Keys>int homeKeyDirection(const Keys&keys){bool up=keys.up,down=keys.down;for(char c:keys.word){up=up||c==';'||c==',';down=down||c=='.'||c=='/';}return up==down?0:up?-1:1;}

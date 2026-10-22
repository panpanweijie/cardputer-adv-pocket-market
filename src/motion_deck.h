#pragma once
#include "cyclic_navigation.h"
#include <algorithm>
#include <cmath>
#include <cstdint>
// Original implementation: instant neutral seed, angle-to-speed control and spring snap.
class MotionDeck {
 public:
 bool ready=false;int axis=2;float position=0,velocity=0,angle=0;
 float nx=0,ny=0,fx=0,fy=0,manualTarget=0;bool manual=false,tableRest=false;
 bool uprightRest=false;
 float restAx=0,restAy=0,restAz=1,restTarget=0;uint32_t restSince=0,releaseSince=0,lastRestRelease=0;bool restReleased=false;
 bool keyboardHeld=false;float keyboardSpeed=0;
 uint32_t last=0,neutralSince=0;
 static float wrap(float x){while(x>180)x-=360;while(x< -180)x+=360;return x;}
 static float clamp(float x,float a,float b){return std::max(a,std::min(b,x));}
 void reset(float selected,uint32_t now){ready=false;axis=2;position=selected;velocity=angle=0;manual=false;tableRest=false;uprightRest=false;restSince=releaseSince=0;lastRestRelease=0;restReleased=false;restTarget=selected;keyboardHeld=false;keyboardSpeed=0;last=now;neutralSince=0;}
 int selected(int count)const{return cyclicIndex((int)std::floor(position+.5f),count);}
 void nudge(int dir,int count){if(count<=0)return;manualTarget=count>1?(manual?manualTarget:std::round(position))+dir:0;manual=true;}
 void endKeyboardSlide(){keyboardHeld=false;keyboardSpeed=0;velocity=0;manualTarget=std::round(position);manual=true;}
 static float speed(float tilt){
  float a=std::fabs(tilt);if(a<=3)return 0;
  const float angles[]={3,8,16,25,38},speeds[]={0,.6f,1.7f,3.2f,5};float out=5;
  for(int i=0;i<4;++i)if(a<angles[i+1]){float t=(a-angles[i])/(angles[i+1]-angles[i]);t=t*t*(3-2*t);out=speeds[i]+t*(speeds[i+1]-speeds[i]);break;}
  return tilt>0?out:-out;
 }
 bool update(float ax,float ay,float az,float gx,float gy,float gz,uint32_t now,int count){
  if(count<=1){position=velocity=0;last=now;return false;}
  if(!std::isfinite(ax)||!std::isfinite(ay)||!std::isfinite(az)||!std::isfinite(gx)||!std::isfinite(gy)||!std::isfinite(gz))return false;
  float norm=std::sqrt(ax*ax+ay*ay+az*az);
  if(norm<.75f||norm>1.25f||std::max(std::fabs(gx),std::max(std::fabs(gy),std::fabs(gz)))>250){last=now;velocity=0;return false;}
  constexpr float deg=57.2957795f;
  float x=std::atan2(ay,az)*deg,y=std::atan2(-ax,std::sqrt(ay*ay+az*az))*deg;
  if(!ready){nx=fx=x;ny=fy=y;ready=true;last=now;return false;}
  uint32_t elapsed=now-last;last=now;
  if(elapsed>200){fx=x;fy=y;velocity=0;neutralSince=now;return false;}
  float dt=std::min(.05f,elapsed*.001f);if(dt<=0)return false;
  float blend=1-std::exp(-dt/.085f);
  fx=wrap(fx+gx*dt+blend*wrap(x-fx-gx*dt));fy=wrap(fy+gy*dt+blend*wrap(y-fy-gy*dt));
  float dx=wrap(fx-nx),dy=wrap(fy-ny);
  // Fixed lateral tilt: front/back rotation never selects the navigation axis.
  angle=dy;
  // Absolute near-horizontal gravity rejects a stale handheld neutral when laid on a desk.
  bool flat=std::fabs(x)<12&&std::fabs(y)<6&&az>0&&std::max(std::fabs(gx),std::max(std::fabs(gy),std::fabs(gz)))<3;
  if(flat){if(!tableRest)velocity=0;ny=fy;angle=0;}
  tableRest=flat;
  // Vertical stands need their own gravity-based hold: a stale neutral can otherwise drift.
  float gyro=std::max(std::fabs(gx),std::max(std::fabs(gy),std::fabs(gz)));
  float ux=ax/norm,uy=ay/norm,uz=az/norm;
  bool vertical=std::fabs(uz)<.5f;
  auto gravityDifference=[&](){return std::acos(clamp(ux*restAx+uy*restAy+uz*restAz,-1,1))*deg;};
  if(uprightRest){
   bool moved=!vertical||gravityDifference()>1.2f||gyro>3.5f;
   if(moved){if(!releaseSince)releaseSince=now;}
   else releaseSince=0;
   if(releaseSince&&now-releaseSince>=20){uprightRest=false;restSince=releaseSince=0;lastRestRelease=now;restReleased=true;velocity=0;manual=false;}
  }else if(vertical&&std::fabs(ux)<.15f&&gyro<1.5f&&(!restReleased||uint32_t(now-lastRestRelease)>=1200)){
   if(!restSince||gravityDifference()>2){restSince=now;restAx=ux;restAy=uy;restAz=uz;restTarget=std::round(position);}
   if(now-restSince>=600){uprightRest=true;releaseSince=0;}
  }else restSince=0;
  bool verticalHold=uprightRest; // A candidate must not consume handheld input or rewrite neutral.
  if(verticalHold){
   ny=fy;angle=0;neutralSince=now;
   if(!keyboardHeld&&!manual){float before=position;position=restTarget;velocity=0;return std::fabs(position-before)>.0004f;}
   // Keyboard navigation remains available while standing. Freeze its final selected card.
   restTarget=std::round(position);
  }
  float target=keyboardHeld?keyboardSpeed:(manual||tableRest)?0:speed(angle),before=position;
  if(std::fabs(target)>.015f){manual=false;neutralSince=0;velocity+=(target-velocity)*(1-std::exp(-dt/.09f));}
  else{
   if(!neutralSince)neutralSince=now;
   if(manual||now-neutralSince>90){
    float snap=manual?manualTarget:std::round(position);
    velocity+=((snap-position)*64-velocity*16)*dt;
    if(std::fabs(snap-position)<.004f&&std::fabs(velocity)<.035f){position=snap;velocity=0;manual=false;}
   }else velocity*=std::exp(-dt/.055f);
  }
  position+=velocity*dt;
  return std::fabs(position-before)>.0004f;
 }
};
struct DeckCard {int index,x,y,w,h;float distance;};
inline int deckCards(float position,int count,DeckCard* out){
 if(count<=0)return 0;
 int middle=(int)std::floor(position+.5f),n=0;
 int span=std::min(count,5);
 for(int i=middle-span/2;i<=middle+(span-1)/2;++i){
  float d=i-position,a=std::min(2.0f,std::fabs(d));int w=(int)std::lround(168-42*a),h=(int)std::lround(82-20*a);
  int cx=(int)std::lround(120+d*100),cy=(int)std::lround(76+5*a*a);
  if(cx+w/2<0||cx-w/2>=240)continue;
  out[n++]={cyclicIndex(i,count),cx-w/2,cy-h/2,w,h,d};
 }
 std::sort(out,out+n,[](const DeckCard&a,const DeckCard&b){return std::fabs(a.distance)==std::fabs(b.distance)?a.index<b.index:std::fabs(a.distance)>std::fabs(b.distance);});return n;
}

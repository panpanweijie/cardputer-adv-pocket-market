#pragma once
#include <cmath>
#include <cstdint>
#include <algorithm>
// Complementary acceleration/gyro filter, neutral calibration and repeat limiting.
class TiltNavigation {
 public:
 bool ready=false;int axis=0;
 int samples=0;float sumX=0,sumY=0,neutralX=0,neutralY=0,filteredX=0,filteredY=0;
 uint32_t last=0,lastStep=0;
 void reset(){ready=false;axis=0;samples=0;sumX=sumY=0;last=lastStep=0;}
 static float wrap(float x){while(x>180)x-=360;while(x< -180)x+=360;return x;}
 int update(float ax,float ay,float az,float gx,float gy,float gz,uint32_t now){
  if(!std::isfinite(ax)||!std::isfinite(ay)||!std::isfinite(az)||!std::isfinite(gx)||!std::isfinite(gy)||!std::isfinite(gz))return 0;
  float norm=std::sqrt(ax*ax+ay*ay+az*az);
  if(norm<.75f||norm>1.25f){if(!ready){samples=0;sumX=sumY=0;}last=now;return 0;}
  constexpr float degrees=57.2957795f;
  float x=std::atan2(ay,az)*degrees,y=std::atan2(-ax,std::sqrt(ay*ay+az*az))*degrees;
  if(!ready){
   if(std::fabs(gx)>12||std::fabs(gy)>12||std::fabs(gz)>12){samples=0;sumX=sumY=0;return 0;}
   if(samples&&(std::fabs(wrap(x-sumX/samples))>4||std::fabs(wrap(y-sumY/samples))>4)){samples=0;sumX=sumY=0;}
   if(!samples){filteredX=x;filteredY=y;}
   sumX+=filteredX+wrap(x-filteredX);sumY+=filteredY+wrap(y-filteredY);++samples;
   if(samples>=30){neutralX=filteredX=sumX/samples;neutralY=filteredY=sumY/samples;ready=true;last=lastStep=now;}
   return 0;
  }
  uint32_t elapsed=now-last;last=now;
  if(elapsed>200){filteredX=x;filteredY=y;lastStep=now;return 0;} // No burst after network I/O.
  float dt=elapsed*.001f;
  filteredX=wrap(filteredX+gx*dt+.08f*wrap(x-filteredX-gx*dt));
  filteredY=wrap(filteredY+gy*dt+.08f*wrap(y-filteredY-gy*dt));
  float dx=wrap(filteredX-neutralX),dy=wrap(filteredY-neutralY);
  if(!axis&&std::max(std::fabs(dx),std::fabs(dy))>=12)axis=std::fabs(dx)>=std::fabs(dy)?1:2;
  float tilt=axis==1?dx:axis==2?dy:0,amount=std::fabs(tilt);
  if(amount<6){lastStep=now-700;return 0;}
  uint32_t repeat=amount>25?350:700;
  if(amount<12||now-lastStep<repeat)return 0;
  lastStep=now;return tilt>0?1:-1;
 }
};

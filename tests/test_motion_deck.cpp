#include <cassert>
#include <limits>
#include "motion_deck.h"
int main(){
 MotionDeck d;uint32_t t=1000;d.reset(2,t);assert(!d.ready);d.update(0,0,1,0,0,0,t+20,10);t+=20;assert(d.ready&&d.position==2); // no 30-sample gate
 for(int i=0;i<10;++i){t+=20;d.update(-.31,0,.95,0,0,0,t,10);}assert(d.position>2&&d.position<2.5f); // fractional movement within 200ms
 for(int i=0;i<80;++i){t+=20;d.update(-.31,0,.95,0,0,0,t,10);}assert(d.position>3);
 for(int i=0;i<130;++i){t+=20;d.update(0,0,1,0,0,0,t,10);}assert(std::fabs(d.position-std::round(d.position))<.008f&&std::fabs(d.velocity)<.04f);
 float cross=d.position;for(int i=0;i<100;++i){t+=20;d.update(0,.6,.8,0,0,0,t,10);}assert(std::fabs(d.position-cross)<.015f);
 float held=d.position;for(int i=0;i<80;++i){t+=20;d.update(0,.025,1,0,0,0,t,10);}assert(std::fabs(d.position-held)<.015f);
 float previous=d.position;for(int i=0;i<300;++i){t+=20;d.update(.6,0,.8,0,0,0,t,10);}assert(d.position<previous&&d.selected(10)>=0&&d.selected(10)<10); // continues around the ring
 d.reset(2,t);t+=20;d.update(0,0,1,0,0,0,t,5);d.nudge(1,5);
 for(int i=0;i<100;++i){t+=20;d.update(0,0,1,0,0,0,t,5);}assert(std::fabs(d.position-3)<.005f);
 d.nudge(-1,5);for(int i=0;i<25;++i){t+=20;d.update(-.31,0,.95,0,0,0,t,5);}assert(d.position<2.2f); // key nudge takes priority while tilted
 float prior=d.position;t+=2000;d.update(0,.6,.8,0,0,0,t,5);assert(d.position==prior); // no network catch-up jump
 assert(!d.update(0,0,3,0,0,0,t+20,5));assert(!d.update(std::numeric_limits<float>::quiet_NaN(),0,1,0,0,0,t+40,5));
 assert(MotionDeck::speed(2)==0&&MotionDeck::speed(-10)==-MotionDeck::speed(10));
 float last=0;for(float a=0;a<90;a+=.1f){float v=MotionDeck::speed(a);assert(v>=last-.0001f&&v<=5);last=v;}
 for(int count:{1,2,22,500})for(int k=0;k<40;++k){float p=(count-1)*k/39.f;DeckCard cards[5];int n=deckCards(p,count,cards);assert(n>0&&n<=5);for(int i=0;i<n;++i){assert(cards[i].index>=0&&cards[i].index<count&&cards[i].w>0&&cards[i].h>0);if(i)assert(std::fabs(cards[i-1].distance)>=std::fabs(cards[i].distance));}}
 DeckCard cards[5];int n=deckCards(2,5,cards);assert(cards[n-1].index==2&&cards[n-1].w==168&&cards[n-1].h==82);
}

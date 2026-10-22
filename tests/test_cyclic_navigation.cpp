#include "motion_deck.h"
#include <cassert>
#include <iostream>
int main(){assert(cyclicListOffset(0,-1,22)==19);assert(cyclicListOffset(19,1,22)==0);assert(cyclicListOffset(0,1,2)==0);assert(cyclicCrossing(21.4f,21.6f,22)==1);assert(cyclicCrossing(-.4f,-.6f,22)==-1);assert(cyclicCrossing(1,1.2,22)==0);assert(cyclicCrossing(0,1,1)==0);
MotionDeck d;uint32_t t=1000;d.reset(21,t);d.update(0,0,1,0,0,0,t,22);d.nudge(1,22);for(int i=0;i<120;++i){t+=20;d.update(0,0,1,0,0,0,t,22);}assert(d.selected(22)==0&&std::fabs(d.position-22)<.01f);d.nudge(-1,22);for(int i=0;i<120;++i){t+=20;d.update(0,0,1,0,0,0,t,22);}assert(d.selected(22)==21);
d.reset(0,t);d.update(0,0,1,0,0,0,t,22);d.nudge(-1,22);for(int i=0;i<120;++i){t+=20;d.update(0,0,1,0,0,0,t,22);}assert(d.selected(22)==21&&std::fabs(d.position+1)<.01f);
for(int count:{1,2,3,22})for(float pos:{-22.7f,-1.f,0.f,21.4f,21.6f,22.f,66.2f}){DeckCard c[5];int n=deckCards(pos,count,c);bool seen[22]={};assert(n>0&&n<=5);for(int i=0;i<n;++i){assert(c[i].index>=0&&c[i].index<count&&!seen[c[i].index]);seen[c[i].index]=true;}}
DeckCard c[5];int n=deckCards(0,22,c);bool left=false,right=false;for(int i=0;i<n;++i){if(c[i].index==21&&c[i].distance==-1)left=true;if(c[i].index==1&&c[i].distance==1)right=true;}assert(left&&right);std::cout<<"PASS list wrap, card forward/backward spring wrap, one crossing pulse, seamless neighbors, single-stock handling and unique card indices\n";}

#include <cassert>
#include <cmath>
#include "motion_deck.h"
int main(){
 MotionDeck d;unsigned t=1000;d.reset(7,t);
 // Enable while held at a 20 degree lateral angle, then put it flat on a desk.
 d.update(-.342f,0,.940f,0,0,0,t+=20,22);
 for(int i=0;i<3000;++i){float n=.003f*std::sin(i*.4f);d.update(n,.02f,1,0,.8f,0,t+=20,22);}
 assert(d.tableRest&&std::fabs(d.position-7)<.01f&&std::fabs(d.velocity)<.04f);
 d.nudge(1,22);for(int i=0;i<100;++i)d.update(0,0,1,0,.8f,0,t+=20,22);
 assert(d.selected(22)==8);
 // Picking it up and tilting laterally still moves; a steady intentional tilt remains active.
 for(int i=0;i<100;++i)d.update(-.31f,0,.95f,0,0,0,t+=20,22);
 assert(!d.tableRest&&d.position>9);
 // A stale neutral in either direction cannot drag the deck to an endpoint at rest.
 for(float seed:{-.45f,.45f}){
  d.reset(11,t);d.update(seed,0,.893f,0,0,0,t+=20,22);
  for(int i=0;i<1500;++i)d.update(0,.03f,.999f,.4f,-.6f,.2f,t+=20,22);
  assert(std::fabs(d.position-11)<.01f);
 }
}

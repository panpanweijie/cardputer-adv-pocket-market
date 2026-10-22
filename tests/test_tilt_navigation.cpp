#include <cassert>
#include <limits>
#include "tilt_navigation.h"
int main(){
 TiltNavigation nav;uint32_t now=0;
 for(int i=0;i<29;++i){now+=20;assert(nav.update(0,0,1,0,0,0,now)==0);}assert(!nav.ready);
 now+=20;assert(nav.update(0,0,1,0,0,0,now)==0);assert(nav.ready);
 for(int i=0;i<80;++i){now+=20;assert(nav.update(0,.025,1,0,0,0,now)==0);}assert(nav.axis==0);
 int count=0;
 for(int i=0;i<100;++i){now+=20;int n=nav.update(0,.5,.866,0,0,0,now);assert(n>=0);count+=n;}assert(count>=2&&count<=6&&nav.axis==1);
 for(int i=0;i<100;++i){now+=20;nav.update(0,0,1,0,0,0,now);}
 count=0;for(int i=0;i<100;++i){now+=20;int n=nav.update(0,-.5,.866,0,0,0,now);assert(n<=0);count-=n;}assert(count>=2&&count<=6);
 now+=2000;assert(nav.update(0,-.5,.866,0,0,0,now)==0); // no catch-up burst
 assert(nav.update(0,0,3,0,0,0,now+20)==0); // a shake is not tilt
 assert(nav.update(std::numeric_limits<float>::quiet_NaN(),0,1,0,0,0,now+40)==0);
 nav.reset();assert(!nav.ready&&nav.axis==0&&nav.samples==0);
 for(int i=0;i<40;++i){now+=20;nav.update(0,0,1,40,0,0,now);}assert(!nav.ready);
 for(int i=0;i<30;++i){now+=20;nav.update(0,0,1,0,0,0,now);}assert(nav.ready);
 count=0;for(int i=0;i<100;++i){now+=20;count+=nav.update(-.5,0,.866,0,0,0,now);}assert(count>0&&nav.axis==2);
}

#include "home_keys.h"
#include <cassert>
#include <string>
#include <iostream>
struct Keys{bool up=false,down=false;std::string word;};
int main(){HomeKeyRepeat r;assert(r.update(1,0)==1);assert(r.update(1,349)==0);assert(r.update(1,350)==1);assert(r.update(1,469)==0);assert(r.update(1,470)==1);assert(r.update(1,1500)==1);assert(r.update(1,1564)==0);assert(r.update(1,1565)==1);assert(r.update(0,1566)==0);assert(r.update(0,2000)==0);assert(r.update(-1,2001)==-1);assert(r.update(1,2002)==1);r.reset();assert(r.update(-1,2003)==-1);Keys k;k.up=true;assert(homeKeyDirection(k)==-1);k.down=true;assert(homeKeyDirection(k)==0);k.up=k.down=false;k.word=".";assert(homeKeyDirection(k)==1);k.word=";";assert(homeKeyDirection(k)==-1);std::cout<<"PASS home short press, accelerated hold, release, reversal, reset and up/down mappings\n";}

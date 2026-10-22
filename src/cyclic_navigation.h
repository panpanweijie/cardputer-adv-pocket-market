#pragma once
#include <cmath>
inline int cyclicIndex(int index,int count){return count>0?(index%count+count)%count:0;}
inline int cyclicListOffset(int offset,int step,int count){int pages=count>3?count-2:1;return cyclicIndex(offset+step,pages);}
inline int cyclicCrossing(float before,float after,int count){if(count<2)return 0;int a=int(std::floor((before+.5f)/count)),b=int(std::floor((after+.5f)/count));return b>a?1:b<a?-1:0;}

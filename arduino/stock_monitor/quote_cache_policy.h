#pragma once
#include <cstdint>
#include <vector>
inline bool cachedQuoteReady(bool valid,uint32_t fetched,uint32_t now,uint32_t interval){return valid&&uint32_t(now-fetched)<interval;}
inline std::vector<int> prefetchOrder(int count,int first,int end,int cursor){
 std::vector<int> out;
 auto add=[&](int i){if(i<0||i>=count||(i>=first&&i<end))return;for(int old:out)if(old==i)return;out.push_back(i);};
 for(int d=0;d<3&&count>0;++d){add((end+d)%count);add((first-1-d+count*4)%count);}
 for(int n=0;n<count;++n)add((cursor+n)%count);
 return out;
}

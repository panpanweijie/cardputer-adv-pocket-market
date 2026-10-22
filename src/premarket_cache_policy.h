#pragma once
#include <algorithm>
#include "quote_policy.h"
inline bool premarketDue(const std::string&code,const PreQuote&q,int64_t now,bool requested,uint32_t attempt,bool error,uint32_t ticks,uint32_t interval){
 if(!preMarketTime(code,now))return false;
 uint32_t wait=error?180000:std::max<uint32_t>(15000,interval);
 return !requested||!attempt||uint32_t(ticks-attempt)>=wait;
}
inline bool acceptPreload(const std::string&code,const PreQuote&q,int64_t now){return validPreMarket(code,q,now)&&q.hasPercent;}

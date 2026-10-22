#pragma once
#include "market_sessions.h"
struct MarketPresence {bool sh=false,sz=false,hk=false,us=false;};
template<class Codes> MarketPresence marketsIn(const Codes& codes){
 MarketPresence p;for(const auto& code:codes){const char* c=code.c_str();if(!std::strncmp(c,"sh",2))p.sh=true;else if(!std::strncmp(c,"sz",2))p.sz=true;else if(!std::strncmp(c,"hk",2))p.hk=true;else if(!std::strncmp(c,"us",2))p.us=true;}return p;
}
inline std::string marketSummary(const MarketPresence&p,int64_t now,const MarketFeed& feed){
 std::string out;
 auto add=[&](const char* label,TradingPhase phase){if(!out.empty())out+=" | ";out+=label;out+=phaseShort(phase);};
 TradingPhase sh=sessionFor("sh000001",now,feed).phase,sz=sessionFor("sz399001",now,feed).phase;
 if(p.sh&&p.sz&&sh==sz)add("沪深",sh);
 else {if(p.sh)add("沪",sh);if(p.sz)add("深",sz);}
 if(p.hk)add("港",sessionFor("hk00700",now,feed).phase);
 if(p.us)add("美",sessionFor("usAAPL",now,feed).phase);
 return out.empty()?"添加自选后显示市场状态":out;
}

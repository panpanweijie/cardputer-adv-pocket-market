#pragma once
#include "market_sessions.h"
inline bool preMarketTime(const std::string& code,int64_t now){return marketFor(code)==StockMarket::US&&scheduledSession(code,now).phase==TradingPhase::PRE;}
inline bool validPreMarket(const std::string&code,const PreQuote&q,int64_t now){return preMarketTime(code,now)&&showPreSession(q,now)&&q.timestamp>=q.sessionStart&&q.timestamp<q.sessionEnd&&q.timestamp<=now+120;}
// Keep labels stable using the local session clock when the online state is unavailable.
inline MarketSession viewSession(const std::string&code,int64_t now,const MarketFeed&feed){
 auto s=sessionFor(code,now,feed);
 if(s.phase!=TradingPhase::UNVERIFIED&&s.phase!=TradingPhase::CLOCK)return s;
 auto local=scheduledSession(code,now);
 auto sampled=scheduledSession(code,feed.timestamp);
 const MarketSignal*sig=local.market==StockMarket::US?&feed.us:local.market==StockMarket::HK?&feed.hk:code.compare(0,2,"sh")==0?&feed.sh:&feed.sz;
 // Preserve a known same-day holiday closure even after the status snapshot ages out.
 if(feed.timestamp<=now+120&&local.date&&sampled.date==local.date&&sig->present&&!sig->open&&contains(sig->reason,"休市")&&(contains(sig->reason,"节")||contains(sig->reason,"假")||contains(sig->reason,"周末")))local.phase=TradingPhase::CLOSED;
 if(feed.timestamp<=now&&sampled.date==local.date&&sig->present&&!sig->open&&contains(sig->reason,"收盘")&&sampled.phase==TradingPhase::OPEN&&local.minute>=sampled.minute)local.phase=TradingPhase::CLOSED;
 return local;
}
inline const char* priceStage(const MarketSession&s){
 if(activePhase(s.phase))return "交易";
 if(s.phase==TradingPhase::LUNCH)return "午休";
 if(s.phase==TradingPhase::PRE)return "盘前";
 return "收盘";
}

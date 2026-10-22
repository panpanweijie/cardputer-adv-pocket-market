#include <cassert>
#include <string>
#include "quote_policy.h"
int main(){
 MarketFeed empty;int64_t night=parseCivil("2026-10-20 03:30:00");
 assert(std::string(priceStage(viewSession("usSPY",night,empty)))=="收盘");
 int64_t open=parseCivil("2026-10-20 14:00:00");assert(std::string(priceStage(viewSession("usSPY",open,empty)))=="交易");
 assert(std::string(priceStage(viewSession("usSPY",parseCivil("2026-10-20 11:00:00"),empty)))=="盘前");
 MarketFeed stale;stale.timestamp=open-3600;stale.us.present=true;stale.us.open=true;assert(viewSession("usSPY",open,stale).phase==TradingPhase::OPEN);
 // Expired pre-open snapshot must not keep the market closed after the opening boundary.
 stale.timestamp=parseCivil("2026-10-20 12:00:00");stale.us.open=false;stale.us.reason="已收盘";assert(viewSession("usSPY",open,stale).phase==TradingPhase::OPEN);
 // Real holiday / half-day closure is retained for that day, never carried into the next trading day.
 stale.us.reason="节假日休市";assert(viewSession("usSPY",open,stale).phase==TradingPhase::CLOSED);
 stale.timestamp=open;stale.us.reason="已收盘";assert(viewSession("usSPY",open+3600,stale).phase==TradingPhase::CLOSED);
 assert(viewSession("usSPY",parseCivil("2026-10-21 14:00:00"),stale).phase==TradingPhase::OPEN);
 assert(std::string(priceStage(viewSession("hk00700",parseCivil("2026-10-20 04:30:00"),empty)))=="午休");
 assert(std::string(priceStage(viewSession("sh601318",parseCivil("2026-10-20 08:00:00"),empty)))=="收盘");
 assert(std::string(priceStage(viewSession("usSPY",0,empty)))=="收盘");
}

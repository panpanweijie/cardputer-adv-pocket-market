#include <cassert>
#include <fstream>
#include <iostream>
#include "market_sessions.h"
int64_t china(const char* s){return parseCivil(s)-8*3600;}
MarketFeed feed(int64_t now,const char* states){
 time_t t=now+8*3600;tm d;gmtime_r(&t,&d);char stamp[40];strftime(stamp,sizeof(stamp),"%Y-%m-%d %H:%M:%S",&d);
 MarketFeed f;assert(decodeMarketFeed(std::string(stamp)+"|"+states,f));return f;
}
int main(int argc,char**argv){
 using P=TradingPhase;using Q=QuoteAge;
 auto now=china("2026-10-20 10:00:00");auto f=feed(now,"HK_open_交易中|SH_open_交易中|SZ_open_交易中|US_close_未开盘");
 assert(sessionFor("hk00700",now,f).phase==P::OPEN);assert(sessionFor("sh600519",now,f).phase==P::OPEN);
 assert(sessionFor("sz000001",now,f).phase==P::OPEN);assert(sessionFor("sh600519",0,f).phase==P::CLOCK);
 assert(sessionFor("hk00700",now,MarketFeed{}).phase==P::UNVERIFIED);
 assert(sessionFor("hk00700",now+601,f).phase==P::UNVERIFIED);assert(sessionFor("hk00700",now-121,f).phase==P::UNVERIFIED);
 auto boundary=china("2026-10-20 11:29:50");auto oldOpen=feed(boundary,"SH_open_交易中");assert(sessionFor("sh600519",boundary+20,oldOpen).phase==P::UNVERIFIED);
 // Independent provider signals, not one shared status for a mixed watchlist.
 auto m=feed(now,"HK_close_重阳节翌日休市|SH_open_交易中|SZ_close_已休市|US_close_未开盘|NEWHK_open_交易中");
 assert(sessionFor("hk00700",now,m).phase==P::CLOSED);assert(sessionFor("sh600519",now,m).phase==P::OPEN);assert(sessionFor("sz000001",now,m).phase==P::CLOSED);
 for(const char* at:{"2026-10-20 09:15:00","2026-10-20 09:24:59","2026-10-20 14:57:00"}){
  auto t=china(at);auto a=feed(t,"SH_open_交易中");assert(sessionFor("sh600519",t,a).phase==P::AUCTION);
 }
 auto t=china("2026-10-20 09:25:00");auto a=feed(t,"SH_close_未开盘");assert(sessionFor("sh600519",t,a).phase==P::BEFORE);
 t=china("2026-10-20 12:00:00");a=feed(t,"SH_close_中午休市|HK_close_午间休市");assert(sessionFor("sh600519",t,a).phase==P::LUNCH);assert(sessionFor("hk00700",t,a).phase==P::LUNCH);
 t=china("2026-10-20 15:10:00");a=feed(t,"SH_close_已收盘|SZ_close_已收盘|KCB_open_交易中|CYB_open_交易中");assert(sessionFor("sh688981",t,a).phase==P::POST);assert(sessionFor("sz300750",t,a).phase==P::POST);assert(sessionFor("sh600519",t,a).phase==P::CLOSED);
 // Early closes and holidays come from today's feed, without annual date tables.
 t=china("2026-12-24 14:00:00");a=feed(t,"HK_close_已收盘|SH_open_交易中");assert(sessionFor("hk00700",t,a).phase==P::CLOSED);assert(sessionFor("sh600519",t,a).phase==P::OPEN);
 t=parseCivil("2026-11-27 18:01:00");a=feed(t,"US_close_已收盘|USB_open_盘后交易");assert(sessionFor("usAAPL",t,a).phase==P::POST);
 t=parseCivil("2026-10-20 13:29:59");a=feed(t,"US_close_未开盘|USB_open_盘前交易");assert(sessionFor("usAAPL",t,a).phase==P::PRE);
 t=parseCivil("2026-10-20 13:30:00");a=feed(t,"US_open_交易中|USB_close_已收盘");assert(sessionFor("usAAPL",t,a).phase==P::OPEN);
 t=parseCivil("2030-10-21 14:00:00");a=feed(t,"US_open_交易中|HK_close_假期休市");assert(sessionFor("usAAPL",t,a).phase==P::OPEN);assert(sessionFor("hk00700",t,a).phase==P::CLOSED);
 assert(nyOffsetUtc(parseCivil("2026-03-08 06:59:59"))==-18000);assert(nyOffsetUtc(parseCivil("2026-03-08 07:00:00"))==-14400);
 assert(nyOffsetUtc(parseCivil("2026-11-01 05:59:59"))==-14400);assert(nyOffsetUtc(parseCivil("2026-11-01 06:00:00"))==-18000);
 assert(scheduledSession("usAAPL",parseCivil("2026-01-02 14:30:00")).minute==570);
 auto s=sessionFor("sh600519",now,f);assert(quoteAge(s,"20261020100000",now)==Q::OK);assert(quoteAge(s,"20261019150000",now)==Q::OLD);assert(quoteAge(s,"20261020093000",now)==Q::DELAYED);assert(quoteAge(s,"garbage",now)==Q::MISSING);
 assert(quoteAge(s,"2026/10/20 09:59:59",now)==Q::OK);assert(quoteAge(s,"20261020110000",now)==Q::MISSING);
 MarketFeed invalid=f;assert(!decodeMarketFeed("garbage",invalid));assert(invalid.timestamp==f.timestamp);assert(!decodeMarketFeed("2026-10-20 10:00:00|HK_bad_x",invalid));
 // Exercise the captured online response, including JSON filter and Chinese holiday text.
 assert(argc==2);DynamicJsonDocument doc(6144),filter(512);filter["code"]=true;filter["data"]["sh000001"]["qt"]["market"][0]=true;
 std::ifstream input(argv[1]);assert(!deserializeJson(doc,input,DeserializationOption::Filter(filter)));MarketFeed live;assert(decodeMarketFeed(doc["data"]["sh000001"]["qt"]["market"][0].as<std::string>(),live));
 assert(sessionFor("hk00700",live.timestamp,live).phase==P::CLOSED);assert(sessionFor("sh600519",live.timestamp,live).phase==P::CLOSED);assert(sessionFor("usAAPL",live.timestamp,live).phase==P::OPEN);
 std::cout<<"PASS: live snapshot, mixed markets, holiday/half-day, auctions, lunch, pre/post, future years, stale feed, DST and quote ages; filtered JSON "<<doc.memoryUsage()<<" bytes\n";
}

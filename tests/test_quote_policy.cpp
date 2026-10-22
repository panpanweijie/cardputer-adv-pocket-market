#include <cassert>
#include <fstream>
#include <iostream>
#include "quote_policy.h"
int main(int argc,char**argv){assert(argc==2);DynamicJsonDocument doc(8192),filter(512);filter["data"]["usBMNR"]["pandata"]=true;std::ifstream input(std::string(argv[1])+"/tencent-pre.json");assert(!deserializeJson(doc,input,DeserializationOption::Filter(filter)));
 auto p=doc["data"]["usBMNR"]["pandata"];auto pre=decodeTencentPre(p);assert(pre.valid);
 assert(preMarketTime("usBMNR",pre.timestamp+30));assert(validPreMarket("usBMNR",pre,pre.timestamp+30));
 assert(!validPreMarket("usBMNR",pre,pre.sessionEnd));assert(!preMarketTime("usBMNR",pre.sessionEnd));
 assert(!validPreMarket("usBMNR",pre,pre.timestamp-121));assert(!validPreMarket("usBMNR",PreQuote{},pre.timestamp));
 for(const char*code:{"hk00700","sh601318","sz300750"})assert(!preMarketTime(code,pre.timestamp)&&!validPreMarket(code,pre,pre.timestamp));
 assert(!preMarketTime("usBMNR",parseCivil("2026-10-20 03:30:00"))); // overnight: no supplemental request or price
 assert(!preMarketTime("usBMNR",parseCivil("2026-10-19 22:00:00"))); // post-market
 assert(!preMarketTime("usBMNR",parseCivil("2026-10-18 10:00:00"))); // Sunday morning
 auto s=scheduledSession("usBMNR",pre.timestamp);assert(std::string(priceStage(s))=="盘前");
 s=scheduledSession("usBMNR",pre.sessionEnd+1);assert(std::string(priceStage(s))=="交易");
 s=scheduledSession("hk00700",parseCivil("2026-10-20 04:30:00"));assert(std::string(priceStage(s))=="午休");
 p["tag"]="after";assert(!decodeTencentPre(p).valid);
 std::cout<<"PASS: US closing+pre only before open; no night/post or A/HK supplements; missing/future/pre-open bounds; lunch truthful\n";
}

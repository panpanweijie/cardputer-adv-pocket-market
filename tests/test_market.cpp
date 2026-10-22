#include <cassert>
#include <fstream>
#include <iostream>
#include <string>
#include "../src/market_data.h"
int main(int argc,char**argv) {
 assert(argc==2);std::string dir=argv[1];ChartBar bars[300];
 for(auto fixture:{"five-day","year","hk","a"}){
  std::ifstream input(dir+"/"+fixture+".json");assert(input.good());
  DynamicJsonDocument filter(2048),doc(49152);setChartFilter(filter);
  assert(!deserializeJson(doc,input,DeserializationOption::Filter(filter)));
  assert(!filter.overflowed()&&!doc.overflowed());
  bool annual=std::string(fixture)=="year";
  int count=decodeChart(doc["chart"]["result"][0],annual?CHART_1Y:CHART_5D,bars,300);
  assert(count>10);std::cout<<fixture<<": bars="<<count<<", parsed heap="<<doc.memoryUsage()<<"\n";
  for(int i=1;i<count;++i){assert(bars[i].timestamp>bars[i-1].timestamp);if(annual)assert(barYear(bars[i].timestamp)>barYear(bars[i-1].timestamp));}
 }
 DynamicJsonDocument doc(24576),filter(3072);setPreFilter(filter);
 std::ifstream input(dir+"/premarket.json");assert(!deserializeJson(doc,input,DeserializationOption::Filter(filter)));
 auto q=decodePreQuote(doc["chart"]["result"][0]);assert(q.valid);
 assert(!showPreSession(q,q.sessionStart-1));assert(showPreSession(q,q.sessionStart));
 assert(showPreSession(q,q.sessionEnd-1));assert(!showPreSession(q,q.sessionEnd));
 assert(!showPreSession(q,q.sessionEnd+3600));assert(!showPreSession(q,0));
 // Annual OHLC must aggregate actual quarters rather than relabel monthly candles.
 doc.clear();auto r=doc.to<JsonObject>();
 auto ts=r.createNestedArray("timestamp");auto quote=r["indicators"]["quote"][0];
 for(const char* key:{"open","high","low","close"})quote.createNestedArray(key);
 const int64_t times[]={1704067200,1711929600,1735689600};
 const float o[]={10,12,15},h[]={13,17,20},l[]={8,9,11},c[]={12,15,18};
 for(int i=0;i<3;++i){ts.add(times[i]);quote["open"].add(o[i]);quote["high"].add(h[i]);quote["low"].add(l[i]);quote["close"].add(c[i]);}
 int n=decodeChart(r,CHART_1Y,bars,300);assert(n==2);assert(bars[0].open==10&&bars[0].high==17&&bars[0].low==8&&bars[0].close==15);
 quote["close"][1]=nullptr;n=decodeChart(r,CHART_1Y,bars,300);assert(n==2&&bars[0].close==12);
 std::cout<<"PASS: real market fixtures; yearly OHLC aggregation; null bars; pre-open visibility; hide at exact open; clock-not-synced\n";
}

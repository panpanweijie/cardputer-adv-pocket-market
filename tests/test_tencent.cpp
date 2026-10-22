#include <cassert>
#include <fstream>
#include <iostream>
#include "../src/tencent_data.h"
int main(int argc,char**argv){assert(argc==2);std::string dir=argv[1];
 DynamicJsonDocument doc(131072),filter(2048);std::ifstream pre(dir+"/tencent-pre.json");
 filter["code"]=true;filter["data"]["usBMNR"]["pandata"]=true;
 assert(!deserializeJson(doc,pre,DeserializationOption::Filter(filter)));
 auto p=doc["data"]["usBMNR"]["pandata"];auto q=decodeTencentPre(p);
 assert(q.valid&&q.price>0&&q.hasPercent&&q.offset==-14400);
 assert(q.timestamp==parseCivil(p["time"].as<const char*>())+14400);
 assert(showPreSession(q,q.timestamp));assert(!showPreSession(q,q.sessionEnd));
 p["tag"]="after";assert(!decodeTencentPre(p).valid);p["tag"]="before";
 p["time"]="2026-03-08 04:01:00";assert(decodeTencentPre(p).offset==-14400);
 p["time"]="2026-11-01 04:01:00";assert(decodeTencentPre(p).offset==-18000);
 p["time"]="2026-10-19 10:00:00";assert(!decodeTencentPre(p).valid);
 assert(parseHttpDate("Mon, 19 Oct 2026 11:17:09 GMT")==parseCivil("2026-10-19 11:17:09"));
 assert(parseHttpDate("garbage")==0);
 ChartBar bars[300];
 const char* fixtures[]={"tencent-day","tencent-month","tencent-hk","tencent-a"};
 const char* codes[]={"usBMNR.N","usBMNR.N","hk00700","sh600519"};
 for(int j=0;j<4;++j){doc.clear();filter.clear();const char* type=j==1?"month":"day";
  filter["code"]=true;filter["data"][codes[j]][type]=true;
  std::ifstream input(dir+"/"+fixtures[j]+".json");auto err=deserializeJson(doc,input,DeserializationOption::Filter(filter));if(err)std::cerr<<fixtures[j]<<": "<<err.c_str()<<"\n";assert(!err);
  assert(!doc.overflowed()&&doc["code"].as<int>()==0);auto rows=doc["data"][codes[j]][type].as<JsonArrayConst>();
  int count=decodeTencentChart(rows,j==1?CHART_1Y:CHART_1D,bars,300);assert(count>1);
  for(int i=1;i<count;++i){assert(bars[i].timestamp>bars[i-1].timestamp);if(j==1)assert(barYear(bars[i].timestamp)>barYear(bars[i-1].timestamp));}
  std::cout<<fixtures[j]<<": "<<count<<" bars; filtered heap "<<doc.memoryUsage()<<"\n";
  if(j==0){int n=decodeTencentChart(rows,CHART_5D,bars,300);assert(n==5);assert(bars[4].timestamp==parseCivil(rows[rows.size()-1][0].as<const char*>()));}
 }
 std::cout<<"PASS: Tencent pre-market + A/HK/US charts; reject post-market; DST; open boundary; HTTP clock; five-close fallback\n";
}

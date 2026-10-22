#pragma once
#include "premarket.h"
#include <ctime>
struct ChartBar { float open, high, low, close; int64_t timestamp; };
enum ChartType { CHART_5D, CHART_1D, CHART_1W, CHART_1M, CHART_1Y, CHART_TYPE_MAX };
inline const char* chartInterval(ChartType p) { const char* v[]={"15m","1d","1wk","1mo","3mo"}; return v[(int)p]; }
inline const char* chartRange(ChartType p) { const char* v[]={"5d","1y","5y","10y","max"}; return v[(int)p]; }
inline void setChartFilter(JsonDocument& f) {
 auto r=f["chart"]["result"][0];r["timestamp"][0]=true;
 for(const char* key:{"open","high","low","close"}) r["indicators"]["quote"][0][key][0]=true;
 r["meta"]["exchangeTimezoneName"]=true;
}
inline int barYear(int64_t epoch) { time_t t=(time_t)epoch; struct tm d; gmtime_r(&t,&d);return d.tm_year+1900; }
inline int decodeChart(JsonVariantConst r,ChartType period,ChartBar* out,int capacity) {
 auto ts=r["timestamp"].as<JsonArrayConst>();auto q=r["indicators"]["quote"][0];
 int count=0;int64_t last=0;
 for(size_t i=0;i<ts.size();++i){
  if(ts[i].isNull()||q["open"][i].isNull()||q["high"][i].isNull()||q["low"][i].isNull()||q["close"][i].isNull()) continue;
  ChartBar b{q["open"][i].as<float>(),q["high"][i].as<float>(),q["low"][i].as<float>(),q["close"][i].as<float>(),ts[i].as<int64_t>()};
  if(b.timestamp<=last||!std::isfinite(b.open)||!std::isfinite(b.high)||!std::isfinite(b.low)||!std::isfinite(b.close)||b.low<=0||b.high<b.low||b.high<b.open||b.high<b.close||b.low>b.open||b.low>b.close) continue;
  last=b.timestamp;
  if(period==CHART_1Y && count && barYear(out[count-1].timestamp)==barYear(b.timestamp)) {
   ChartBar& a=out[count-1];a.high=std::fmax(a.high,b.high);a.low=std::fmin(a.low,b.low);a.close=b.close;
  } else if(count<capacity) out[count++]=b;
 }
 return count;
}
inline bool showPreSession(const PreQuote& q,int64_t now) {
 return now>1700000000 && q.valid && now>=q.sessionStart && now<q.sessionEnd;
}

#pragma once
#include "market_data.h"
#include <cstdio>
#include <cstring>
#include <string>
// Calendar conversion independent of device timezone and successful NTP startup.
inline int64_t civilDays(int y,unsigned m,unsigned d) {
 y-=m<=2;int era=(y>=0?y:y-399)/400;unsigned yoe=(unsigned)(y-era*400);
 unsigned doy=(153*(m+(m>2?-3:9))+2)/5+d-1;
 return int64_t(era)*146097+yoe*365+yoe/4-yoe/100+doy-719468;
}
inline int64_t parseCivil(const char*s) {
 int y,m,d,h=0,n=0,sec=0;if(!s||std::sscanf(s,"%d-%d-%d %d:%d:%d",&y,&m,&d,&h,&n,&sec)<3) return 0;
 if(y<2000||y>2100||m<1||m>12||d<1||d>31||h<0||h>23||n<0||n>59||sec<0||sec>60) return 0;
 return civilDays(y,m,d)*86400+h*3600+n*60+sec;
}
inline int nyOffsetForDate(int64_t local) {
 time_t t=(time_t)local;struct tm d;gmtime_r(&t,&d);int month=d.tm_mon+1,day=d.tm_mday;
 bool dst=month>3&&month<11;
 if(month==3||month==11){int dow1=(d.tm_wday-(day-1)%7+7)%7;int firstSunday=1+(7-dow1)%7;
  int transition=month==3?firstSunday+7:firstSunday;
  dst=month==3?(day>transition||(day==transition&&d.tm_hour>=3)):(day<transition||(day==transition&&d.tm_hour<2));}
 return dst?-14400:-18000;
}
inline int64_t parseHttpDate(const char*s) {
 char week[8]={},month[8]={};int day,year,h,m,sec;
 if(!s||std::sscanf(s,"%7s %d %7s %d %d:%d:%d",week,&day,month,&year,&h,&m,&sec)!=7) return 0;
 const char*months[]={"Jan","Feb","Mar","Apr","May","Jun","Jul","Aug","Sep","Oct","Nov","Dec"};int mo=0;
 for(int i=0;i<12;++i)if(std::strcmp(month,months[i])==0)mo=i+1;
 if(!mo||year<2020||year>2100||day<1||day>31) return 0;
 return civilDays(year,mo,day)*86400+h*3600+m*60+sec;
}
inline PreQuote decodeTencentPre(JsonVariantConst p) {
 PreQuote q;const char*tag=p["tag"]|"";if(std::strcmp(tag,"before")!=0)return q;
 int64_t local=parseCivil(p["time"]|"");if(!local)return q;
 time_t t=(time_t)local;struct tm d;gmtime_r(&t,&d);int minutes=d.tm_hour*60+d.tm_min;
 if(minutes<240||minutes>=570)return q;
 q.offset=nyOffsetForDate(local);q.timestamp=local-q.offset;
 int64_t midnight=local-(local%86400);q.sessionStart=midnight+4*3600-q.offset;q.sessionEnd=midnight+9*3600+30*60-q.offset;
 q.price=p["last"].as<double>();q.percent=p["pct"].as<double>();
 q.valid=std::isfinite(q.price)&&q.price>0;q.hasPercent=!p["pct"].isNull()&&std::isfinite(q.percent);
 return q;
}
inline int decodeTencentChart(JsonArrayConst rows,ChartType period,ChartBar*out,int capacity) {
 int n=0;int64_t last=0;size_t start=period==CHART_5D&&rows.size()>5?rows.size()-5:0;
 for(size_t i=start;i<rows.size();++i){auto r=rows[i];if(r.size()<5)continue;
  const char*date=r[0]|"";int64_t stamp=parseCivil(date);if(!stamp||stamp<=last)continue;
  ChartBar b{r[1].as<float>(),r[3].as<float>(),r[4].as<float>(),r[2].as<float>(),stamp};
  if(!std::isfinite(b.open)||!std::isfinite(b.high)||!std::isfinite(b.low)||!std::isfinite(b.close)||b.low<=0||b.high<b.low||b.high<b.open||b.high<b.close||b.low>b.open||b.low>b.close)continue;
  last=stamp;
  if(period==CHART_1Y&&n&&barYear(out[n-1].timestamp)==barYear(stamp)){
   auto&a=out[n-1];a.high=std::fmax(a.high,b.high);a.low=std::fmin(a.low,b.low);a.close=b.close;
  }else if(n<capacity)out[n++]=b;
 }
 return n;
}

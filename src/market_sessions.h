#pragma once
#include "tencent_data.h"
#include <string>
#include <cctype>
// The provider's current-day market feed decides whether the market is open/closed.
// Local schedules only refine auctions/lunch/pre/post. No annual holiday table.
enum class StockMarket {US,HK,CN,UNKNOWN};
enum class TradingPhase {CLOCK,UNVERIFIED,CLOSED,BEFORE,PRE,OPEN,LUNCH,AUCTION,POST};
struct MarketSession {StockMarket market=StockMarket::UNKNOWN;TradingPhase phase=TradingPhase::UNVERIFIED;int date=0,minute=0,offset=0;};
struct MarketSignal {bool present=false,open=false;std::string reason;};
struct MarketFeed {int64_t timestamp=0;MarketSignal us,hk,sh,sz,extended,star,chinext;};
inline bool decodeMarketFeed(const std::string& raw,MarketFeed& out){
 if(raw.size()>4096)return false;
 size_t first=raw.find('|');if(first==std::string::npos)return false;
 MarketFeed next;next.timestamp=parseCivil(raw.substr(0,first).c_str())-8*3600;
 if(next.timestamp<1700000000)return false;
 size_t begin=first+1;
 while(begin<raw.size()){
  size_t end=raw.find('|',begin);if(end==std::string::npos)end=raw.size();std::string item=raw.substr(begin,end-begin);begin=end+1;
  size_t a=item.find('_'),b=a==std::string::npos?a:item.find('_',a+1);if(b==std::string::npos)continue;
  auto key=item.substr(0,a),state=item.substr(a+1,b-a-1);MarketSignal* signal=nullptr;
  if(key=="US")signal=&next.us;else if(key=="HK")signal=&next.hk;else if(key=="SH")signal=&next.sh;else if(key=="SZ")signal=&next.sz;
  else if(key=="USB")signal=&next.extended;else if(key=="KCB")signal=&next.star;else if(key=="CYB")signal=&next.chinext;
  if(!signal||(state!="open"&&state!="close"))continue;
  signal->present=true;signal->open=state=="open";signal->reason=item.substr(b+1);
 }
 if(!next.us.present&&!next.hk.present&&!next.sh.present&&!next.sz.present)return false;
 out=next;return true;
}
inline StockMarket marketFor(const std::string& code){
 if(code.compare(0,2,"us")==0)return StockMarket::US;
 if(code.compare(0,2,"hk")==0)return StockMarket::HK;
 if(code.compare(0,2,"sh")==0||code.compare(0,2,"sz")==0)return StockMarket::CN;
 return StockMarket::UNKNOWN;
}
inline int nyOffsetUtc(int64_t utc){
 time_t value=utc;tm d;gmtime_r(&value,&d);int year=d.tm_year+1900;
 auto sunday=[&](int month,int nth){int dow=(civilDays(year,month,1)+4)%7;return 1+(7-dow)%7+7*(nth-1);};
 int64_t start=civilDays(year,3,sunday(3,2))*86400+7*3600;
 int64_t end=civilDays(year,11,sunday(11,1))*86400+6*3600;
 return utc>=start&&utc<end?-14400:-18000;
}
inline MarketSession scheduledSession(const std::string& code,int64_t now){
 MarketSession s;s.market=marketFor(code);
 if(now<1700000000){s.phase=TradingPhase::CLOCK;return s;}
 if(s.market==StockMarket::UNKNOWN)return s;
 s.offset=s.market==StockMarket::US?nyOffsetUtc(now):8*3600;
 time_t value=now+s.offset;tm d;gmtime_r(&value,&d);
 s.date=(d.tm_year+1900)*10000+(d.tm_mon+1)*100+d.tm_mday;s.minute=d.tm_hour*60+d.tm_min;
 if(d.tm_wday==0||d.tm_wday==6){s.phase=TradingPhase::CLOSED;return s;}
 int m=s.minute;
 if(s.market==StockMarket::US){
  int close=960,postEnd=1200;
  s.phase=m<240?TradingPhase::CLOSED:m<570?TradingPhase::PRE:m<close?TradingPhase::OPEN:m<postEnd?TradingPhase::POST:TradingPhase::CLOSED;
 }else if(s.market==StockMarket::HK){
  s.phase=m<540?TradingPhase::BEFORE:m<570?TradingPhase::AUCTION:m<720?TradingPhase::OPEN:m<780?TradingPhase::LUNCH:m<960?TradingPhase::OPEN:m<970?TradingPhase::AUCTION:TradingPhase::CLOSED;
 }else{
  s.phase=m<555?TradingPhase::BEFORE:m<565?TradingPhase::AUCTION:m<570?TradingPhase::BEFORE:m<690?TradingPhase::OPEN:m<780?TradingPhase::LUNCH:m<897?TradingPhase::OPEN:m<900?TradingPhase::AUCTION:TradingPhase::CLOSED;
 }
 return s;
}
inline bool freshMarketFeed(const MarketFeed& feed,int64_t now){return now>1700000000&&feed.timestamp>1700000000&&feed.timestamp<=now+120&&now-feed.timestamp<=600;}
inline bool contains(const std::string& s,const char* part){return s.find(part)!=std::string::npos;}
inline MarketSession sessionFor(const std::string& code,int64_t now,const MarketFeed& feed){
 MarketSession s=scheduledSession(code,now);
 if(s.phase==TradingPhase::CLOCK||s.market==StockMarket::UNKNOWN)return s;
 // Require a recent online snapshot even on weekends: an unavailable feed is explicit.
 if(!freshMarketFeed(feed,now)){s.phase=TradingPhase::UNVERIFIED;return s;}
 const MarketSignal* sig=s.market==StockMarket::US?&feed.us:s.market==StockMarket::HK?&feed.hk:code.compare(0,2,"sh")==0?&feed.sh:&feed.sz;
 if((code.compare(0,5,"sh688")==0||code.compare(0,5,"sh689")==0)&&feed.star.present)sig=&feed.star;
 if((code.compare(0,5,"sz300")==0||code.compare(0,5,"sz301")==0)&&feed.chinext.present)sig=&feed.chinext;
 if(!sig->present){s.phase=TradingPhase::UNVERIFIED;return s;}
 auto sampled=scheduledSession(code,feed.timestamp);
 bool bothActive=(s.phase==TradingPhase::OPEN||s.phase==TradingPhase::AUCTION)&&(sampled.phase==TradingPhase::OPEN||sampled.phase==TradingPhase::AUCTION);
 if(s.phase!=sampled.phase&&!bothActive){s.phase=TradingPhase::UNVERIFIED;return s;}
 const auto& reason=sig->reason;
 if(contains(reason,"午")&&(contains(reason,"休")||contains(reason,"停"))){s.phase=TradingPhase::LUNCH;return s;}
 if(contains(reason,"休市")&&(contains(reason,"节")||contains(reason,"假")||contains(reason,"周末"))){s.phase=TradingPhase::CLOSED;return s;}
 if(contains(reason,"竞价")){s.phase=TradingPhase::AUCTION;return s;}
 if(s.market==StockMarket::US&&feed.extended.present&&feed.extended.open){
  if(contains(feed.extended.reason,"盘前")){s.phase=TradingPhase::PRE;return s;}
  if(contains(feed.extended.reason,"盘后")){s.phase=TradingPhase::POST;return s;}
 }
 if(contains(reason,"盘前")&&sig->open){s.phase=TradingPhase::PRE;return s;}
 if(contains(reason,"盘后")&&sig->open){s.phase=TradingPhase::POST;return s;}
 if(sig->open){
  bool fixedPrice=code.compare(0,5,"sh688")==0||code.compare(0,5,"sh689")==0||code.compare(0,5,"sz300")==0||code.compare(0,5,"sz301")==0;
  if(s.market==StockMarket::CN&&fixedPrice&&s.minute>=905&&s.minute<930){s.phase=TradingPhase::POST;return s;}
  // A market reporting open outside the ordinary clock window remains open.
  // Do not claim a closing auction after an early close unless the feed says so.
  if(s.phase!=TradingPhase::AUCTION)s.phase=TradingPhase::OPEN;
  return s;
 }
 if(contains(reason,"收盘")||contains(reason,"休市")){s.phase=TradingPhase::CLOSED;return s;}
 if(contains(reason,"未开盘")||contains(reason,"未开市")){
  if(s.phase==TradingPhase::BEFORE||s.phase==TradingPhase::AUCTION)return s;
  s.phase=TradingPhase::UNVERIFIED;return s;
 }
 s.phase=TradingPhase::UNVERIFIED;return s;
}
inline bool activePhase(TradingPhase p){return p==TradingPhase::OPEN||p==TradingPhase::AUCTION;}
inline const char* phaseShort(TradingPhase p){
 switch(p){case TradingPhase::CLOCK:return "校时";case TradingPhase::UNVERIFIED:return "待核";case TradingPhase::CLOSED:return "休市";case TradingPhase::BEFORE:return "待开";case TradingPhase::PRE:return "盘前";case TradingPhase::OPEN:return "交易";case TradingPhase::LUNCH:return "午休";case TradingPhase::AUCTION:return "竞价";case TradingPhase::POST:return "盘后";}return "待核";
}
inline const char* phaseFull(TradingPhase p){return p==TradingPhase::OPEN?"交易中":phaseShort(p);}
inline const char* marketLabel(StockMarket m){return m==StockMarket::US?"美股":m==StockMarket::HK?"港股":m==StockMarket::CN?"A股":"市场";}
inline int64_t quoteEpoch(const std::string& raw,StockMarket market){
 std::string digits;for(unsigned char c:raw)if(std::isdigit(c))digits+=c;
 if(digits.size()!=14)return 0;
 auto num=[&](int p,int n){int v=0;for(int i=p;i<p+n;++i)v=v*10+digits[i]-'0';return v;};
 int y=num(0,4),mo=num(4,2),day=num(6,2),h=num(8,2),mi=num(10,2),sec=num(12,2);
 if(y<2020||y>2100||mo<1||mo>12||day<1||day>31||h>23||mi>59||sec>59)return 0;
 int64_t local=civilDays(y,mo,day)*86400+h*3600+mi*60+sec;
 return local-(market==StockMarket::US?nyOffsetForDate(local):8*3600);
}
enum class QuoteAge {MISSING,OLD,DELAYED,OK};
inline QuoteAge quoteAge(const MarketSession&s,const std::string& raw,int64_t now){
 int64_t q=quoteEpoch(raw,s.market);if(!q||!s.date)return QuoteAge::MISSING;
 time_t value=q+s.offset;tm d;gmtime_r(&value,&d);int date=(d.tm_year+1900)*10000+(d.tm_mon+1)*100+d.tm_mday;
 if(q>now+600)return QuoteAge::MISSING;
 if(date<s.date)return QuoteAge::OLD;
 if(activePhase(s.phase)&&now-q>5*60)return QuoteAge::DELAYED;
 return QuoteAge::OK;
}

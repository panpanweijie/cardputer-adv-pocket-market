#include <cassert>
#include <vector>
#include "market_summary.h"
int main(){
 auto now=parseCivil("2026-10-20 02:00:00");MarketFeed feed;
 assert(decodeMarketFeed("2026-10-20 10:00:00|SH_open_交易中|SZ_open_交易中|HK_close_假期休市|US_close_已收盘",feed));
 std::vector<std::string> codes={"sh600519","sz300750","hk00700","usAAPL"};auto p=marketsIn(codes);
 assert(marketSummary(p,now,feed)==u8"沪深交易 | 港休市 | 美休市");
 feed.sz.open=false;feed.sz.reason=u8"已休市";assert(marketSummary(p,now,feed)==u8"沪交易 | 深休市 | 港休市 | 美休市");
 codes={"usAAPL","usNVDA"};p=marketsIn(codes);assert(marketSummary(p,now,feed)==u8"美休市");
 codes.push_back("hk00700");p=marketsIn(codes);assert(marketSummary(p,now,feed)==u8"港休市 | 美休市");
 codes.pop_back();p=marketsIn(codes);assert(marketSummary(p,now,feed)==u8"美休市");
 codes.assign(250,"usAAPL");codes.push_back("sz000001");p=marketsIn(codes);assert(p.sz&&p.us&&!p.hk&&!p.sh); // beyond the first viewport
 assert(marketSummary(p,now,MarketFeed{})==u8"深待核 | 美待核");
 codes.clear();p=marketsIn(codes);assert(!p.sh&&!p.sz&&!p.hk&&!p.us);assert(marketSummary(p,now,feed)==u8"添加自选后显示市场状态");
}

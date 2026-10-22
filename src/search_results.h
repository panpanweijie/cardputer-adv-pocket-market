#pragma once
#include "watchlist_utils.h"
struct StockSearchResult {std::string code,name;};
inline std::vector<StockSearchResult> parseSearchResults(const std::string& results){
 std::vector<StockSearchResult> hits;size_t start=0;
 while(start<results.size()&&hits.size()<8){
  size_t end=results.find('^',start);if(end==std::string::npos)end=results.size();
  std::string item=results.substr(start,end-start);start=end+1;
  size_t p=item.find('~');if(p==std::string::npos)continue;
  size_t q=item.find('~',p+1);if(q==std::string::npos)continue;
  size_t r=item.find('~',q+1);if(r==std::string::npos)continue;
  std::vector<std::string> parsed;std::string error;
  if(!parseSymbols(item.substr(0,p)+item.substr(p+1,q-p-1),parsed,error)||parsed.size()!=1)continue;
  if(item.substr(item.rfind('~')+1)=="QZ")continue; // warrant suggestions are outside the stock/ETF watchlist.
  bool duplicate=false;for(const auto& hit:hits)if(hit.code==parsed[0])duplicate=true;
  if(!duplicate)hits.push_back({parsed[0],item.substr(q+1,r-q-1)});
 }return hits;
}

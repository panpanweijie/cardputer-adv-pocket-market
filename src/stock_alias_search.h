#pragma once
#include "stock_aliases.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cstring>
inline std::string stockQueryKey(const std::string&text){std::string out;for(unsigned char c:text){if(c==' '||c=='\t'||c=='\r'||c=='\n'||c=='-'||c=='\'')continue;out+=c>='A'&&c<='Z'?c+32:c;}return out;}
inline int aliasTokenScore(const std::string&q,const char*raw){
 std::string key=stockQueryKey(raw);if(q==key)return 1;
 if(q.size()>=2&&key.compare(0,q.size(),q)==0)return 4;
 return 99;
}
inline int stockAliasScore(const StockAlias&a,const std::string&q){
 if(q==stockQueryKey(a.code)||q==stockQueryKey(a.code+2))return 0;
 // Accept unpadded numeric HK code only for exact known symbols.
 if(std::strncmp(a.code,"hk",2)==0){std::string n=a.code+2;auto first=n.find_first_not_of('0');if(first!=std::string::npos&&q==n.substr(first))return 0;}
 int score=std::min(aliasTokenScore(q,a.name),std::min(aliasTokenScore(q,a.pinyin),aliasTokenScore(q,a.initials)));
 std::string extra=a.aliases;size_t from=0;
 while(from<=extra.size()){size_t end=extra.find(';',from);if(end==std::string::npos)end=extra.size();score=std::min(score,aliasTokenScore(q,extra.substr(from,end-from).c_str()));if(end==extra.size())break;from=end+1;}
 return score;
}
// Keep only bounded indices and scores. Never build a full dictionary in RAM.
inline std::vector<const StockAlias*> localStockMatches(const std::string&query,bool exactOnly=false){
 struct Match{const StockAlias*row;int score;};std::vector<Match> top;const auto key=stockQueryKey(query);if(key.empty()||query.size()>96)return {};
 for(const auto&row:STOCK_ALIASES){int score=stockAliasScore(row,key);if(score>(exactOnly?1:4))continue;
  auto it=std::find_if(top.begin(),top.end(),[&](const Match&m){return score<m.score;});top.insert(it,{&row,score});if(top.size()>8)top.pop_back();
 }
 std::vector<const StockAlias*> out;for(auto&m:top)out.push_back(m.row);return out;
}

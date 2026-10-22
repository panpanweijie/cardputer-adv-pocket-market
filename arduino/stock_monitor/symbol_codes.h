#pragma once
#include <string>
// Provider exchange routing tags are not part of a saved US ticker.
// Keep genuine class suffixes (.A/.B), and keep raw quote.exchange for charts.
inline std::string canonicalUsTicker(std::string ticker){
 while(true){size_t dot=ticker.rfind('.');if(dot==std::string::npos)break;auto tag=ticker.substr(dot);
  if(tag!=".AM"&&tag!=".OQ"&&tag!=".N"&&tag!=".O"&&tag!=".PS"&&tag!=".PK")break;
  ticker.erase(dot);
 }
 return ticker;
}
inline std::string canonicalWatchSymbol(const std::string&code){return code.compare(0,2,"us")==0?"us"+canonicalUsTicker(code.substr(2)):code;}

// Class-share minute routes require the provider exchange suffix. Never save it as the ticker.
inline std::string premarketRequestCode(const std::string& saved,const std::string& exchange){
 if(saved.compare(0,2,"us")!=0)return saved;
 const auto ticker=saved.substr(2);
 if(ticker.find('.')==std::string::npos || exchange.empty() || canonicalUsTicker(exchange)!=ticker)return saved;
 for(unsigned char c:exchange)if(!((c>='A'&&c<='Z')||(c>='0'&&c<='9')||c=='.'||c=='-'||c=='^'))return saved;
 return "us"+exchange;
}

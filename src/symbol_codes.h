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

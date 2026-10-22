#pragma once
#include "symbol_codes.h"
#include <string>
#include <vector>
#include <cstdlib>
#include <cmath>
inline std::string summaryCode(const std::string& code){return code.compare(0,2,"hk")==0?"r_"+code:canonicalWatchSymbol(code);}
struct SummaryQuote {std::string name,exchange,price,time,change,percent,english;};
inline bool quoteNumber(const std::string&s,double&out){if(s.empty())return false;char*end=nullptr;out=std::strtod(s.c_str(),&end);return end&&*end=='\0'&&std::isfinite(out);}
inline bool decodeSummary(const std::string&payload,const std::string&request,SummaryQuote&out){
 // Match the complete provider identifier, never a substring of another symbol.
 const std::string marker="v_"+request+"=\"";auto begin=payload.find(marker);if(begin==std::string::npos)return false;begin+=marker.size();auto end=payload.find('"',begin);if(end==std::string::npos)return false;
 std::vector<std::string> fields;size_t p=begin;while(p<=end){auto n=payload.find('~',p);if(n==std::string::npos||n>end)n=end;fields.push_back(payload.substr(p,n-p));if(n==end)break;p=n+1;}
 if(fields.size()<33)return false;double price,change,percent;if(!quoteNumber(fields[3],price)||price<=0||!quoteNumber(fields[31],change)||!quoteNumber(fields[32],percent))return false;
 // Reject a mismatched security even if a server unexpectedly labels it incorrectly.
 std::string code=request.compare(0,2,"r_")==0?request.substr(2):request;std::string expected=code.substr(2),actual=fields[2];if(code.compare(0,2,"us")==0){if(actual!=expected){auto dot=actual.rfind('.');if(dot!=std::string::npos)actual.resize(dot);}}if(actual!=expected)return false;
 out={fields[1],fields[2],fields[3],fields[30],fields[31],fields[32],fields.size()>46?fields[46]:""};return true;
}

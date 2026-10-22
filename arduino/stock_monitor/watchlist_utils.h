#pragma once
#include "symbol_codes.h"
#include <string>
#include <vector>
#include <algorithm>
#include <cctype>
inline bool parseSymbols(const std::string& text,std::vector<std::string>& result,std::string& error) {
 result.clear();size_t begin=0;
 while(begin<text.size()){
  size_t end=text.find(',',begin);if(end==std::string::npos)end=text.size();
  std::string code=text.substr(begin,end-begin);begin=end+1;
  while(!code.empty()&&std::isspace((unsigned char)code.front()))code.erase(code.begin());
  while(!code.empty()&&std::isspace((unsigned char)code.back()))code.pop_back();
  if(code.empty())continue;
  if(code.size()<3){error="Bad symbol";return false;}
  code[0]=std::tolower((unsigned char)code[0]);code[1]=std::tolower((unsigned char)code[1]);
  std::string market=code.substr(0,2);
  if(market!="us"&&market!="hk"&&market!="sh"&&market!="sz"){error="Use us/hk/sh/sz prefix";return false;}
  for(size_t i=2;i<code.size();++i){char&c=code[i];c=std::toupper((unsigned char)c);if(!std::isalnum((unsigned char)c)&&c!='.'&&c!='-'&&c!='^'){error="Invalid symbol character";return false;}}
  code=canonicalWatchSymbol(code);
  if(code.size()<3){error="Missing ticker";return false;}
  if(std::find(result.begin(),result.end(),code)==result.end())result.push_back(code);
 }
 return true;
}
inline int zoomWindow(int current,int count,bool zoomIn){
 if(count<=0)return current;
 int available=std::min(current,count),floor=std::min(5,count);
 return zoomIn?std::max(floor,available/2):std::min(count,std::max(floor,available*2));
}

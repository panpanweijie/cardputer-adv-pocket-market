#pragma once
#include <string>
#include "gbk_table.h"
inline bool validUtf8(const std::string&s){
 for(size_t i=0;i<s.size();){uint8_t c=s[i++];if(c<0x80)continue;int n;uint32_t code;
  if(c>=0xc2&&c<=0xdf){n=1;code=c&31;}else if(c>=0xe0&&c<=0xef){n=2;code=c&15;}else if(c>=0xf0&&c<=0xf4){n=3;code=c&7;}else return false;
  if(i+n>s.size())return false;
  for(int j=0;j<n;++j){uint8_t b=s[i++];if((b&0xc0)!=0x80)return false;code=(code<<6)|(b&63);}
  if((n==1&&code<0x80)||(n==2&&code<0x800)||(n==3&&code<0x10000)||code>0x10ffff||(code>=0xd800&&code<=0xdfff))return false;
 }return true;
}
inline std::string gbkToUtf8(const std::string&s){
 if(validUtf8(s))return s;
 std::string out;out.reserve(s.size()*3/2);
 for(size_t i=0;i<s.size();){uint8_t lead=s[i++];if(lead<0x80){out.push_back(lead);continue;}
  if(lead<0x81||lead>0xfe||i>=s.size()){out+='?';continue;}
  uint8_t trail=s[i];if(trail<0x40||trail==0x7f||trail>0xfe){out+='?';continue;}++i;
  unsigned index=(lead-0x81)*190+(trail-0x40)-(trail>0x7f?1:0);
#ifdef ARDUINO
  uint16_t u=pgm_read_word(&GBK_TO_UNICODE[index]);
#else
  uint16_t u=GBK_TO_UNICODE[index];
#endif
  if(!u){out+='?';continue;}
  if(u<0x800){out.push_back(0xc0|(u>>6));out.push_back(0x80|(u&63));}
  else{out.push_back(0xe0|(u>>12));out.push_back(0x80|((u>>6)&63));out.push_back(0x80|(u&63));}
 }return out;
}
inline void removeLastUtf8(std::string&s){
 if(s.empty())return;size_t i=s.size()-1;while(i>0&&((uint8_t)s[i]&0xc0)==0x80)--i;s.erase(i);
}

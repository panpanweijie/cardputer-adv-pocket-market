#include <cassert>
#include "offline_pinyin.h"
#include "text_encoding.h"
void type(OfflinePinyin &ime,const char* s){ime.clear();while(*s)ime.append(*s++);}
int main(){
 OfflinePinyin ime;assert(!ime.count&&ime.candidate(0)==nullptr);
 type(ime,"tengxun");assert(std::string(ime.candidate(0))==u8"腾讯");assert(std::string(ime.candidate(1))==u8"腾讯控股");
 type(ime,"GUIZHOUMAOTAI");assert(std::string(ime.candidate(0))==u8"贵州茅台");
 type(ime,"ningdeshidai");assert(std::string(ime.candidate(0))==u8"宁德时代");
 type(ime,"zhong");assert(std::string(ime.candidate(0))==u8"中");assert(ime.count>3&&ime.count<=24);ime.turn(1);assert(ime.page==1);ime.turn(-1);assert(ime.page==0);ime.turn(-1);assert(ime.page==(ime.count+2)/3-1);
 for(int i=0;i<100;++i)ime.append('a');assert(ime.composing.size()==32);assert(ime.count==0);ime.backspace();assert(ime.composing.size()==31);
 type(ime,"zzz");assert(!ime.count);assert(ime.candidate(1)==nullptr);
 for(auto& entry:PINYIN_WORDS){assert(validUtf8(entry.text));assert(std::strlen(entry.key)>0);}
 std::string query=u8"腾讯控股";removeLastUtf8(query);assert(query==u8"腾讯控");
 ime.clear();assert(ime.composing.empty()&&ime.count==0&&ime.page==0);
}

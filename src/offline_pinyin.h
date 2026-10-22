#pragma once
#include <array>
#include <string>
#include <cstring>
#include "pinyin_words.h"
// Fixed-size candidate indices; dictionary is read directly from Flash.
class OfflinePinyin {
 public:
 std::string composing;
 std::array<unsigned short,24> matches{};
 int count=0,page=0;
 void refresh(){
  count=page=0;if(composing.empty())return;
  for(int pass=0;pass<2;++pass){
   for(size_t i=0;i<sizeof(PINYIN_WORDS)/sizeof(PINYIN_WORDS[0])&&count<24;++i){
    auto key=PINYIN_WORDS[i].key;bool exact=composing==key;
    if((pass==0?!exact:exact)||std::strncmp(key,composing.c_str(),composing.size())!=0)continue;
    bool duplicate=false;for(int j=0;j<count;++j)if(std::strcmp(PINYIN_WORDS[matches[j]].text,PINYIN_WORDS[i].text)==0)duplicate=true;
    if(!duplicate)matches[count++]=i;
   }
  }
 }
 void clear(){composing.clear();count=page=0;}
 void append(char c){if(c>='A'&&c<='Z')c+=32;if(c>='a'&&c<='z'&&composing.size()<32){composing+=c;refresh();}}
 void backspace(){if(!composing.empty())composing.pop_back();refresh();}
 void turn(int direction){if(count){int pages=(count+2)/3;page=(page+direction+pages)%pages;}}
 const char* candidate(int slot)const{int i=page*3+slot;return slot>=0&&slot<3&&i<count?PINYIN_WORDS[matches[i]].text:nullptr;}
};

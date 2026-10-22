#pragma once
#include "search_results.h"
#include "offline_pinyin.h"
#include "stock_alias_search.h"
// HTTPClient handles chunked transfer; this sink enforces a hard response cap.
class SearchResponse:public Stream {
 public:String body;bool overflow=false;
 size_t write(uint8_t c) override {if(body.length()>=12288){overflow=true;return 0;}return body.concat((char)c)?1:0;}
 int available() override{return 0;}int read() override{return -1;}int peek() override{return -1;}void flush() override{}
};
struct SearchHit { String code,name; };
std::vector<SearchHit> searchHits;
String searchQuery,searchStatus="Code / name / pinyin / initials";
int searchCursor=0;
bool chineseInput=true;
OfflinePinyin pinyin;
void searchText(String value,int x,int y,int width,uint16_t color){
 screen->setFont(&fonts::efontCN_14);screen->setTextSize(1);screen->setTextColor(color);screen->setTextWrap(false);
 std::string label=value.c_str();while(!label.empty()&&screen->textWidth(label.c_str())>width)removeLastUtf8(label);
 screen->setCursor(x,y);screen->print(label.c_str());screen->setFont(&fonts::Font0);
}
void showSearch(){
 frame("FIND STOCK",chineseInput?"CN / TAB EN":"EN / TAB CN");
 screen->setFont(&fonts::efontCN_14);screen->setTextSize(1);
 std::string tail=searchQuery.c_str();
 while(!tail.empty()&&screen->textWidth(("> "+tail+"_").c_str())>222){size_t cut=1;while(cut<tail.size()&&((uint8_t)tail[cut]&0xc0)==0x80)++cut;tail.erase(0,cut);}
 searchText(String(("> "+tail+"_").c_str()),8,23,222,INK);
 if(chineseInput&&!pinyin.composing.empty()){
  textAt(String(pinyin.composing.c_str()),8,48,ACCENT);
  textAt(String(pinyin.page+1)+"/"+String(max(1,(pinyin.count+2)/3)),0,48,MUTED,1,232);
  for(int slot=0;slot<3;++slot){const char* word=pinyin.candidate(slot);int x=6+slot*77;
   screen->fillRoundRect(x,68,74,24,2,PANEL);
   textAt(String(slot+1),x+2,76,ACCENT);
   if(word)searchText(word,x+10,73,62,INK);
  }
  if(!pinyin.count)textAt("ENT search / DEL edit / TAB EN",8,102,MUTED);
  footer("ENT find 1-3 pick SPC first");
 }else{
  if(searchHits.empty())textAt(fitText(searchStatus,222),8,56,MUTED);
  else {int first=searchCursor/3*3;for(int row=0;row<3&&first+row<(int)searchHits.size();++row){
   int i=first+row,y=42+row*24;screen->fillRoundRect(6,y,228,22,2,i==searchCursor?EDGE:PANEL);
   textAt(searchHits[i].code,10,y+7,i==searchCursor?ACCENT:MUTED);
   searchText(searchHits[i].name,90,y+3,138,INK);
  }}
  footer(searchHits.empty()?"ENT search TAB CN/EN ` back":";/. select ENT add DEL edit");
 }
 presentUI();
}
void leaveSearch(){std::vector<SearchHit>().swap(searchHits);searchQuery="";pinyin.clear();searchStatus="Code / name / pinyin / initials";currentState=searchReturnState;if(currentState==STATE_CHART)showChart();else showHome();}
void runStockSearch(){
 searchHits.clear();searchCursor=0;
 if(!searchQuery.length()){searchStatus="Code / name / pinyin / initials";showSearch();return;}
 for(const auto*hit:localStockMatches(searchQuery.c_str()))searchHits.push_back({hit->code,hit->name});
 if(!searchHits.empty()){searchStatus="Local matches";showSearch();return;}
 if(WiFi.status()!=WL_CONNECTED){searchStatus="WiFi offline. ` back > TAB";showSearch();return;}
 if(ESP.getFreeHeap()<80000||ESP.getMaxAllocHeap()<24000){searchStatus="Low memory; shorten watchlist";showSearch();return;}
 searchStatus="Searching...";showSearch();
 String encoded;const char hex[]="0123456789ABCDEF";
 for(unsigned char c:searchQuery){if(isalnum(c)||c=='-'||c=='.')encoded+=(char)c;else{encoded+='%';encoded+=hex[c>>4];encoded+=hex[c&15];}}
 WiFiClientSecure client;client.setInsecure();client.setHandshakeTimeout(5);client.setTimeout(5);HTTPClient http;
 prepareHttp(http);
 if(!http.begin(client,"https://smartbox.gtimg.cn/s3/?q="+encoded+"&t=all")){searchStatus="Search connection failed";showSearch();return;}
 int status=http.GET();
 SearchResponse response;
 int received=status==200?http.writeToStream(&response):-1;
 if(status==200&&received>=0&&!response.overflow){
  String &body=response.body;int a=body.indexOf('"'),b=body.lastIndexOf('"');
  if(a>=0&&b>a){DynamicJsonDocument doc(12288);auto error=deserializeJson(doc,body.substring(a,b+1));
   if(!error){String results=doc.as<String>();
    for(const auto& hit:parseSearchResults(results.c_str()))searchHits.push_back({String(hit.code.c_str()),String(hit.name.c_str())});
    searchStatus=searchHits.empty()?"No supported stocks found":"";
   }else searchStatus="Search response invalid";
  }else searchStatus="Search response invalid";
 }else searchStatus=status==200?"Search response too large":"Search HTTP "+String(status);
 http.end();showSearch();
}
void addSearchHit(){
 if(searchHits.empty())return;String code=searchHits[searchCursor].code;
 for(int i=0;i<stockCount;++i)if(stockCodes[i]==code){leaveSearch();homeScrollOffset=i/3*3;showHome();return;}
 String previous=prefs.getString("list","");String next;
 for(auto &c:stockCodes){if(next.length())next+=',';next+=c;}if(next.length())next+=',';next+=code;
 if(!loadStocksFromString(next)){searchHits.clear();searchStatus=watchlistError;showSearch();return;}
 if(prefs.putString("list",next)!=next.length()){loadStocksFromString(previous);searchHits.clear();searchStatus="Storage full; not saved";showSearch();return;}
 leaveSearch();homeScrollOffset=max(0,(stockCount-1)/3*3);showHome();
}

void commitPinyin(int slot){
 const char* word=pinyin.candidate(slot);if(!word)return;
 if(searchQuery.length()+strlen(word)>96){searchStatus="Query too long";return;}
 searchQuery+=word;pinyin.clear();
}
void handleSearchKeys(const Keyboard_Class::KeysState& st,bool esc,bool enter,bool del,bool tab){
 if(esc){if(!pinyin.composing.empty()){pinyin.clear();showSearch();}else leaveSearch();return;}
 if(tab){
  // Keep unconverted letters when switching to English (e.g. AAPL).
  if(!pinyin.composing.empty()&&searchQuery.length()+pinyin.composing.size()<=96)searchQuery+=pinyin.composing.c_str();
  pinyin.clear();chineseInput=!chineseInput;searchHits.clear();showSearch();return;
 }
 if(enter){
  if(!pinyin.composing.empty()){
   std::string query=std::string(searchQuery.c_str())+pinyin.composing;
   if(!localStockMatches(query,true).empty()||!pinyin.count){
    if(query.size()<=96){searchQuery=query.c_str();pinyin.clear();runStockSearch();}else searchStatus="Query too long";
   }else commitPinyin(0);
  }else if(searchHits.empty())runStockSearch();else addSearchHit();
  if(currentState==STATE_SEARCH)showSearch();return;
 }
 if(del){searchHits.clear();if(!pinyin.composing.empty())pinyin.backspace();else{std::string query=searchQuery.c_str();removeLastUtf8(query);searchQuery=query.c_str();}searchStatus="Code / name / pinyin / initials";showSearch();return;}
 for(char c:st.word){
  if(!searchHits.empty()){if(c==';'||c==',')searchCursor=max(0,searchCursor-1);if(c=='.'||c=='/')searchCursor=min((int)searchHits.size()-1,searchCursor+1);}
  else if(chineseInput){
   if(c>='1'&&c<='3'&&!pinyin.composing.empty())commitPinyin(c-'1');
   else if(c==' '&&!pinyin.composing.empty())commitPinyin(0);
   else if((c==';'||c==',')&&!pinyin.composing.empty())pinyin.turn(-1);
   else if((c=='.'||c=='/')&&!pinyin.composing.empty())pinyin.turn(1);
   else if((c>='a'&&c<='z')||(c>='A'&&c<='Z'))pinyin.append(c);
   else if(c>='0'&&c<='9'&&pinyin.composing.empty()&&searchQuery.length()<96)searchQuery+=c;
  }else if(c>=32&&c<=126&&c!='`'&&searchQuery.length()<96)searchQuery+=c;
 }
 showSearch();
}

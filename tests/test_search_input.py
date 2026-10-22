"""Exercise the actual firmware keyboard handler with small host-side UI stubs."""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[1]
source=(root/'src/search_view.h').read_text().split('void commitPinyin(int slot){',1)[1]
harness=r'''
#include <cassert>
#include <vector>
#include "offline_pinyin.h"
#include "stock_alias_search.h"
#include "text_encoding.h"
struct String:std::string{using std::string::string;String(){}String(const std::string&s):std::string(s){}unsigned length()const{return size();}};
namespace Keyboard_Class {struct KeysState{std::vector<char> word;};}
enum {STATE_HOME,STATE_SEARCH};int currentState=STATE_SEARCH;
String searchQuery,searchStatus;OfflinePinyin pinyin;bool chineseInput=true;
std::vector<int> searchHits;int searchCursor=0,searchCalls=0;
using std::min;using std::max;
void showSearch(){}void leaveSearch(){currentState=STATE_HOME;}
void runStockSearch(){++searchCalls;}void addSearchHit(){currentState=STATE_HOME;}
void commitPinyin(int slot){
'''+source+r'''
void keys(const char*s){Keyboard_Class::KeysState st;while(*s)st.word.push_back(*s++);handleSearchKeys(st,false,false,false,false);}
void special(bool enter=false,bool del=false,bool tab=false,bool esc=false){Keyboard_Class::KeysState st;handleSearchKeys(st,esc,enter,del,tab);}
int main(){
 keys("tengxun");assert(pinyin.composing=="tengxun");assert(searchCalls==0);keys("1");assert(searchQuery==u8"腾讯");assert(pinyin.composing.empty());
 special(true);assert(searchCalls==1);special(false,true);assert(searchQuery==u8"腾");assert(validUtf8(searchQuery));special(false,true);assert(searchQuery.empty());
 keys("fbu");assert(pinyin.composing=="fbu");special(false,false,false,true);assert(currentState==STATE_SEARCH&&pinyin.composing.empty());
 keys("AAPL");special(false,false,true);assert(!chineseInput&&searchQuery=="aapl"&&pinyin.composing.empty());
 keys(".B");assert(searchQuery=="aapl.B");searchQuery="";keys("00700");assert(searchQuery=="00700");
 special(false,false,true);searchQuery="";keys("00700");assert(searchQuery=="00700");
 searchQuery="";keys("zhong");int page=pinyin.page;keys(".");assert(pinyin.page!=page);keys(";");assert(pinyin.page==page);
 special(true);assert(searchQuery==u8"中"&&searchCalls==1);special(true);assert(searchCalls==2);
 searchHits.clear();searchQuery="";pinyin.clear();keys("kekokele");int previous=searchCalls;special(true);assert(searchCalls==previous+1&&searchQuery=="kekokele"&&pinyin.composing.empty());
 searchQuery="";keys("kkkl");previous=searchCalls;special(true);assert(searchCalls==previous+1&&searchQuery=="kkkl");
 searchQuery="";keys("zzzzzz");previous=searchCalls;special(true);assert(searchCalls==previous+1&&searchQuery=="zzzzzz");
 searchHits={1};special(true);assert(currentState==STATE_HOME);
}
'''
with tempfile.TemporaryDirectory() as d:
    src=Path(d)/'input.cpp';src.write_text(harness);exe=Path(d)/'input'
    subprocess.run(['c++','-std=c++11','-I',str(root/'src'),str(src),'-o',str(exe)],check=True)
    subprocess.run([str(exe)],check=True)
print('Search keyboard and UTF-8 edit tests passed')

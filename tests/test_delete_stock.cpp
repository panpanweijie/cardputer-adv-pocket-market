#include <cassert>
#include <string>
#include <vector>
#include <algorithm>
#include <map>
using std::min;using std::max;
struct String:std::string {
 using std::string::string;String(){}String(const std::string&s):std::string(s){}
 unsigned length()const{return size();}
 bool reserve(size_t n){std::string::reserve(n);return true;}
};
struct StockInfo {int id;};
std::vector<String> stockCodes;std::vector<StockInfo> stocks;
enum {STATE_HOME,STATE_CHART,STATE_DELETE_CONFIRM};
int currentState,deleteStockIdx,stockCount,currentChartIdx,homeScrollOffset,chartLoadedIdx,chartCount,quoteScopeFirst,quoteScopeEnd;
String deleteStockCode,deleteError;
struct Prefs {
 bool fail=false;int writes=0;std::map<std::string,String> values;
 size_t putString(const char* key,const String& value){++writes;if(fail)return 0;values[key]=value;return value.size();}
 String getString(const char* key,const char* fallback){return values.count(key)?values[key]:String(fallback);}
}prefs;
void updateListedMarkets(){}
void showDeleteConfirm(){}void showChart(){}void showHome(){}
#include "delete_stock.h"
void reset(){
 stockCodes={"hk00700","usAAPL","sh600519"};stocks={{1},{2},{3}};stockCount=3;currentChartIdx=1;homeScrollOffset=0;currentState=STATE_CHART;
 chartLoadedIdx=1;chartCount=30;quoteScopeFirst=1;quoteScopeEnd=2;deleteStockIdx=-1;deleteStockCode="";deleteError="";
 prefs.fail=false;prefs.writes=0;prefs.values["list"]="hk00700,usAAPL,sh600519";
}
int main(){
 reset();beginDeleteStock();assert(currentState==STATE_DELETE_CONFIRM&&prefs.writes==0&&stockCount==3);
 handleDeleteKeys(false,false);assert(stockCount==3&&prefs.writes==0); // D/DEL/other keys
 handleDeleteKeys(true,true);assert(currentState==STATE_CHART&&stockCount==3&&prefs.writes==0); // ESC wins
 beginDeleteStock();handleDeleteKeys(false,true);assert(currentState==STATE_HOME&&stockCount==2&&prefs.values["list"]=="hk00700,sh600519");
 assert(stocks[0].id==1&&stocks[1].id==3&&stockCodes[1]=="sh600519");assert(chartLoadedIdx==-1&&chartCount==0&&quoteScopeFirst==-1&&currentChartIdx==1);
 reset();beginDeleteStock();prefs.fail=true;handleDeleteKeys(false,true);assert(stockCount==3&&currentState==STATE_DELETE_CONFIRM&&!deleteError.empty());assert(prefs.values["list"]=="hk00700,usAAPL,sh600519");
 prefs.fail=false;handleDeleteKeys(false,true);assert(stockCount==2&&currentState==STATE_HOME);
 reset();beginDeleteStock();stockCodes[1]="usNVDA";handleDeleteKeys(false,true);assert(stockCount==3&&prefs.writes==0&&currentState==STATE_DELETE_CONFIRM);
 reset();stockCodes={"usAAPL"};stocks={{2}};stockCount=1;currentChartIdx=0;beginDeleteStock();prefs.fail=true;handleDeleteKeys(false,true);assert(stockCount==1&&currentState==STATE_DELETE_CONFIRM);
 prefs.fail=false;handleDeleteKeys(false,true);assert(stockCount==0&&stockCodes.empty()&&stocks.empty()&&prefs.values["list"]=="");assert(homeScrollOffset==0&&currentChartIdx==0&&currentState==STATE_HOME);
 reset();currentChartIdx=2;beginDeleteStock();handleDeleteKeys(false,true);assert(stockCount==2&&stockCodes.back()=="usAAPL"&&currentChartIdx==1);
 reset();currentState=STATE_HOME;beginDeleteStock();assert(currentState==STATE_HOME&&prefs.writes==0);
}

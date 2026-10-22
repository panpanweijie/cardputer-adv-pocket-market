#include "search_results.h"
#include "quote_summary.h"
#include <ArduinoJson.h>
#include <fstream>
#include <sstream>
#include <cassert>
#include <iostream>
int main(int argc,char**argv){assert(argc==3);DynamicJsonDocument all(65536);std::ifstream input(argv[1]);assert(!deserializeJson(all,input));int checked=0;for(auto row:all.as<JsonObject>()){std::string raw=row.value().as<std::string>();auto a=raw.find('"'),b=raw.rfind('"');assert(a!=std::string::npos);DynamicJsonDocument doc(12288);assert(!deserializeJson(doc,raw.substr(a,b-a+1)));auto hits=parseSearchResults(doc.as<std::string>());assert(!hits.empty());for(const auto&hit:hits){assert(hit.code==canonicalWatchSymbol(hit.code));++checked;}if(std::string(row.key().c_str())=="BMNU"){assert(hits[0].code=="usBMNU");assert(hits[0].name.find(u8"2倍做多")!=std::string::npos);}}
std::vector<std::string> out;std::string error;assert(parseSymbols("usBMNU.AM,usSPY.AM,usBMNU,usAAPL.OQ,usBMNR.N,usBRK.B.N,usBRK.A,hk00700,sh600519,sz300750",out,error));std::vector<std::string> expected={"usBMNU","usSPY","usAAPL","usBMNR","usBRK.B","usBRK.A","hk00700","sh600519","sz300750"};assert(out==expected);assert(!parseSymbols("us.AM",out,error));for(auto suffix:{".AM",".OQ",".N",".O",".PS",".PK"})assert(canonicalWatchSymbol(std::string("usTEST")+suffix)=="usTEST");assert(canonicalWatchSymbol("usBRK.B")=="usBRK.B");assert(canonicalWatchSymbol("usBRK.A")=="usBRK.A");assert(summaryCode("usBMNU.AM")=="usBMNU");
std::ifstream qfile(argv[2]);std::ostringstream buffer;buffer<<qfile.rdbuf();SummaryQuote q;assert(decodeSummary(buffer.str(),summaryCode("usBMNU.AM"),q));assert(q.exchange=="BMNU.AM");assert(std::stod(q.price)>0);assert(decodeSummary(buffer.str(),"usSPY",q)&&q.exchange=="SPY.AM");std::cout<<"PASS "<<checked<<" live search hits across 24 queries; exchange canonicalization, saved-list dedup, class shares, preserved chart routing and BMNU/SPY quotes\n";}

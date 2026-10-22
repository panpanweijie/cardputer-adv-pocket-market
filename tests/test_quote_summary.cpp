#include "quote_summary.h"
#include <cassert>
#include <fstream>
#include <sstream>
#include <iostream>
int main(int argc,char**argv){assert(argc==2);std::ifstream f(argv[1],std::ios::binary);std::ostringstream b;b<<f.rdbuf();auto raw=b.str();SummaryQuote q;
for(auto code:{"hk00700","hk09988","sh000001","sz000001","sz159934","usTSLA","usSPY"}){assert(decodeSummary(raw,summaryCode(code),q));double price,change,percent;assert(quoteNumber(q.price,price));assert(quoteNumber(q.change,change));assert(quoteNumber(q.percent,percent));double previous=price-change;assert(previous>0);assert(std::abs(change/previous*100-percent)<0.011);}
assert(decodeSummary(raw,"r_hk00700",q));assert(q.time=="2026/10/20 13:48:01");assert(decodeSummary(raw,"hk00700",q));assert(q.time=="2026/10/20 13:32:57");assert(!decodeSummary(raw,"r_hk0070",q));assert(!decodeSummary("v_hk00700=\"bad\";","hk00700",q));
std::vector<std::string> fields(47,"");fields[2]="BRK.B.N";fields[3]="500.1";fields[31]="1.1";fields[32]="0.22";auto make=[&](){std::string s="v_usBRK.B=\"";for(size_t i=0;i<fields.size();++i)s+=(i?"~":"")+fields[i];return s+"\";";};assert(decodeSummary(make(),"usBRK.B",q));fields[3]="NaN";assert(!decodeSummary(make(),"usBRK.B",q));fields[3]="500garbage";assert(!decodeSummary(make(),"usBRK.B",q));fields[3]="500";fields[2]="WRONG.N";assert(!decodeSummary(make(),"usBRK.B",q));std::cout<<"PASS quote field mapping, exact symbol, HK live vs delayed, US dotted ticker, malformed values and arithmetic\n";}

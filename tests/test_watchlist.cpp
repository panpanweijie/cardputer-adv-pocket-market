#include <cassert>
#include <iostream>
#include "../src/watchlist_utils.h"
int main(){
 std::vector<std::string> codes;std::string error;
 assert(parseSymbols(" USAapl, hk00700,usAAPL, sz159934, ,sh600519 ",codes,error));
 assert(codes.size()==4&&codes[0]=="usAAPL"&&codes[1]=="hk00700");
 std::string large;for(int i=0;i<250;++i){if(i)large+=",";large+="usT"+std::to_string(i);}
 assert(parseSymbols(large,codes,error)&&codes.size()==250);
 assert(!parseSymbols("usAAPL&x=1",codes,error));assert(!parseSymbols("AAPL",codes,error));
 assert(parseSymbols("",codes,error)&&codes.empty());
 assert(zoomWindow(300,300,true)==150);assert(zoomWindow(150,300,true)==75);
 assert(zoomWindow(75,300,false)==150);assert(zoomWindow(150,300,false)==300);
 assert(zoomWindow(300,5,true)==5);assert(zoomWindow(60,2,true)==2);
 assert(zoomWindow(5,300,true)==5);assert(zoomWindow(300,300,false)==300);
 std::cout<<"PASS: 250 symbols without fixed count cap; normalize/deduplicate; invalid input; empty watchlist; meaningful zoom boundaries\n";
}

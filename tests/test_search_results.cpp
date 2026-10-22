#include <cassert>
#include <fstream>
#include <sstream>
#include <ArduinoJson.h>
#include "search_results.h"
int main(int argc,char**argv){
 assert(argc==2);std::ifstream f(argv[1]);std::stringstream buf;buf<<f.rdbuf();auto body=buf.str();auto a=body.find('"'),b=body.rfind('"');
 DynamicJsonDocument doc(12288);assert(!deserializeJson(doc,body.substr(a,b-a+1)));
 auto hits=parseSearchResults(doc.as<std::string>());assert(!hits.empty());bool found=false;for(auto& hit:hits)if(hit.code=="hk00700"&&hit.name==u8"腾讯控股")found=true;assert(found);
 assert(parseSearchResults("jj~007005~Fund~x^broken^hk~00700~Name~x^hk~00700~Duplicate~x").size()==1);
 assert(parseSearchResults("").empty());
 auto us=parseSearchResults("us~aapl.oq~Apple~pg~GP^us~brk.b.n~Berkshire~b~GP");assert(us.size()==2&&us[0].code=="usAAPL"&&us[1].code=="usBRK.B");
 assert(parseSearchResults("hk~13005~Warrant~w~QZ").empty());
 std::string many;for(int i=0;i<20;++i)many+="us~A"+std::to_string(i)+"~Name~x^";assert(parseSearchResults(many).size()==8);
}

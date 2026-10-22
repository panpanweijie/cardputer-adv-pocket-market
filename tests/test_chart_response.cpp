#include <cassert>
#include <fstream>
#include <iostream>
#include "tencent_data.h"
size_t slots(JsonVariantConst v){size_t n=0;if(v.is<JsonArrayConst>()){for(auto c:v.as<JsonArrayConst>())n+=1+slots(c);}else if(v.is<JsonObjectConst>()){for(auto kv:v.as<JsonObjectConst>())n+=1+slots(kv.value());}return n;}
int main(int argc,char**argv){
 assert(argc==2);DynamicJsonDocument doc(65536),filter(2048);filter["code"]=true;filter["data"]["hk00700"]["day"]=true;
 std::ifstream input(argv[1]);assert(!deserializeJson(doc,input,DeserializationOption::Filter(filter)));assert(doc["code"].as<int>()==0);
 ChartBar bars[300];int count=decodeTencentChart(doc["data"]["hk00700"]["day"],CHART_1D,bars,300);assert(count==120);
 for(int i=1;i<count;++i)assert(bars[i].timestamp>bars[i-1].timestamp);
 size_t estimatedEsp=doc.memoryUsage()-slots(doc.as<JsonVariantConst>())*(JSON_ARRAY_SIZE(1)-16);assert(estimatedEsp<32768);
 std::cout<<"ESP32 budget estimate "<<estimatedEsp<<"; "<<count<<" real HK daily bars; host filtered JSON "<<doc.memoryUsage()<<" bytes (host pointers differ from ESP32)\n";
}

#include "wifi_scan_policy.h"
#include <cassert>
#include <iostream>
int main(){std::vector<WifiScanEntry> a={{"中文网络",-80},{"",-90},{"Home",-50},{"中文网络",-40},{"",-70}};auto b=compactWifiScan(a);assert(b.size()==3);assert(b[0].ssid=="中文网络"&&b[0].rssi==-40);assert(b[2].ssid.empty()&&b[2].rssi==-70);a.clear();for(int i=0;i<40;i++)a.push_back({std::to_string(i),-30-i});assert(compactWifiScan(a).size()==40);assert(compactWifiScan({}).empty());std::cout<<"PASS UTF8 SSID, dedup strongest, sort, hidden, >20 networks\n";}

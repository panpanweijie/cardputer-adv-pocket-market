#pragma once
#include <string>
#include <vector>
#include <algorithm>
struct WifiScanEntry {std::string ssid;int rssi;};
inline std::vector<WifiScanEntry> compactWifiScan(const std::vector<WifiScanEntry>& input){
 std::vector<WifiScanEntry> out;
 for(const auto& e:input){
  auto it=std::find_if(out.begin(),out.end(),[&](const WifiScanEntry& v){return v.ssid==e.ssid;});
  if(it==out.end())out.push_back(e);else if(e.rssi>it->rssi)it->rssi=e.rssi;
 }
 std::stable_sort(out.begin(),out.end(),[](const WifiScanEntry&a,const WifiScanEntry&b){return a.rssi>b.rssi;});
 return out;
}

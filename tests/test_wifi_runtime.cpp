#include <string>
#include <vector>
#include <cassert>
#include <iostream>
#include "wifi_scan_policy.h"
using String=std::string;
constexpr int WIFI_SCANNING_BIT=1,WIFI_SCAN_DONE_BIT=2,WIFI_STA=1,WL_CONNECTED=3,WIFI_SCAN_RUNNING=-1,WIFI_SCAN_FAILED=-2;
enum wifi_ps_type_t{WIFI_PS_NONE,WIFI_PS_MIN_MODEM};
struct WiFiGenericClass{static int bits;protected:static void clearStatusBits(int b){bits&=~b;}};int WiFiGenericClass::bits=0;
unsigned long lastReconnect=0;unsigned long clockNow=100;unsigned long millis(){return clockNow;}
int stopped=0;void esp_wifi_scan_stop(){++stopped;}
struct FakeWiFi{
 bool online=true,autoReconnect=true;int disconnects=0,starts=0,finish=2,startResult=WIFI_SCAN_RUNNING;wifi_ps_type_t sleep=WIFI_PS_MIN_MODEM;
 std::vector<WifiScanEntry> entries={{"中文网络",-40},{"Home",-60}};
 bool getAutoReconnect(){return autoReconnect;}void setAutoReconnect(bool v){autoReconnect=v;}
 wifi_ps_type_t getSleep(){return sleep;}void setSleep(wifi_ps_type_t v){sleep=v;}void setSleep(bool v){sleep=v?WIFI_PS_MIN_MODEM:WIFI_PS_NONE;}
 void mode(int){}int status(){return online?WL_CONNECTED:0;}void disconnect(bool,bool){++disconnects;online=false;}
 void scanDelete(){WiFiGenericClass::bits&=~WIFI_SCAN_DONE_BIT;}
 int scanNetworks(bool,bool,bool,int){++starts;if(startResult==WIFI_SCAN_RUNNING)WiFiGenericClass::bits|=WIFI_SCANNING_BIT;return startResult;}
 int scanComplete(){if(finish>=0){WiFiGenericClass::bits&=~WIFI_SCANNING_BIT;WiFiGenericClass::bits|=WIFI_SCAN_DONE_BIT;}return finish;}
 String SSID(int i){return entries[i].ssid;}int RSSI(int i){return entries[i].rssi;}
}WiFi;
enum{STATE_HOME,STATE_WIFI_SCAN,STATE_SETTINGS};int currentState=STATE_HOME,wifiCount=0,wifiCursor=0;bool wifiScanDone=false,prefetchBusy=false;
std::vector<String>wifiSSIDs;std::vector<int>wifiRSSIs;void showWifiScan(){}
struct {template<class...A>void printf(const char*,A...){}}Serial;
#include "wifi_scan_runtime.h"
void next(){clockNow+=301;tickWifiScan();}
void reset(){cancelWifiScan();WiFi=FakeWiFi{};WiFiGenericClass::bits=0;currentState=STATE_HOME;wifiScanDone=false;wifiCount=0;prefetchBusy=false;}
int main(){
 reset();startWifiScan();assert(WiFi.disconnects==0&&WiFi.online);assert(WiFi.sleep==WIFI_PS_NONE);next();assert(WiFi.starts==1);startWifiScan();assert(WiFi.starts==1);next();assert(wifiScanDone&&wifiCount==2);assert(WiFi.starts==1&&WiFi.online&&WiFi.sleep==WIFI_PS_MIN_MODEM);cancelWifiScan();assert(WiFi.autoReconnect);
 reset();WiFi.online=false;startWifiScan();assert(WiFi.disconnects==1);next();cancelWifiScan();assert(!(WiFiGenericClass::bits&WIFI_SCANNING_BIT));assert(WiFi.autoReconnect);
 reset();WiFi.startResult=WIFI_SCAN_FAILED;startWifiScan();next();assert(!wifiScanDone);next();assert(wifiScanDone&&WiFi.starts==2);
 reset();WiFi.finish=0;startWifiScan();next();next();assert(wifiScanDone&&WiFi.starts==1&&wifiCount==0);
 reset();WiFi.finish=WIFI_SCAN_RUNNING;startWifiScan();next();clockNow+=6000;tickWifiScan();assert(!wifiScanRunning&&!wifiScanDone);assert(!(WiFiGenericClass::bits&WIFI_SCANNING_BIT));next();clockNow+=6000;tickWifiScan();assert(wifiScanDone&&WiFi.starts==2);
 std::cout<<"PASS actual scan runtime: connected preserved, offline attempt stopped, single pass, R debounce, cancel clears flags, bounded failure/timeout retry, sleep restored\n";
}

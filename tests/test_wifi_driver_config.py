"""Execute the actual vendored scanNetworks function with deterministic uninitialized stack fill.
Old wrapper is a negative control: home_chan_dwell_time remains 0xAA.
"""
from pathlib import Path
import subprocess,tempfile
root=Path(__file__).resolve().parents[1]
s=(root/'lib/WiFi/src/WiFiScan.cpp').read_text();start=s.index('int16_t WiFiScanClass::scanNetworks(');end=s.index('\n\n\n/**',start)
body=s[start:end]
pre=r'''
#include <cstdint>
#include <cstdlib>
#define WIFI_SCAN_RUNNING -1
#define WIFI_SCAN_FAILED -2
#define WIFI_SCANNING_BIT 1
#define WIFI_SCAN_DONE_BIT 2
#define WIFI_SCAN_TYPE_PASSIVE 1
#define WIFI_SCAN_TYPE_ACTIVE 0
#define ESP_OK 0
struct wifi_scan_config_t {uint8_t*ssid;uint8_t*bssid;uint8_t channel;bool show_hidden;int scan_type;struct {uint32_t passive;struct{uint32_t min,max;}active;}scan_time;uint8_t home_chan_dwell_time;};
struct WiFiGenericClass {static int getStatusBits(){return 0;}static void clearStatusBits(int){}static void setStatusBits(int){}static bool waitStatusBits(int,int){return true;}};
struct {void enableSTA(bool){}} WiFi;
struct WiFiScanClass {static uint32_t _scanTimeout,_scanStarted;static bool _scanAsync;static uint16_t _scanCount;static void scanDelete(){};int16_t scanNetworks(bool,bool,bool,uint32_t,uint8_t,const char*,const uint8_t*);};
uint32_t WiFiScanClass::_scanTimeout=0,WiFiScanClass::_scanStarted=0;bool WiFiScanClass::_scanAsync=false;uint16_t WiFiScanClass::_scanCount=0;
uint32_t millis(){return 100;}
int esp_wifi_scan_start(wifi_scan_config_t*c,bool){
 if(c->home_chan_dwell_time!=30)std::exit(42);
 if(c->channel!=0||!c->show_hidden||c->ssid||c->bssid||c->scan_time.active.min!=100||c->scan_time.active.max!=200)std::exit(43);
 return ESP_OK;
}
'''
post='\nint main(){WiFiScanClass s;return s.scanNetworks(true,true,false,200,0,nullptr,nullptr)==WIFI_SCAN_RUNNING?0:44;}\n'
with tempfile.TemporaryDirectory() as tmp:
 for old in [False,True]:
  code=body
  if old:code=code.replace('wifi_scan_config_t config = {};\n    config.home_chan_dwell_time = 30;','wifi_scan_config_t config;')
  p=Path(tmp)/('old.cpp' if old else 'fixed.cpp');p.write_text(pre+code+post)
  exe=p.with_suffix('');subprocess.run(['c++','-std=c++11','-O0','-ftrivial-auto-var-init=pattern',str(p),'-o',str(exe)],check=True)
  status=subprocess.run([str(exe)]).returncode
  assert status==(42 if old else 0),(old,status)
print('PASS actual WiFiScan.cpp: fixed configuration initialized; old wrapper reproducibly fails negative control (42)')

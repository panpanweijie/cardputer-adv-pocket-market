#pragma once
// Keep Arduino scan state aligned with explicit driver cancellation.
class ScanStatusAccess:public WiFiGenericClass {
 public:static void clearScan(){clearStatusBits(WIFI_SCANNING_BIT|WIFI_SCAN_DONE_BIT);}
};
bool wifiScanRunning=false,wifiScanSession=false;
unsigned long wifiScanStarted=0;
unsigned wifiScanPass=0;
bool wifiScanOldAuto=true;
wifi_ps_type_t wifiScanOldSleep=WIFI_PS_MIN_MODEM;
String wifiScanError;
void cancelWifiScan(){
    if(wifiScanRunning){esp_wifi_scan_stop();ScanStatusAccess::clearScan();}
    WiFi.scanDelete();wifiScanRunning=false;wifiScanDone=true;
    if(wifiScanSession){WiFi.setSleep(wifiScanOldSleep);WiFi.setAutoReconnect(wifiScanOldAuto);wifiScanSession=false;}
}
void startWifiScan(){
    if(currentState==STATE_WIFI_SCAN&&!wifiScanDone)return;
    if(!wifiScanSession){wifiScanOldAuto=WiFi.getAutoReconnect();wifiScanOldSleep=WiFi.getSleep();wifiScanSession=true;}
    WiFi.setAutoReconnect(false);
    WiFi.mode(WIFI_STA);WiFi.setSleep(false);
    // Preserve an established association; only cancel offline connection attempts.
    if(WiFi.status()!=WL_CONNECTED){WiFi.disconnect(false,false);lastReconnect=0;}
    WiFi.scanDelete();wifiScanPass=0;wifiScanRunning=false;wifiScanStarted=millis();
    currentState=STATE_WIFI_SCAN;wifiCount=0;wifiCursor=0;wifiSSIDs.clear();wifiRSSIs.clear();wifiScanDone=false;wifiScanError="";
    showWifiScan();
}
void tickWifiScan(){
    if(currentState!=STATE_WIFI_SCAN||wifiScanDone)return;
    if(!wifiScanRunning){
        if(millis()-wifiScanStarted<300||prefetchBusy)return;
        int n=WiFi.scanNetworks(true,true,false,200);++wifiScanPass;
        Serial.printf("WiFi scan start: result=%d, state=%d, attempt=%u\n",n,WiFi.status(),wifiScanPass);
        if(n==WIFI_SCAN_FAILED){
            // A failed start can leave a previous scan flag set; clear only this scanner state.
            esp_wifi_scan_stop();ScanStatusAccess::clearScan();WiFi.scanDelete();
            wifiScanStarted=millis();wifiScanError="Scan failed. R to retry";
            if(wifiScanPass>=2){wifiScanDone=true;WiFi.setSleep(wifiScanOldSleep);}showWifiScan();return;
        }
        wifiScanRunning=true;showWifiScan();return;
    }
    int n=WiFi.scanComplete();
    if(n==WIFI_SCAN_RUNNING&&millis()-wifiScanStarted<5500)return;
    if(n<0){
        esp_wifi_scan_stop();ScanStatusAccess::clearScan();WiFi.scanDelete();wifiScanRunning=false;wifiScanStarted=millis();
        wifiScanError="Scan timeout. R to retry";
        if(wifiScanPass>=2){wifiScanDone=true;WiFi.setSleep(wifiScanOldSleep);}showWifiScan();return;
    }
    Serial.printf("WiFi scan completed: %d APs\n",n);
    std::vector<WifiScanEntry> entries;
    for(int i=0;i<n;++i)entries.push_back({WiFi.SSID(i).c_str(),WiFi.RSSI(i)});
    entries=compactWifiScan(entries);wifiSSIDs.clear();wifiRSSIs.clear();
    for(const auto&e:entries){wifiSSIDs.push_back(e.ssid.c_str());wifiRSSIs.push_back(e.rssi);}
    wifiCount=wifiSSIDs.size();wifiCursor=0;WiFi.scanDelete();wifiScanRunning=false;wifiScanDone=true;
    wifiScanError=wifiCount?"":"No networks. R to retry";WiFi.setSleep(wifiScanOldSleep);showWifiScan();
}

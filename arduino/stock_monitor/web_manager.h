#pragma once
#include "phone_page.h"
#include "stock_aliases.h"
WebServer phoneServer(80);
bool phoneServerStarted=false;
bool phoneRoutesReady=false;
String phoneToken;
String phoneSavedStatus;

void phoneJson(int status,const String& body){
    phoneServer.sendHeader("Cache-Control","no-store");
    phoneServer.sendHeader("X-Content-Type-Options","nosniff");
    phoneServer.send(status,"application/json; charset=utf-8",body);
}
void phoneError(int status,const String& error){
    DynamicJsonDocument doc(512);doc["error"]=error;String out;serializeJson(doc,out);phoneJson(status,out);
}
void initializePhoneRoutes(){
    if(phoneRoutesReady)return;phoneRoutesReady=true;
    phoneServer.on("/",HTTP_GET,[]{
        phoneServer.sendHeader("Cache-Control","no-store");
        phoneServer.sendHeader("X-Frame-Options","DENY");
        phoneServer.send_P(200,"text/html; charset=utf-8",PHONE_PAGE);
    });
    phoneServer.on("/api/watchlist",HTTP_GET,[]{
        String list=prefs.getString("list","");
        // Read current runtime list, including the initial defaults before first save.
        list="";for(int i=0;i<stockCount;++i){if(i)list+=",";list+=stockCodes[i];}
        // Names reuse the quote cache / curated aliases; opening the page makes no extra quote requests.
        size_t capacity=512+list.length()*2+JSON_OBJECT_SIZE(stockCount);
        for(int i=0;i<stockCount;++i)capacity+=stockCodes[i].length()+stocks[i].displayName.length()+stocks[i].name.length()+128;
        DynamicJsonDocument doc(capacity);doc["symbols"]=list;doc["csrf"]=phoneToken;
        JsonObject names=doc.createNestedObject("names");
        for(int i=0;i<stockCount;++i){
            String name=stocks[i].displayName;
            for(const auto& alias:STOCK_ALIASES)if(stockCodes[i]==alias.code){name=alias.name;break;}
            if(!name.length() && stocks[i].name!="Loading...")name=stocks[i].name;
            if(name.length() && name!=stockCodes[i])names[stockCodes[i]]=name;
        }
        if(doc.overflowed()){phoneError(503,"名称读取失败，请重试");return;}
        String out;serializeJson(doc,out);phoneJson(200,out);
    });
    phoneServer.on("/api/watchlist",HTTP_POST,[]{
        if(currentState!=STATE_PHONE_MANAGER || phoneServer.arg("csrf")!=phoneToken){phoneError(403,"请重新扫码后保存");return;}
        if(!phoneServer.hasArg("symbols")){phoneError(400,"缺少股票列表");return;}
        String input=phoneServer.arg("symbols");
        String previous="";for(int i=0;i<stockCount;++i){if(i)previous+=",";previous+=stockCodes[i];}
        if(!loadStocksFromString(input)){phoneError(400,watchlistError);return;}
        String canonical="";for(int i=0;i<stockCount;++i){if(i)canonical+=",";canonical+=stockCodes[i];}
        size_t written=prefs.putString("list",canonical);
        if(canonical.length()&&written!=canonical.length()){
            loadStocksFromString(previous);phoneError(507,"设备存储空间不足，列表未保存");return;
        }
        hasQuoteFetch=false;
        phoneSavedStatus="Saved "+String(stockCount)+" stocks";
        DynamicJsonDocument doc(256);doc["count"]=stockCount;doc["ok"]=true;String out;serializeJson(doc,out);phoneJson(200,out);
        showPhoneManager();
    });
    phoneServer.onNotFound([]{phoneServer.send(404,"text/plain","Not found");});
}
void startPhoneManager(){
    currentState=STATE_PHONE_MANAGER;phoneSavedStatus="";
    if(WiFi.status()==WL_CONNECTED){
        initializePhoneRoutes();phoneToken=String((uint32_t)esp_random(),HEX)+String((uint32_t)esp_random(),HEX);
        if(!phoneServerStarted){phoneServer.begin();phoneServerStarted=true;}
    }
    showPhoneManager();
}
void stopPhoneManager(){
    if(phoneServerStarted)phoneServer.stop();
    phoneServerStarted=false;phoneToken="";currentState=phoneReturnState;
    if(currentState==STATE_CHART)showChart();else showHome();
}
void tickPhoneManager(){
    if(currentState!=STATE_PHONE_MANAGER)return;
    if(phoneServerStarted){phoneServer.handleClient();return;}
    if(WiFi.status()==WL_CONNECTED)startPhoneManager();
}

#pragma once
#include <freertos/FreeRTOS.h>
#include <freertos/task.h>
#include <freertos/queue.h>
#include <new>
#include "premarket_cache_policy.h"
// Worker owns HTTP only. It never touches stocks, the screen, preferences or clock.
struct QuotePrefetchJob{
 uint32_t generation=0,started=0;
 std::vector<int> indices;std::vector<String> codes;
 String url,payload,date,marketLine,requestCode;PreQuote pre;QueueHandle_t destination=nullptr;bool ok=false,premarket=false;
};
QueueHandle_t quotePrefetchResults=nullptr;
void quotePrefetchWorker(void* arg){
 auto*job=static_cast<QuotePrefetchJob*>(arg);
 if(job->premarket){
  WiFiClientSecure client;client.setInsecure();client.setHandshakeTimeout(4);client.setTimeout(4);HTTPClient http;
  if(http.begin(client,job->url)){
   prepareHttp(http);http.setConnectTimeout(2500);http.setTimeout(4000);
   int status=http.GET();job->date=http.header("Date");
   if(status==HTTP_CODE_OK&&http.getSize()<=150000){
    const String&code=job->requestCode;DynamicJsonDocument filter(2048),doc(6144);
    filter["code"]=true;filter["data"][code]["pandata"]=true;filter["data"][code]["qt"]["market"][0]=true;
    auto error=deserializeJson(doc,http.getStream(),DeserializationOption::Filter(filter));
    if(!error&&doc["code"].as<int>()==0&&doc["data"][code].is<JsonObject>()){
     job->pre=decodeTencentPre(doc["data"][code]["pandata"]);job->marketLine=doc["data"][code]["qt"]["market"][0]|"";job->ok=true;
    }
   }
   http.end();
  }
 }else{
  WiFiClient client;HTTPClient http;
  if(http.begin(client,job->url)){
   prepareHttp(http);http.setConnectTimeout(2000);http.setTimeout(3000);
   int status=http.GET();job->date=http.header("Date");
   if(status==HTTP_CODE_OK&&http.getSize()<=24576){job->payload=http.getString();job->ok=job->payload.length()>0&&job->payload.length()<=24576;}
   http.end();
  }
 }
 xQueueSend(job->destination,&job,portMAX_DELAY);
 vTaskDelete(nullptr);
}
void tickQuotePrefetch(){
 static uint32_t lastStart=0,lastInput=0;static int cursor=0;static bool lastWasPre=false;
 const uint32_t now=millis();
 if(M5Cardputer.Keyboard.isPressed()||M5Cardputer.BtnA.isPressed()||homeKeysHeld)lastInput=now;
 QuotePrefetchJob*done=nullptr;
 if(quotePrefetchResults&&xQueueReceive(quotePrefetchResults,&done,0)==pdTRUE){
  prefetchBusy=false;bool visibleChanged=false;
  if(done->generation==watchlistGeneration){
   int64_t stamp=parseHttpDate(done->date.c_str());if(stamp>1700000000){serverClock=stamp;serverClockMillis=now;}
   if(done->premarket){
    int i=done->indices[0];
    if(i>=0&&i<stockCount&&stockCodes[i]==done->codes[0]&&int32_t(stocks[i].preAttempt-done->started)<=0&&preMarketTime(stockCodes[i].c_str(),marketNow())){
     auto&stock=stocks[i];bool accepted=done->ok&&acceptPreload(stockCodes[i].c_str(),done->pre,marketNow())&&(!stock.pre.valid||done->pre.sessionStart!=stock.pre.sessionStart||done->pre.timestamp>=stock.pre.timestamp);
     stock.preError=!accepted;stock.preErrorText=accepted?"":"PRE LOAD RETRY";
     if(accepted){stock.pre=done->pre;stock.regularStart=stock.pre.sessionEnd;stock.regularEnd=stock.regularStart+6*3600+30*60;}
     MarketFeed next;if(done->ok&&decodeMarketFeed(done->marketLine.c_str(),next)&&freshMarketFeed(next,marketNow())){marketFeed=next;marketFeedOk=true;}
     visibleChanged=i>=homeScrollOffset&&i<homeScrollOffset+3;
    }
   }else if(done->ok){
   const std::string raw=done->payload.c_str();
   for(size_t n=0;n<done->indices.size();++n){int i=done->indices[n];
    if(i<0||i>=stockCount||stockCodes[i]!=done->codes[n])continue;
    if(stocks[i].valid&&int32_t(stocks[i].quoteFetchedAt-done->started)>0)continue; // Never overwrite a newer manual refresh.
    SummaryQuote q;if(decodeSummary(raw,summaryCode(done->codes[n].c_str()),q)){applySummaryQuote(i,q);if(i>=homeScrollOffset&&i<homeScrollOffset+3)visibleChanged=true;}
   }
   }
  }
  delete done;
  if(visibleChanged&&currentState==STATE_HOME&&!displayOff&&!homeKeysHeld&&(!motionEnabled||now-motionMovedAt>900))showHome();
 }
 if(prefetchBusy||currentState!=STATE_HOME||displayOff||WiFi.status()!=WL_CONNECTED||stockCount<1||now-lastInput<1000||now-lastStart<1500||homeKeysHeld||(motionEnabled&&now-motionMovedAt<1000))return;
 int first=homeScrollOffset,end=min(stockCount,first+3);
 if(!visibleQuotesReady(first,end))return; // Visible quotes always have priority.
 if(ESP.getFreeHeap()<85000||ESP.getMaxAllocHeap()<24576)return;
 if(!quotePrefetchResults){quotePrefetchResults=xQueueCreate(1,sizeof(QuotePrefetchJob*));if(!quotePrefetchResults)return;}
 auto*job=new(std::nothrow) QuotePrefetchJob;if(!job)return;
 job->generation=watchlistGeneration;job->started=now;job->destination=quotePrefetchResults;job->url="http://qt.gtimg.cn/q=";
 for(int i:prefetchOrder(stockCount,first,end,cursor)){
  if(cachedQuoteReady(stocks[i].valid,stocks[i].quoteFetchedAt,now,backgroundRefreshMs(fetchInterval)))continue;
  if(stocks[i].quoteAttemptAt&&now-stocks[i].quoteAttemptAt<backgroundRefreshMs(fetchInterval))continue;
  job->indices.push_back(i);job->codes.push_back(stockCodes[i]);
  if(job->indices.size()>1)job->url+=",";job->url+=summaryCode(stockCodes[i].c_str()).c_str();
  if(job->indices.size()==3)break;
 }
 // Visible pre-market quotes first, then alternate adjacent preloads with summary batches.
 int preIndex=-1;bool visiblePreDue=false;
 std::vector<int> order;for(int i=first;i<end;++i)order.push_back(i);
 for(int i:prefetchOrder(stockCount,first,end,cursor))order.push_back(i);
 for(int i:order)if(premarketDue(stockCodes[i].c_str(),stocks[i].pre,marketNow(),stocks[i].preRequested,stocks[i].preAttempt,stocks[i].preError,now,(i>=first&&i<end)?fetchInterval:backgroundRefreshMs(fetchInterval))){
  preIndex=i;visiblePreDue=i>=first&&i<end;break;
 }
 if(preIndex>=0&&(visiblePreDue||!lastWasPre||job->indices.empty())&&ESP.getFreeHeap()>=135000&&ESP.getMaxAllocHeap()>=49152){
  job->premarket=true;job->indices.clear();job->codes.clear();job->indices.push_back(preIndex);job->codes.push_back(stockCodes[preIndex]);
  job->requestCode=premarketRequestCode(stockCodes[preIndex].c_str(),stocks[preIndex].exchangeCode.c_str()).c_str();
  job->url="https://web.ifzq.gtimg.cn/appstock/app/usMinute/query?code="+job->requestCode;
 }
 if(job->indices.empty()){delete job;return;}
 lastStart=now;prefetchBusy=true;
 for(int i:job->indices){if(job->premarket){stocks[i].preAttempt=now;stocks[i].preRequested=true;}else stocks[i].quoteAttemptAt=now;}
 lastWasPre=job->premarket;
 cursor=(job->indices.back()+1)%stockCount;
 if(xTaskCreate(quotePrefetchWorker,"quote-preload",job->premarket?12288:8192,job,1,nullptr)!=pdPASS){prefetchBusy=false;delete job;}
}

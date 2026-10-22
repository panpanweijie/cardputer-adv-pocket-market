#pragma once
void tickMarketFeed(){
 if(stockCount==0||(currentState!=STATE_HOME&&currentState!=STATE_CHART)||WiFi.status()!=WL_CONNECTED)return;
 bool crossed=false;
 if(marketFeed.timestamp&&stockCount>0){
  int i=currentState==STATE_CHART?currentChartIdx:homeScrollOffset;
  if(i>=0&&i<stockCount)crossed=scheduledSession(stockCodes[i].c_str(),marketNow()).phase!=scheduledSession(stockCodes[i].c_str(),marketFeed.timestamp).phase;
 }
 const unsigned long interval=crossed?15000UL:marketFeedOk?120000UL:30000UL;
 if(marketFeedAttempt&&millis()-marketFeedAttempt<interval)return;
 marketFeedAttempt=millis();marketFeedOk=false;
 WiFiClientSecure client;client.setInsecure();client.setHandshakeTimeout(5);client.setTimeout(5);HTTPClient http;
 // A single benchmark returns all cash-market states, including holiday reasons.
 if(!http.begin(client,"https://web.ifzq.gtimg.cn/appstock/app/minute/query?code=sh000001"))return;
 prepareHttp(http);int status=http.GET();syncHttpClock(http);
 if(status==HTTP_CODE_OK&&http.getSize()<=150000){
  DynamicJsonDocument filter(512);filter["code"]=true;filter["data"]["sh000001"]["qt"]["market"][0]=true;
  DynamicJsonDocument doc(6144);auto error=deserializeJson(doc,http.getStream(),DeserializationOption::Filter(filter));
  if(!error&&doc["code"].as<int>()==0){
   const char* line=doc["data"]["sh000001"]["qt"]["market"][0]|"";
   MarketFeed next;if(decodeMarketFeed(line,next)&&freshMarketFeed(next,marketNow())){marketFeed=next;marketFeedOk=true;}
  }
 }
 http.end();
 if(currentState==STATE_HOME)showHome();else showChart();
}

#pragma once
// ESC always pops one visible page; HOME is the root, with motion as its child mode.
void navigateBack(){
 switch(currentState){
  case STATE_DELETE_CONFIRM:cancelDeleteStock();return;
  case STATE_SEARCH:leaveSearch();return;
  case STATE_PHONE_MANAGER:stopPhoneManager();return;
  case STATE_WIFI_INPUT:currentState=STATE_WIFI_SCAN;showWifiScan();return;
  case STATE_WIFI_SCAN:cancelWifiScan();resumeWifi();currentState=STATE_SETTINGS;showSettings();return;
  case STATE_SETTINGS:currentState=STATE_HOME;resumeWifi();showHome();return;
  case STATE_EDIT:currentState=STATE_HOME;showHome();return;
  case STATE_CHART:currentState=STATE_HOME;showHome();return;
  case STATE_HOME:
   if(motionFeatureAvailable&&motionEnabled){int selected=motionSelected;toggleMotion();homeScrollOffset=max(0,min(selected,stockCount-3));showHome();}
   return;
 }
}

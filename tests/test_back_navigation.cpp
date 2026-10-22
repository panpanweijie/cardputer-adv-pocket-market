#include <cassert>
#include <algorithm>
using std::max;using std::min;
enum AppState {STATE_HOME,STATE_CHART,STATE_EDIT,STATE_SETTINGS,STATE_WIFI_SCAN,STATE_WIFI_INPUT,STATE_PHONE_MANAGER,STATE_SEARCH,STATE_DELETE_CONFIRM};
AppState currentState=STATE_HOME,searchReturnState=STATE_HOME,phoneReturnState=STATE_HOME;
bool motionFeatureAvailable=true,motionEnabled=false,wifiScanRunning=true;
int motionSelected=12,homeScrollOffset=0,stockCount=22,homeDraws=0,chartDraws=0,scans=0,resumes=0;
struct {void scanDelete(){++scans;}}WiFi;
void showHome(){++homeDraws;}void showChart(){++chartDraws;}
void showWifiScan(){}void showSettings(){}void resumeWifi(){++resumes;}
void toggleMotion(){motionEnabled=!motionEnabled;}
void cancelDeleteStock(){currentState=STATE_CHART;}
void leaveSearch(){currentState=searchReturnState;}
void stopPhoneManager(){currentState=phoneReturnState;}
#include "back_navigation.h"
int main(){
 currentState=STATE_HOME;motionEnabled=true;navigateBack();assert(!motionEnabled&&currentState==STATE_HOME&&homeScrollOffset==12);
 navigateBack();assert(currentState==STATE_HOME&&!motionEnabled);
 currentState=STATE_CHART;motionEnabled=true;navigateBack();assert(currentState==STATE_HOME&&motionEnabled);navigateBack();assert(!motionEnabled);
 currentState=STATE_DELETE_CONFIRM;navigateBack();assert(currentState==STATE_CHART);
 for(AppState origin:{STATE_HOME,STATE_CHART}){
  currentState=STATE_SEARCH;searchReturnState=origin;navigateBack();assert(currentState==origin);
  currentState=STATE_PHONE_MANAGER;phoneReturnState=origin;navigateBack();assert(currentState==origin);
 }
 currentState=STATE_WIFI_INPUT;navigateBack();assert(currentState==STATE_WIFI_SCAN);navigateBack();assert(currentState==STATE_SETTINGS&&!wifiScanRunning&&scans==1);navigateBack();assert(currentState==STATE_HOME&&resumes==1);
 currentState=STATE_EDIT;navigateBack();assert(currentState==STATE_HOME);
 stockCount=0;motionEnabled=true;navigateBack();assert(!motionEnabled&&homeScrollOffset==0);
 motionFeatureAvailable=false;motionEnabled=false;navigateBack();assert(currentState==STATE_HOME);
}

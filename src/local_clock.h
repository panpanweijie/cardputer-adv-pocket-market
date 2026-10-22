#pragma once
#include "market_sessions.h"
#include <string>
#include <cstdio>
constexpr int LOCAL_ZONE_COUNT=5;
inline const char* localZoneName(int zone){const char*names[]={"China/HK","New York","Tokyo","London","UTC"};return names[zone>=0&&zone<LOCAL_ZONE_COUNT?zone:0];}
inline int localZoneOffset(int zone,int64_t utc){
 if(zone==1)return nyOffsetUtc(utc);
 if(zone==2)return 9*3600;
 if(zone==3){time_t t=utc;tm d;gmtime_r(&t,&d);int year=d.tm_year+1900;auto lastSunday=[&](int month,int days){int dow=int((civilDays(year,month,days)+4)%7);return days-dow;};int64_t start=civilDays(year,3,lastSunday(3,31))*86400+3600,end=civilDays(year,10,lastSunday(10,31))*86400+3600;return utc>=start&&utc<end?3600:0;}
 return zone==4?0:8*3600;
}
inline std::string localClockText(int zone,int64_t utc){
 const char*tags[]={"BJT","NY","JST","LDN","UTC"};if(zone<0||zone>=LOCAL_ZONE_COUNT)zone=0;
 if(utc<1700000000)return std::string(tags[zone])+" --:--:--";
 time_t t=utc+localZoneOffset(zone,utc);tm d;gmtime_r(&t,&d);char out[24];std::snprintf(out,sizeof(out),"%s %02d:%02d:%02d",tags[zone],d.tm_hour,d.tm_min,d.tm_sec);return out;
}

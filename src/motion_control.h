#pragma once
#include "motion_keys.h"
void toggleMotion(){
 if(!motionFeatureAvailable)return;
 if(!motionEnabled&&!M5.Imu.isEnabled()){motionStatus="IMU unavailable";return;}
 motionEnabled=!motionEnabled;prefs.putBool("motion",motionEnabled);motionStatus="";
 motionSelected=max(0,min(homeScrollOffset,stockCount-1));motionDeck.reset(motionSelected,millis());motionMovedAt=millis();
}
void moveMotionSelection(int direction){
 if(!stockCount)return;motionDeck.nudge(direction,stockCount);motionMovedAt=millis();
}
void tickMotion(){
 static MotionKeyHold keyHold;
 static uint32_t sampled=0,drawn=0;static bool onHome=false;static float lastPosition=-100;
 if(!motionFeatureAvailable||!motionEnabled||currentState!=STATE_HOME||displayOff){onHome=false;keyHold.reset();cardKeyBoundaryLatch.reset();cardTiltBoundaryLatch.reset();if(motionDeck.keyboardHeld)motionDeck.endKeyboardSlide();return;}
 if(!onHome){if(motionDeck.ready)motionDeck.reset(max(0,min(motionSelected,stockCount-1)),millis());onHome=true;motionMovedAt=millis();lastPosition=-100;}
 uint32_t now=millis();const auto&keys=M5Cardputer.Keyboard.keysState();int direction=motionKeyDirection(keys);
 auto command=keyHold.update(direction,now);
 if(command.endedSlide)motionDeck.endKeyboardSlide();
 if(command.tap){motionDeck.velocity=0;moveMotionSelection(command.tap);}
 motionDeck.keyboardHeld=direction!=0;motionDeck.keyboardSpeed=command.speed;
 if(direction)motionMovedAt=now; // Defer synchronous quote requests until the key is released.
 if(now-sampled<20)return;sampled=now;
 if(!M5.Imu.isEnabled()){motionEnabled=false;motionStatus="IMU unavailable";showHome();return;}
 if(!(M5.Imu.update()&m5::IMU_Class::sensor_mask_accel))return;
 const auto data=M5.Imu.getImuData();
 float before=motionDeck.position;
 if(motionDeck.update(data.accel.x,data.accel.y,data.accel.z,data.gyro.x,data.gyro.y,data.gyro.z,now,stockCount))motionMovedAt=now;
 int crossed=cyclicCrossing(before,motionDeck.position,stockCount);
 if(crossed)boundaryPulse.trigger(crossed,now);
 // Periodically normalize both visual position and spring target together.
 if(stockCount>1&&std::fabs(motionDeck.position)>1000){float shift=std::floor(motionDeck.position/stockCount)*stockCount;motionDeck.position-=shift;motionDeck.manualTarget-=shift;lastPosition=-100;}
 motionSelected=motionDeck.selected(stockCount);homeScrollOffset=max(0,min(motionSelected-1,stockCount-3));
 if(now-drawn>=33&&std::fabs(motionDeck.position-lastPosition)>.001f){drawn=now;lastPosition=motionDeck.position;showHome();}
}

#include <cassert>
#include <string>
#include <vector>
#include "motion_deck.h"
using String=std::string;using std::min;using std::max;
namespace m5 {struct IMU_Class{enum{sensor_mask_accel=1};};}
struct ImuData{struct XYZ{float x=0,y=0,z=0;}accel,gyro;};
struct {struct {bool available=true;int reads=0;bool isEnabled(){return available;}int update(){++reads;return 1;}ImuData getImuData(){ImuData d;d.accel.z=1;return d;}}Imu;}M5;
struct {struct {struct Keys{bool left=false,right=false;std::vector<char>word;}keys;const Keys&keysState(){return keys;}}Keyboard;}M5Cardputer;
constexpr bool motionFeatureAvailable=true;
bool motionEnabled=false,displayOff=false;int motionSelected=0,homeScrollOffset=0,stockCount=5;
uint32_t motionMovedAt=0,now=1000;unsigned long millis(){return now;}
enum{STATE_HOME,STATE_CHART};int currentState=STATE_HOME;MotionDeck motionDeck;String motionStatus;
struct {bool saved=false;void putBool(const char*,bool value){saved=value;}}prefs;
void showHome(){}
#include "motion_control.h"
void frames(int n){for(int i=0;i<n;++i){now+=20;tickMotion();}}
int main(){
 toggleMotion();assert(motionEnabled&&prefs.saved);frames(1);assert(motionDeck.ready);
 moveMotionSelection(-1);frames(60);assert(motionSelected==0);
 moveMotionSelection(1);frames(100);assert(motionSelected==1&&homeScrollOffset==0);
 moveMotionSelection(1);moveMotionSelection(1);moveMotionSelection(1);moveMotionSelection(1);frames(130);assert(motionSelected==4&&homeScrollOffset==2);
 M5Cardputer.Keyboard.keys.left=true;frames(1);frames(10);assert(motionDeck.manualTarget==3); // one short tap, no repeat before threshold
 M5Cardputer.Keyboard.keys.left=false;frames(90);assert(motionSelected==3);
 M5Cardputer.Keyboard.keys.left=true;frames(100);assert(motionSelected==0); // accelerates to the first card
 M5Cardputer.Keyboard.keys.left=false;frames(90);assert(motionSelected==0&&std::fabs(motionDeck.velocity)<.04f);
 M5Cardputer.Keyboard.keys.right=true;frames(50);M5Cardputer.Keyboard.keys.left=true;frames(80);assert(std::fabs(motionDeck.velocity)<.04f); // both keys stop
 M5Cardputer.Keyboard.keys.right=false;M5Cardputer.Keyboard.keys.left=false;
 displayOff=true;int reads=M5.Imu.reads;frames(10);assert(reads==M5.Imu.reads);displayOff=false;frames(1);assert(motionDeck.ready);
 currentState=STATE_CHART;reads=M5.Imu.reads;frames(10);assert(reads==M5.Imu.reads);
 toggleMotion();assert(!motionEnabled&&!prefs.saved);M5.Imu.available=false;toggleMotion();assert(!motionEnabled&&motionStatus=="IMU unavailable");
 stockCount=0;moveMotionSelection(1);
}

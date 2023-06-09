#include <Wire.h>
#include <RotaryEncoder.h>
#include "SSD1306.h"

// Define Run/SET pin
const int PIN_RUN = 23;

// Define trigger pin
const int PIN_TRIG = 26;

// Define OLED display pins
const int OLED_Addr = 0x3c;
const int SDAPin = 21;
const int SCLPin = 22;

// Define encoder pins
const int PIN_CH0_A = 16;
const int PIN_CH0_B =  5;
const int PIN_CH1_A = 18;
const int PIN_CH1_B = 19;
const int PIN_RPT_A = 15;
const int PIN_RPT_B =  4;

// Define duration controller
struct DurationController {
  int minVal;
  int maxVal;
  int lastPos;
  int val;
  RotaryEncoder encoder;
};

// Prepare the operation channel variable
int currentChannel;
int cycleNo;
int leftTime_ms_Ch0;
int leftTime_ms_Ch1;
int initialTime;

// Create a display instance
SSD1306 display(OLED_Addr, SDAPin, SCLPin);

// Create rotary encoder instances
RotaryEncoder encoder_ch0(PIN_CH0_A, PIN_CH0_B, RotaryEncoder::LatchMode::TWO03);
RotaryEncoder encoder_ch1(PIN_CH1_A, PIN_CH1_B, RotaryEncoder::LatchMode::TWO03);
RotaryEncoder encoder_rpt(PIN_RPT_A, PIN_RPT_B, RotaryEncoder::LatchMode::TWO03);

// Create duration controller instances
struct DurationController durationCtrl_Ch0 = {1, 86400, -1, 0, encoder_ch0};
struct DurationController durationCtrl_Ch1 = {1, 86400, -1, 0, encoder_ch1};
struct DurationController durationCtrl_Rpt = {0, 1024, -1, 0, encoder_rpt};

// Duration controller function
void encoderProc(DurationController &ctrl) {
  ctrl.encoder.tick();
  int newPos = ctrl.encoder.getPosition();
  if (newPos < ctrl.minVal) {
    ctrl.encoder.setPosition(ctrl.minVal);
    newPos = ctrl.minVal;
  } else if (newPos > ctrl.maxVal) {
    ctrl.encoder.setPosition(ctrl.maxVal);
    newPos = ctrl.maxVal;
  }
  if (ctrl.lastPos != newPos) {
    ctrl.val = newPos;
    ctrl.lastPos = newPos;
  }
}

void setup() {

  // Set pin modes
  pinMode(PIN_CH0_A, INPUT_PULLUP);
  pinMode(PIN_CH0_B, INPUT_PULLUP);
  pinMode(PIN_CH1_A, INPUT_PULLUP);
  pinMode(PIN_CH1_B, INPUT_PULLUP);
  pinMode(PIN_RPT_A, INPUT_PULLUP);
  pinMode(PIN_RPT_B, INPUT_PULLUP);
  pinMode(PIN_RUN, INPUT);
  pinMode(PIN_TRIG, OUTPUT);
  digitalWrite(PIN_TRIG, LOW);

  // Initialize rotary encoder position
  durationCtrl_Ch0.encoder.setPosition(60);
  durationCtrl_Ch1.encoder.setPosition(60);
  durationCtrl_Rpt.encoder.setPosition(0);

  // Initialize display
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Starting...");
  display.display();

  // Initialize running parameters
  currentChannel = -1;
  cycleNo = -1;
  durationCtrl_Ch0.val = 60;
  durationCtrl_Ch1.val = 60;
  leftTime_ms_Ch0 = durationCtrl_Ch0.val;
  leftTime_ms_Ch1 = durationCtrl_Ch1.val;
  initialTime = millis();

  // Serial connection  
  Serial.begin(115200);
  Serial.println("Started");
  
}
 
void loop() {
  
  if (digitalRead(PIN_RUN)) {

    if (currentChannel == -1) {

      // Initial setup after trigger
      digitalWrite(PIN_TRIG, LOW);
      currentChannel = 0;
      cycleNo = 1;
      leftTime_ms_Ch0 = durationCtrl_Ch0.val * 1000;
      leftTime_ms_Ch1 = durationCtrl_Ch1.val * 1000;
      initialTime = millis();
    
    } else if (currentChannel == 0) {

      // Ch0 operations
      digitalWrite(PIN_TRIG, LOW);
      leftTime_ms_Ch0 = initialTime + durationCtrl_Ch0.val * 1000 - millis();
      if (leftTime_ms_Ch0 <= 0) {
        currentChannel = 1;
        leftTime_ms_Ch0 = durationCtrl_Ch0.val * 1000;
        initialTime = millis();
      }
      
    } else if (currentChannel == 1) {

      // Ch1 operations
      digitalWrite(PIN_TRIG, HIGH);
      leftTime_ms_Ch1 = initialTime + durationCtrl_Ch1.val * 1000 - millis();
      if (leftTime_ms_Ch1 <= 0) {
        cycleNo += 1;
        currentChannel = 0;
        leftTime_ms_Ch1 = durationCtrl_Ch1.val * 1000;
        initialTime = millis();
      }
          
    }

    // Display current state
    display.clear();
    if ((durationCtrl_Rpt.val != 0) and (durationCtrl_Rpt.val < cycleNo)) {
      digitalWrite(PIN_TRIG, LOW);
      currentChannel = -2;
      display.drawString(0, 0, "[FIN] x" + String(durationCtrl_Rpt.val));
      display.drawString(0, 21, "Ch0: " + String(durationCtrl_Ch0.val) + " s");
      display.drawString(0, 42, "Ch1: " + String(durationCtrl_Ch1.val) + " s");
    } else {
      display.drawString(0, 0, "[RUN] " + String(cycleNo) + " / " + String(durationCtrl_Rpt.val));
      display.drawString(0, 21, "Ch0: " + String(leftTime_ms_Ch0 / 1000) + " s");
      display.drawString(0, 42, "Ch1: " + String(leftTime_ms_Ch1 / 1000) + " s");    
    } 
    display.display();

  } else {

    // Initialize running parameters
    digitalWrite(PIN_TRIG, LOW);
    currentChannel = -1;
    cycleNo = -1;
    leftTime_ms_Ch0 = durationCtrl_Ch0.val;
    leftTime_ms_Ch1 = durationCtrl_Ch0.val;
    initialTime = millis();

    // Enable rotary encoders
    encoderProc(durationCtrl_Ch0);
    encoderProc(durationCtrl_Ch1);
    encoderProc(durationCtrl_Rpt); 

    // Display settings
    display.clear();
    display.drawString(0, 0, "[SET] x" + String(durationCtrl_Rpt.val));
    display.drawString(0, 21, "Ch0: " + String(durationCtrl_Ch0.val) + " s");
    display.drawString(0, 42, "Ch1: " + String(durationCtrl_Ch1.val) + " s");
    display.display();
  
  }

}

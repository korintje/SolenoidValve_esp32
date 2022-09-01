// Include modules
#include <Wire.h>
#include "SSD1306.h"

// Global constants
const int trigerPin = 4;
const int relayPins[] = {25, 33, 32}; 
const int togglePins[] = {18, 35, 34}; 
const int OLED_Addr = 0x3c;
const int SDAPin = 21;
const int SCLPin = 22;

// Global variables
SSD1306 display(OLED_Addr, SDAPin, SCLPin);
int currentMode;
int relayStates[] = {LOW, LOW, LOW};


void setup() {

  // Set currentMode to the initial value: 2
  currentMode = 2;

  // Start serial connection
  Serial.begin(115200);
  Serial.println("-- Starting setup ---");

  // Set pin mode
  pinMode(trigerPin, INPUT);
  for (int i = 0; i <= 2; i++) {
    pinMode(relayPins[i], OUTPUT);
    pinMode(togglePins[i], INPUT);  
  }

  // Initialize display
  display.init();
  display.flipScreenVertically();
  display.setFont(ArialMT_Plain_16);
  display.drawString(0, 0, "Starting...");
  display.display();

  Serial.println("-- Setup finish ---");
  
}


void loop() {

  int operateMode = digitalRead(trigerPin);

  Serial.println(operateMode);

  if (operateMode) {

    // Print current mode
    if (currentMode != 1) {
      currentMode = 1;
      Serial.println("Operating mode");
      display.drawString(0, 108, "*a");
      display.display();
    }

    // int states[2];    

    /* --- ↓↓ ここを編集すればバルブをコントロールできます ↓↓ ---- */
    /* --- ↓↓ Edit here to control valve states ↓↓ ---- */

    // (1) valve2（真空ポンプ） のみ2秒OPEN
    set_valves((const int []){LOW, HIGH, LOW});
    delay(2000);

    // (2) valve1（VOC） のみ1秒OPEN
    set_valves((const int []){HIGH, LOW, LOW});
    delay(1000);

    // (3) 全てCLOSE 10秒待機
    set_valves((const int []){LOW, LOW, LOW});
    delay(10000);

    // (4) valve2（真空ポンプ）のみ2秒OPEN
    set_valves((const int []){LOW, HIGH, LOW});
    delay(2000);
        
    // (5) valve3（窒素） のみ1秒OPEN
    set_valves((const int []){LOW, LOW, HIGH});
    delay(1000);
        
    // (6) 全てCLOSE 10秒待機
    set_valves((const int []){LOW, LOW, LOW});
    delay(10000);
    
    /* --- ↑↑ Edit here to control valve states ↑↑ ---- */
    /* --- ↑↑ ここを編集すればバルブをコントロールできます ↑↑ ---- */
    
  } else {

    // Print current mode
    if (currentMode != 0) {
      currentMode = 0;
      Serial.println("Setting mode");
      display.drawString(0, 108, "*m");
      display.display();
    }

    // Toggle to switch valves manually
    for (int i = 0; i <= 2; i++) {
      if (digitalRead(togglePins[i])) {
        set_valve(i, HIGH);
      } else {
        set_valve(i, LOW);
      }    
    }

  } 

}


// Set a valve state (HIGH or LOW)
void set_valve(int valve_id, int valve_state) {
  digitalWrite(relayPins[valve_id], valve_state);
  relayStates[valve_id] = valve_state;
  update_display();
}

// Set valve states by array
void set_valves(const int *valve_states) {
  for (int i = 0; i <= 2; i++) {
    set_valve(i, valve_states[i]);
  }
}

// Update display
void update_display() {
  int _n;
  display.clear();
  _n = display.drawString(0, 0, "V1: ");
  _n = display.drawString(0, 21, "V2: ");
  _n = display.drawString(0, 42, "V3: ");
  for (int i = 0; i <= 2; i++) {
    if (relayStates[i]) {
      _n = display.drawString(32, i * 21, "OPEN");
    } else {
      _n = display.drawString(32, i * 21, "CLOSE");
    }
  }
  if (currentMode == 1) {
    display.drawString(108, 0, "*a");
  } else {
    display.drawString(108, 0, "*m");
  }
  display.display();   
}

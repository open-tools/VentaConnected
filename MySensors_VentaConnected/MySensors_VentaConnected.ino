/*
 MySensors - Venta Connected
 (C) 2017 Reinhold Kainhofer, reinhold@kainhofer.com
 License: ???
*/

#define SKETCH_NAME "VentaConnected"
#define SKETCH_VERSION "0.1"



// Enable debug prints to serial monitor
#define MY_DEBUG 
//#define MY_BAUD_RATE 9600
#define MY_NODE_ID 9
#define MY_RADIO_NRF24                    // Enable and select radio type attached
#define MY_REPEATER_FEATURE               // Enabled repeater feature for this node

#include <SPI.h>
#include <MySensors.h> 


/**************************************
 * OLED display settings
 **************************************/

#include <SSD1306Ascii.h>
#include <SSD1306AsciiAvrI2c.h>
// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C
SSD1306AsciiAvrI2c oled;


/**************************************
//***** MySensors settings
 **************************************/

#define WAITING_TIME 250

// Pin Setup:
// Button & LED Actions (output):
//   - PIN_PWR  D3 ... Power button press
//   - PIN_UD   D4 ... Up/Down button press
//   - PIN_ERR  D8 ... Control Error LED
// Button detection (input, pullup!):
//   - PIN_OPEN D5 ... Case open Button => causes Error
// LED detection (input, pullup):
//   - LED_PWR   D6 ... Power LED 
//   - LED_ERR  D7 ... Error LED
//   - LED_1    A1 ... Level 1 LED
//   - LED_2    A2 ... Level 2 LED
//   - LED_3    A3 ... Level 3 LED

#define PIN_PWR  3
#define PIN_UD   4
#define PIN_OPEN 5
#define LED_PWR  6
#define LED_1    A1
#define LED_2    A2
#define LED_3    A3
#define LED_ERR  7
#define PIN_ERR  8

#define ID_POWER 1
#define ID_LEVEL 2
#define ID_ERROR 3
#define ID_OPEN 4
//#define ID_REPEATER 254



bool power = false;
int level = 0;
bool error = -20;
bool caseopen = false;

bool prev_power = false;
int prev_level = -20;
bool prev_error = -20;
bool prev_caseopen = false;


MyMessage msg_power(ID_POWER, V_STATUS);
MyMessage msg_level(ID_LEVEL, V_LEVEL);
MyMessage msg_error(ID_ERROR, V_STATUS);
MyMessage msg_open(ID_OPEN, V_STATUS);


/**************************************
 ***** Implementation
 **************************************/
void setup() {
  // PIN modes
  pinMode(PIN_PWR, OUTPUT);
  pinMode(PIN_UD,  OUTPUT);
  pinMode(PIN_OPEN,INPUT_PULLUP);
  pinMode(LED_PWR,  INPUT_PULLUP);
  pinMode(LED_1,   INPUT_PULLUP);
  pinMode(LED_2,   INPUT_PULLUP);
  pinMode(LED_3,   INPUT_PULLUP);
  pinMode(LED_ERR, INPUT);
  pinMode(PIN_ERR, OUTPUT);

  // Setup initial states for OUTPUT pins:
  // -) error led needs to be pulled up, otherwise it is on
  // -) All button pins should be low (i.e. no current on transistor base)
  digitalWrite(PIN_ERR, LOW);

  // Setup the interrupts for all relevant pins: Only two interrupt pins => Can't use interrupts!
//  attachInterrupt(digitalPinToInterrupt(LED_PWR), venta_interrupt, CHANGE);
//  attachInterrupt(digitalPinToInterrupt(LED_1), venta_interrupt, CHANGE);
//  attachInterrupt(digitalPinToInterrupt(LED_2), venta_interrupt, CHANGE);
//  attachInterrupt(digitalPinToInterrupt(LED_3), venta_interrupt, CHANGE);
//  attachInterrupt(digitalPinToInterrupt(LED_ERR), venta_interrupt, CHANGE);
//  attachInterrupt(digitalPinToInterrupt(PIN_OPEN), venta_interrupt, CHANGE);


  // Setup the OLED display
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);
}

void before() {
  // Restore the error led override
  digitalWrite(PIN_ERR, loadState(ID_ERROR));  
}

void presentation() {
  present(ID_POWER, S_BINARY, "Venta POWER", true);
  present(ID_LEVEL, S_CUSTOM, "Venta LEVEL", true);
  present(ID_ERROR, S_BINARY, "Venta ERROR", true);
  present(ID_OPEN, S_BINARY, "Venta Case Open", true);
//  present(ID_REPEATER, S_ARDUINO_REPEATER_NODE);

  sendSketchInfo(SKETCH_NAME, SKETCH_VERSION);
}

void loop() {
  readVentaState();

  bool changed = false;
  if (power != prev_power) {
    Serial.println(F("POWER changed from "));Serial.print(prev_power); Serial.print(F(" to ")); Serial.println(power);
    changed = true;
    send(msg_power.set(power));
    prev_power = power;
  }
  if (level != prev_level) {
    Serial.println(F("LEVEL changed from "));Serial.print(prev_level); Serial.print(F(" to ")); Serial.println(level);
    changed = true;
    send(msg_level.set(level));
    prev_level = level;
  }
  if (error != prev_error) {
    Serial.println(F("ERROR changed from "));Serial.print(prev_error); Serial.print(F(" to ")); Serial.println(error);
    changed = true;
    send(msg_error.set(error));
    prev_error = error;
  }
  if (caseopen != prev_caseopen) {
    Serial.println(F("CASEOPEN changed from "));Serial.print(prev_caseopen); Serial.print(F(" to ")); Serial.println(caseopen);
    changed = true;
    send(msg_open.set(caseopen));
    prev_caseopen = caseopen;
  }

updateDisplay();

  wait(WAITING_TIME);
}

void receive(const MyMessage &message) {
  readVentaState();
  switch (message.sensor) {
    
    case ID_POWER:
        if (message.type == V_STATUS) {
          bool newstate = message.getBool();
          Serial.print(F("Incoming message to set power to: "));
          Serial.println(newstate);
          if (power != newstate) {
            pressButton(PIN_PWR);
            Serial.println(F("   => Emulating button press to toggle power"));
          } else {
            Serial.println(F("   => No need to change power state"));
          }
        } else {
          Serial.print(F("Incoming message for ID_POWER, unknown message type "));
          Serial.println(message.type);
        }
        break;
      
    case ID_LEVEL:
        if (message.type == V_LEVEL) {
          int newlevel = message.getInt();
          Serial.print(F("Incoming message to set Level from "));
          Serial.print(level);
          Serial.print(F("to: "));
          Serial.println(newlevel);

          if (newlevel<0) {
            Serial.println(F("  Levels below 0 are invalid"));
          } else {
            newlevel = min(newlevel, 3); // Maximum level is 3!

            bool newpower = (newlevel>0);
            if (newpower != power) {
              pressButton(PIN_PWR);
              Serial.println(F("   => Toggle power state for new level"));
              wait(500); // Give the humidifier some time to turn on, just to be on the safe side
              readVentaState();
            }
            // If we are in error state, a level change will exit the error state.
            // However, check if the error persists after changing the level!
            if (newpower && (newlevel != level)) {
              int reqchanges = (3 + newlevel - level) % 3;
              for (int i = 0; i < reqchanges; i++) {
                Serial.println(F("  Emulating UP/DOWN button press"));
                pressButton(PIN_UD);
                wait(100);
              }
            }
          }
          readVentaState();
          if (error) {
            Serial.println(F("ERROR status after level change!"));
          }
        } else {
          Serial.print(F("Incoming message for ID_LEVEL, unknown message type "));
          Serial.println(message.type);
        }
        break;

    case ID_ERROR:
        if (message.type == V_STATUS) {
          bool newstate = message.getBool();
          Serial.print(F("Incoming message to set ERROR LED to: "));
          Serial.println(newstate);
          
          saveState(ID_ERROR, newstate);
          if (newstate==1) {
            ledOn(PIN_ERR);
          } else {
            ledOff(PIN_ERR);
          }
        } else {
          Serial.print(F("Incoming message for ID_ERROR, unknown message type "));
          Serial.println(message.type);
        }
        break;
      
    case ID_OPEN:
        Serial.print(F("Incoming message for ID_OPEN, which cannot receive any messages, type="));
        Serial.println(message.type);
        break;
        
    default:
        Serial.print(F("Incoming message for sensor "));
        Serial.print(message.sensor);
        Serial.print(F(", which cannot receive any messages, type="));
        Serial.println(message.type);
  }
}


void readVentaState() {
  level = 0;
  power = digitalRead(LED_PWR) == LOW;
  error = digitalRead(LED_ERR) == LOW;
  caseopen = digitalRead(PIN_OPEN) == LOW;

  if (digitalRead(LED_1) == LOW) {
    level = 1;
  } else if (digitalRead(LED_2) == LOW) {
    level = 2;
  } else if (digitalRead(LED_3) == LOW) {
    level = 3;
  } else if (power == 0) {
    level = 0;
  } else if (error == 1) {
    level = -1;
  } else {
    level = -1;
  }
}



void pressButton(int pin) {
  digitalWrite(pin, HIGH);
  wait(100);
  digitalWrite(pin, LOW);
}
void ledOn(int pin) {
  digitalWrite(pin, HIGH);
}
void ledOff(int pin) {
  digitalWrite(pin, LOW);
}


void updateDisplay() {
  oled.setCursor(0, 0);
  oled.set2X();
  oled.print(F("Power: "));
  oled.print(power ? F("ON") : F("OFF"));  oled.clearToEOL(); oled.println();

  if (power) {
    oled.set2X();
    oled.print(F("Level "));
    oled.print(level);
    oled.clearToEOL(); oled.println();

    oled.print(F("Error: "));
    oled.print(error ? F("YES") : F("no"));
    oled.clearToEOL(); oled.println();
  } else {
    oled.clearToEOL(); oled.println();
    oled.clearToEOL(); oled.println();
  }

  oled.set1X();
  oled.print(F("Case Open: "));
  oled.print(caseopen ? F("YES") : F("no"));
  oled.clearToEOL(); oled.println();

  oled.set1X();
//  oled.print(F("pwr:"));oled.print(pwr); oled.print(F(", "));
  //oled.print("ud:");oled.print(ud); oled.print(", ");
  //oled.print("err:");oled.print(err); oled.print(", ");
  oled.clearToEOL(); oled.println();
  oled.clear(0, oled.displayWidth() - 1, oled.row(), oled.displayRows());
}



/*********************************************************************
This is an example for our Monochrome OLEDs based on SSD1306 drivers

  Pick one up today in the adafruit shop!
  ------> http://www.adafruit.com/category/63_98

This example is for a 128x64 size display using I2C to communicate
3 pins are required to interface (2 I2C and one reset)

Adafruit invests time and resources providing this open source code, 
please support Adafruit and open-source hardware by purchasing 
products from Adafruit!

Written by Limor Fried/Ladyada  for Adafruit Industries.  
BSD license, check license.txt for more information
All text above, and the splash screen must be included in any redistribution
*********************************************************************/
//#define SSD1306_128_64

//#include <SPI.h>
//#include <Wire.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>

//#define OLED_RESET 4
//Adafruit_SSD1306 display(OLED_RESET);
//#if (SSD1306_LCDHEIGHT != 64)
//# error(F("Height incorrect, please fix Adafruit_SSD1306.h!"));
//#endif

#include <SSD1306Ascii.h>
#include <SSD1306AsciiAvrI2c.h>
// 0X3C+SA0 - 0x3C or 0x3D
#define I2C_ADDRESS 0x3C
SSD1306AsciiAvrI2c oled;

#define PIN_PWR  3
#define PIN_UD   4
#define PIN_OPEN 5
#define LED_ON   6
#define LED_1    A1
#define LED_2    A2
#define LED_3    A3
#define LED_ERR  7
#define PIN_ERR  8


void setup()   {                
  Serial.begin(9600);

  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
//  display.begin(SSD1306_SWITCHCAPVCC, 0x3D);  // initialize with the I2C addr 0x3D (for the 128x64)
//  display.begin(SSD1306_SWITCHCAPVCC, 0x3C);  // initialize with the I2C addr 0x3D (for the 128x64)
  // init done
//  display.clearDisplay();
  oled.begin(&Adafruit128x64, I2C_ADDRESS);
  oled.setFont(Adafruit5x7);

  // Set up pins:
  // Button & LED Actions (output):
  //   - PIN_PWR  D3 ... Power button press
  //   - PIN_UD   D4 ... Up/Down button press
  //   - PIN_ERR  D8 ... Control Error LED
  // Button detection (input, pullup!):
  //   - PIN_OPEN D5 ... Case open Button => causes Error
  // LED detection (input, NO pullup, already connected to +VCC via LED+resistor):
  //   - LED_ON   D6 ... Power LED 
  //   - LED_ERR  D7 ... Error LED
  //   - LED_1    A1 ... Level 1 LED
  //   - LED_2    A2 ... Level 2 LED
  //   - LED_3    A3 ... Level 3 LED

  pinMode(PIN_PWR, OUTPUT);
  pinMode(PIN_UD,  OUTPUT);
  pinMode(PIN_OPEN,INPUT_PULLUP);
  pinMode(LED_ON,  INPUT);
  pinMode(LED_1,   INPUT);
  pinMode(LED_2,   INPUT);
  pinMode(LED_3,   INPUT);
  pinMode(LED_ERR, INPUT);
  pinMode(PIN_ERR, OUTPUT);
  digitalWrite(PIN_ERR, HIGH);

  pinMode(A0, INPUT_PULLUP);
}

int level = 0;
bool power = false;
bool error = false;
bool caseopen = false;

void loop() {
  power = digitalRead(LED_ON) == LOW;
  error = digitalRead(LED_ERR) == LOW;
  caseopen = digitalRead(PIN_OPEN) == LOW;

  if (digitalRead(LED_1) == LOW) {
    level = 1;
  }
  if (digitalRead(LED_2) == LOW) {
    level = 2;
  }
  if (digitalRead(LED_3) == LOW) {
    level = 3;
  }

int pwr = digitalRead(A0);
//digitalWrite(PIN_PWR, digitalRead(A0));
//  digitalRead(A6)
//  digitalRead(A7)


  oled.clear();
  oled.set2X();
  oled.print(F("Power: "));
  oled.println(power ? F("ON") : F("OFF"));

  oled.print(F("Error: "));
  oled.println(error ? F("YES") : F("no"));

  oled.set1X();
  oled.print(F("Case Open: "));
  oled.println(caseopen ? F("YES") : F("no"));

  oled.set2X();
  oled.print(F("Level "));
  oled.println(level);

  oled.set1X();
  oled.print(F("pwr:"));oled.print(pwr); oled.print(F(", "));
  oled.println();


//  ledOn(PIN_ERR);
//  delay(150);
//  ledOff(PIN_ERR);
//  delay(350);

  pressButton(PIN_UD);
  delay(1000);

}

void pressButton(int pin) {
  digitalWrite(pin, HIGH);
  delay(100);
  digitalWrite(pin, LOW);
}
void ledOn(int pin) {
  digitalWrite(pin, LOW);
}
void ledOff(int pin) {
  digitalWrite(pin, HIGH);
}


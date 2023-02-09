#include <SPI.h>                    // SPI comm library
#include <SD.h>                     // SD card comm library
#include <Wire.h>                   // Needed for color sensor
#include "Adafruit_TCS34725.h"      // Needed for color sensor

// Analog outputs for the UNO
#define redpin 3
#define greenpin 5
#define bluepin 6

// for a common anode LED, connect the common pin to +5V
// for common cathode, connect the common to ground

// set to false if using a common cathode LED
#define commonAnode true

// our RGB -> eye-recognized gamma color
byte gammatable[256];

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);

File txtFile; // microSD file object         
const int chipSelect = 10; // location of microSD communications;
int run = 0000;
char runFile[11] = "0000run.txt";
char flagFile[13] = "0000flags.txt";
unsigned long clocktime;

void setup() {
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

    // use these three pins to drive an LED
  #if defined(ARDUINO_ARCH_ESP32)
    ledcAttachPin(redpin, 1);
    ledcSetup(1, 12000, 8);
    ledcAttachPin(greenpin, 2);
    ledcSetup(2, 12000, 8);
    ledcAttachPin(bluepin, 3);
    ledcSetup(3, 12000, 8);
  #else
    pinMode(redpin, OUTPUT);
    pinMode(greenpin, OUTPUT);
    pinMode(bluepin, OUTPUT);
  #endif

  for (int i=0; i<256; i++) {
    float x = i;
    x /= 255;
    x = pow(x, 2.5);
    x *= 255;

    if (commonAnode) {
      gammatable[i] = 255 - x;
    } else {
      gammatable[i] = x;
    }
  }

  Serial.print("Initializing SD card...");

  if (!SD.begin()) {
    Serial.println("initialization failed!");
    return;
  }
  Serial.println("initialization done.");

  for (int i=0; i<999; i++) {
    sprintf(runFile, "%04drun.txt",run);
    if (1 == SD.exists(runFile)) {
      run++;
    }
    else {
      txtFile = SD.open(runFile, FILE_WRITE);
      txtFile.close();
      sprintf(flagFile, "%04dflags.txt",run);
      break;
    }
  }
}

void loop() {

  if (tcs.begin()) {
    Serial.println("Found sensor");
  } else {
    Serial.println("No TCS34725 found ... check your connections");
  }
  float red, green, blue;
  
  tcs.setInterrupt(false);  // turn on LED

  delay(60);  // takes 50ms to read

  tcs.getRGB(&red, &green, &blue);
  
  tcs.setInterrupt(true);  // turn off LED
  
  clocktime=millis();      // time in milli seconds

  // print the time and colors recorded
  txtFile = SD.open(runFile, FILE_WRITE);
  if (txtFile) {
  txtFile.print(clocktime, 1); 
  txtFile.print("\tR: "); txtFile.print(int(red)); 
  txtFile.print("\tG: "); txtFile.print(int(green)); 
  txtFile.print("\tB: "); txtFile.println(int(blue)); 
  // close the text file 
  txtFile.close();
  }
  else {
    Serial.println("error opening run file");
  }

}
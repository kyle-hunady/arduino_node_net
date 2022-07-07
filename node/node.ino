#include <SPI.h>
#include "printf.h"
#include "RF24.h"
#include <EEPROM.h>
#include "LowPower.h"

//#define DEBUG // comment to remove print statements
#ifdef DEBUG
  #define DEBUG_PRINTLN(x)  Serial.println(x)
  #define DEBUG_PRINT(x)    Serial.print(x)
  #define DEBUG_BEGIN(x)    Serial.begin(x)
#else 
  #define DEBUG_PRINTLN(x)
  #define DEBUG_PRINT(x)
  #define DEBUG_BEGIN(x)
#endif

#define CE_PIN 8
#define CSN_PIN 7
RF24 radio(CE_PIN, CSN_PIN);

uint8_t address[][6] = {"1Node", "2Node"};
bool radioNumber = 1; // 0 uses address[0] to transmit, 1 uses address[1] to transmit
uint8_t data[2];
uint8_t attempts;
byte deviceID;

void setup() {
  DEBUG_BEGIN(115200);
  getID();
  radioInit();
  #ifdef DEBUG
    printRadioDetails();
  #endif
}

void loop() {
  data[1] = attempts;
  sendData();
  attempts++; 

  goToSleep(8);  // duration in seconds
}

String arrToStr(uint8_t arr[], int len) { 
  String str = "{";
  for (int i = 0; i < len; i++) {
    str += (String) arr[i];
    if (i != len - 1)
      str += ",";
  }
  str += "}";
  return str;
}

void getID() {
  DEBUG_PRINTLN("==================================================");
  deviceID = EEPROM.read(0);
  data[0] = deviceID;
  DEBUG_PRINTLN("DEVICE ID: " + (String) deviceID);
  DEBUG_PRINTLN("==================================================");
}

void radioInit() {
  if (!radio.begin()) {
    DEBUG_PRINTLN("radio hardware is not responding!!");
    while (1) {} // hold in infinite loop
  }
  radio.setPALevel(RF24_PA_LOW);  // RF24_PA_MAX is default.
  radio.setPayloadSize(sizeof(data)); // float datatype occupies 4 bytes
  radio.openWritingPipe(address[radioNumber]);     // always uses pipe 0
  radio.openReadingPipe(1, address[!radioNumber]); // using pipe 1
  radio.stopListening();
}

void printRadioDetails() {
   printf_begin();             // needed only once for printing details
//   radio.printDetails();       // (smaller) function that prints raw register values
   radio.printPrettyDetails(); // (larger) function that prints human readable data
}

void sendData() {
  DEBUG_PRINT("Sending: " + arrToStr(data,sizeof(data)));
  DEBUG_PRINT("...");
  
  unsigned long start_timer = micros();                    // start the timer
  bool report = radio.write(&data, sizeof(data));      // transmit & save the report
  unsigned long end_timer = micros();                      // end the timer

  if (report) {
    DEBUG_PRINT("OK! (");
    DEBUG_PRINT(end_timer - start_timer);  
    DEBUG_PRINTLN("us)");
  } else {
    DEBUG_PRINTLN("failed"); // data was not delivered
  }
}

void goToSleep(int duration) {
  radio.powerDown();
  int cycles = duration / 8;
  for (int i = 0; i < cycles; i++) {
    LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF);  
  }
  radio.powerUp();
}

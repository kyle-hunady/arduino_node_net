#include <SPI.h>
#include "printf.h"
#include "RF24.h"
#include <EEPROM.h>
#include "Wire.h"
#include <MPU6050_light.h>

#define CE_PIN 8
#define CSN_PIN 7
RF24 radio(CE_PIN, CSN_PIN);

uint8_t address[][6] = {"1Node", "2Node"};
bool radioNumber = 1; // 0 uses address[0] to transmit, 1 uses address[1] to transmit
uint8_t data[3];
byte deviceID;

MPU6050 mpu(Wire);
unsigned long startTimer;
unsigned long sendInterval = 400;

void setup() {
  Serial.begin(115200);
  beginGyro();
  getID();
  radioInit();
  printRadioDetails();
  startTimer = millis();
}

void loop() {
  mpu.update();
 
  if (millis() - startTimer > sendInterval) {
    getCurrentAngle();
    sendData();
    startTimer = millis();
  }
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

byte getByte(float &val) {
  int mappedAngle = map((int) val, 0,360,0,255);
  return (byte) mappedAngle;
}

void getID() {
  Serial.println("==================================================");
  deviceID = EEPROM.read(0);
  data[0] = deviceID;
  Serial.println("DEVICE ID: " + (String) deviceID);
  Serial.println("==================================================");
}

void radioInit() {
  if (!radio.begin()) {
    Serial.println("radio hardware is not responding!!");
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
  Serial.print("Sending: " + arrToStr(data,sizeof(data)));
  Serial.print("...");
  
  unsigned long start_timer = micros();                    // start the timer
  bool report = radio.write(&data, sizeof(data));      // transmit & save the report
  unsigned long end_timer = micros();                      // end the timer

  if (report) {
    Serial.print("OK! (");
    Serial.print(end_timer - start_timer);  
    Serial.println("us)");
  } else {
    Serial.println("failed"); // data was not delivered
  }
}

void beginGyro() {
  Wire.begin();
  
  byte status = mpu.begin();
  Serial.print(F("MPU6050 status: "));
  Serial.println(status);
  while(status!=0){ } // stop everything if could not connect to MPU6050
  
  Serial.println(F("Calculating offsets, do not move MPU6050"));
//  delay(1000);
  // mpu.upsideDownMounting = true; // uncomment this line if the MPU6050 is mounted upside-down
  mpu.calcOffsets(); // gyro and accelero
  Serial.println("Done!\n");
}

void getCurrentAngle() {
  float xFloat = mpu.getAngleX();
  float yFloat = mpu.getAngleY();
  float zFloat = mpu.getAngleZ();

  byte x = getByte(xFloat);
  byte y = getByte(yFloat);
  byte z = getByte(zFloat);

  data[0] = x;
  data[1] = y;
  data[2] = z;
}

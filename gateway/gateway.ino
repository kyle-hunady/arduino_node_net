#include <SPI.h>
#include "printf.h"
#include "RF24.h"

#define CE_PIN 2
#define CSN_PIN 4
RF24 radio(CE_PIN, CSN_PIN);

uint8_t address[][6] = {"1Node", "2Node"};
bool radioNumber = 0; // 0 uses address[0] to transmit, 1 uses address[1] to transmit
uint8_t data[2];

void setup() {
  Serial.begin(115200);
  radioInit();
  printRadioDetails();
}

void loop() {
    uint8_t pipe;
    if (radio.available(&pipe)) {             // is there a data? get the pipe number that recieved it
      uint8_t bytes = radio.getPayloadSize(); // get the size of the data
      radio.read(&data, bytes);            // fetch data from FIFO
      Serial.print("Received ");
      Serial.print(bytes);                    // print the size of the data
      Serial.print(" bytes on pipe ");
      Serial.print(pipe);                     // print the pipe number
      Serial.print(": ");
      Serial.println(arrToStr(data,sizeof(data)));                // print the data's value
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

void radioInit() {
  if (!radio.begin()) {
    Serial.println("radio hardware is not responding!!");
    while (1) {} // hold in infinite loop
  }

  radio.setPALevel(RF24_PA_LOW);  // RF24_PA_MAX is default.
  radio.setPayloadSize(sizeof(data)); // float datatype occupies 4 bytes
  radio.openWritingPipe(address[radioNumber]);     // always uses pipe 0
  radio.openReadingPipe(1, address[!radioNumber]); // using pipe 1
  radio.startListening();
}

void printRadioDetails() {
   printf_begin();             // needed only once for printing details
//   radio.printDetails();       // (smaller) function that prints raw register values
   radio.printPrettyDetails(); // (larger) function that prints human readable data
}

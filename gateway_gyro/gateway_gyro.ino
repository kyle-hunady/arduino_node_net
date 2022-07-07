#if defined(ESP32)
  #include <WiFi.h>
#elif defined(ESP8266)
  #include <ESP8266WiFi.h>
#endif
#include <Firebase_ESP_Client.h>

//Provide the token generation process info.
#include "addons/TokenHelper.h"
//Provide the RTDB payload printing info and other helper functions.
#include "addons/RTDBHelper.h"

// Insert your network credentials
#define WIFI_SSID "YOUR_NETWORK_NAME"
#define WIFI_PASSWORD "YOUR_WIFI_PASSWORD"

// Insert Firebase project API Key
#define API_KEY "YOUR_API_KEY"

// Insert RTDB URLefine the RTDB URL */
#define DATABASE_URL "YOUR_REALTIME_DATABASE_URL" 

//Define Firebase Data object
FirebaseData fbdo;

FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

#include <SPI.h>
#include "printf.h"
#include "RF24.h"

#define CE_PIN 2
#define CSN_PIN 4
RF24 radio(CE_PIN, CSN_PIN);

uint8_t address[][6] = {"1Node", "2Node"};
bool radioNumber = 0; // 0 uses address[0] to transmit, 1 uses address[1] to transmit
uint8_t data[3];

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

void setup(){
  Serial.begin(115200);
  firebaseInit();
  radioInit();
  printRadioDetails();
}

void loop(){
  uint8_t pipe;
    if (radio.available(&pipe)) {             // is there a data? get the pipe number that recieved it
      uint8_t bytes = radio.getPayloadSize(); // get the size of the data
      radio.read(&data, bytes);            // fetch data from FIFO
      Serial.print("Received: ");
//      Serial.print(bytes);                    // print the size of the data
//      Serial.print(" bytes on pipe ");
//      Serial.print(pipe);                     // print the pipe number
//      Serial.print(": ");
      Serial.println(arrToStr(data,sizeof(data)));
//      Serial.print("..."); 
      // print the data's value
      if (Firebase.ready() && signupOK){
        sendFirebaseData();    
      }
    }
}

void firebaseInit() {
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  Serial.print("Connecting to Wi-Fi");
  while (WiFi.status() != WL_CONNECTED){
    Serial.print(".");
    delay(300);
  }
  Serial.println();
  Serial.print("Connected with IP: ");
  Serial.println(WiFi.localIP());
  Serial.println();

  /* Assign the api key (required) */
  config.api_key = API_KEY;

  /* Assign the RTDB URL (required) */
  config.database_url = DATABASE_URL;

  /* Sign up */
  if (Firebase.signUp(&config, &auth, "", "")){
    Serial.println("ok");
    signupOK = true;
  }
  else{
    Serial.printf("%s\n", config.signer.signupError.message.c_str());
  }

  /* Assign the callback function for the long running token generation task */
  config.token_status_callback = tokenStatusCallback; //see addons/TokenHelper.h
  
  Firebase.begin(&config, &auth);
  Firebase.reconnectWiFi(true);
}

void sendFirebaseData() {
  String targets[] = {"gyro/x", "gyro/y", "gyro/z"};
  for (int i = 0; i < 3; i++) {
    String target = targets[i];
    Serial.print("\tSending " + target + "/" + (String) data[i]);
    Serial.print("...");
    unsigned long start_timer = millis();
    if (Firebase.RTDB.setIntAsync(&fbdo, target, data[i])){
      unsigned long end_timer = millis();
        Serial.print("OK! (");
        Serial.print(end_timer - start_timer);
        Serial.println("ms)");
      }
      else {
        Serial.print("failed: ");
        Serial.println(fbdo.errorReason());
      }
  }
}

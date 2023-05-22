#include <Arduino.h>
#include <WiFi.h>
#include <Firebase_ESP_Client.h>
#include "addons/TokenHelper.h"
#include "addons/RTDBHelper.h"
#include <Adafruit_Sensor.h>
#include <DHT.h>

// WiFi, Firebase database, Pin Setting
#define WIFI_SSID "my_SSID"
#define WIFI_PASSWORD "my_PSWD"
#define API_KEY "my_API"
#define DATABASE_URL "my_URL" 

//optional (according to the board)
#define DHT11_PIN 27 
#define SOILMOIST_PIN 33

//For Finding number of ESP32
int name = -1; 

FirebaseData fbdo;
FirebaseAuth auth;
FirebaseConfig config;

unsigned long sendDataPrevMillis = 0;
int count = 0;
bool signupOK = false;

//DHT Sensor pin setting
DHT dht(DHT11_PIN, DHT11);

void setup(){
  Serial.begin(115200);
  dht.begin();  
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

  config.api_key = API_KEY;
  config.database_url = DATABASE_URL;

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

void loop(){
  
    if (Firebase.ready() && signupOK && (millis() - sendDataPrevMillis > 5000 || sendDataPrevMillis == 0)){
    sendDataPrevMillis = millis();
    int i = 1;

    while(name == -1){
      String myPath = "/test" + String(i) + "/code" ;
       
        if (Firebase.RTDB.getInt(&fbdo, myPath)) {
          if (fbdo.dataType() == "int") {
            int intValue = fbdo.intData();
            Serial.printf("There exist : test%d\n",intValue);
          }
          i++; // myname
        }
        else {
          Serial.println("I found my name !");

          if (Firebase.RTDB.setInt(&fbdo, myPath , i)){
            name = i;
          }
          else {
            Serial.println("FAILED");
          }
        }
    }
  
    // The Firebase path of Sensor Data
    String smPath = "/test" + String(name) + "/Soil Moisture" ;
    String temPath = "/test" + String(name) + "/Temperature" ;
    String humPath = "/test" + String(name) + "/Humidity" ;

    int val = analogRead(SOILMOIST_PIN);     // sensor pin connected to 35 pin of ESP32
    float scaled  = 100-((val-1000)/(21.0));
    if(scaled < 0 || scaled > 100)scaled = -1.00;
    
    float temperature = dht.readTemperature();
    float humidity = dht.readHumidity();
  
    // Soil Moisture sensor data sending
    if (Firebase.RTDB.setFloat(&fbdo, smPath , scaled)){
      Serial.printf("Sm : %.1f\n",scaled);
    }
    else {
      Serial.println("Sm FAILED");
    }
    
    // Temperature sensor data sending
    if (Firebase.RTDB.setFloat(&fbdo, temPath , temperature)){
      Serial.printf("Tmp : %.1f\n",temperature);
    }
    else {
      Serial.println("Tmp FAILED");
    }
    
    // Humidity sensor data sending
    if (Firebase.RTDB.setFloat(&fbdo, humPath , humidity)){
      Serial.printf("Mst : %.1f\n",humidity);
    }
    else {
      Serial.println("Mst FAILED");
    }
  }
}


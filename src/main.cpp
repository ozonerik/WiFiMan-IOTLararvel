#include <Arduino.h>
#include <ESP32Web.h>
#include <HTTPClient.h>               //Download: https://electronoobs.com/eng_arduino_httpclient.php
#include <WiFi.h>
// For documentation on ESP32Web.h, see: https://esp32web.com/docs


// LED Pin
#define LED_PIN 2

//Variables used in the code
String LED_id = "1";                  //Just in case you control more than 1 LED
bool toggle_pressed = false;          //Each time we press the push button    
String data_to_send = "";             //Text data to send to the server
unsigned int Actual_Millis, Previous_Millis;
int refresh_time = 200;               //Refresh rate of connection to website (recommended more than 1s)

//Inputs/outputs
int button1 = 5;                     //Connect push button on this pin
int LED = 4;

//ip or doman
String serverName = "https://iotapps.smkn1krangkeng.sch.id/api/led4update";
String serialNo = "1234";
String stLED = "1"; 

//Button press interruption
void IRAM_ATTR isr() {
  toggle_pressed = true; 
}


void setup() {
    Serial.begin(115200);
    delay(4000); // not always needed. Some boards need a delay to allow Serial to be ready

    pinMode(LED_PIN, OUTPUT);
    digitalWrite(LED_PIN, LOW);

    // Looping Functions: 
    t0_AP_Mode.setInterval(1000, ledBlink); // timer for LED blink
    t0_AP_Mode.setInterval(1000, stopAP); // timer for AP mode (WiFi Hostpot)

    // check config file -> run AP or STA mode -> start web server
    checkWiFiConfig();

    Serial.println("Setup done");

    pinMode(LED, OUTPUT);                   //Set pin 2 as OUTPUT
    pinMode(button1, INPUT_PULLDOWN);       //Set pin 13 as INPUT with pulldown
    attachInterrupt(button1, isr, RISING);  //Create interruption on pin 13
}

void fungsiLED(String serverPath){
      HTTPClient http;
      http.begin(serverPath.c_str());
      int httpResponseCode = http.GET();
      
      if (httpResponseCode>0) {
        Serial.print("HTTP Response code: ");
        Serial.println(httpResponseCode);
        String payload = http.getString();
        Serial.println(payload);
          if(httpResponseCode == 200){
            if(payload == "LED_is_off"){
              digitalWrite(LED, LOW);
            }
            else if(payload == "LED_is_on"){
              digitalWrite(LED, HIGH);
            }  
          }
      }
      else {
        Serial.print("Error code: ");
        Serial.println(httpResponseCode);
      }
      // Free resources
      http.end();
}

void loop() {

   handleState(); // handle the state of the web server (AP or STA)

   // ----- Write your looping code below -------- //

   if(WiFi.status()== WL_CONNECTED){
      HTTPClient http;                                  //Create new client
      if(toggle_pressed){
        if(digitalRead(LED)==LOW){
          data_to_send = "&st=1"; 
        }else{
          data_to_send = "&st=0";
        }                              //If button was pressed we send text: "toggle_LED"    
        toggle_pressed = false;                         //Also equal this variable back to false 
      }else{
        data_to_send = "";
      }
      
      //Begin new connection to website
      String serverPath=serverName+"?sn="+serialNo;  
      http.begin(serverPath.c_str());   //Indicate the destination webpage 
      http.setAuthorization("smeker", "p4ssw0rd123");
      http.addHeader("Content-Type", "application/x-www-form-urlencoded");         //Prepare the header
      
      int response_code = http.POST(data_to_send);
      
      //If the code is higher than 0, it means we received a response
      if(response_code > 0){
        Serial.println("HTTP code " + String(response_code));                     //Print return code
  
        if(response_code == 200){                                                 //If code is 200, we received a good response and we can read the echo data
          String response_body = http.getString();                                //Save the data comming from the website
          Serial.print("Server reply: ");                                         //Print data to the monitor for debug
          Serial.println(response_body);

          //If the received data is LED_is_off, we set LOW the LED pin
          if(response_body == "LED_is_off"){
            digitalWrite(LED, LOW);
          }
          //If the received data is LED_is_on, we set HIGH the LED pin
          else if(response_body == "LED_is_on"){
            digitalWrite(LED, HIGH);
          }  
        }//End of response_code = 200
      }//END of response_code > 0
      
      else{
       Serial.print("Error sending POST, code: ");
       Serial.println(response_code);
      }
      http.end();  
    }
    else{
      Serial.println("WIFI connection error");
    }



}




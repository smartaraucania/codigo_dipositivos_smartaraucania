#include <Adafruit_NeoPixel.h>
#include <Wire.h> //The DHT12 uses I2C comunication.
#include <WiFi.h>

#include <HTTPClient.h>

#include "M5Atom.h"

//---internals sensors---
float accX = 0, accY = 0, accZ = 0;
float gyroX = 0, gyroY = 0, gyroZ = 0;
float temp = 0;
bool IMU6886Flag = false;
//---internals sensors---

//---PIR sensor---
int movement = 0;
//---PIR sensor---

//--wifi credentials--
const char* ssid     = "my-ssid";
const char* password = "my-password";
//--wifi credentials--

void setup(){
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(115200);  // start serial for output
  M5.begin(true, false, true);

  //---internals sensors---
  if (M5.IMU.Init() != 0){
    IMU6886Flag = false;
  }else{
    IMU6886Flag = true;
  }
  //---internals sensors---  

  //---PIR sensor---
  pinMode(22, INPUT);
  //---PIR sensor---

  //--wifi starting--
  WiFi.begin(ssid, password); 
 
  while (WiFi.status() != WL_CONNECTED) { //Check for the connection
    delay(500);
    Serial.println("Connecting...");
  }
 
  Serial.print("Connected to the WiFi network with IP: ");
  Serial.println(WiFi.localIP());
  //--wifi stasrting--
  
}

void loop()
{
  imu6886sensor();
  delay(1000);
  motionSensor();
  postData();
  M5.update();      
}

String tempS = "";
String accXS = "";
String accYS = "";
String accZS = "";
String gyroXS = "";
String gyroYS = "";
String gyroZS = "";
String mvmt = "";

void postData(){

  tempS = (String) temp;
  accXS = (String) accX;
  accYS = (String) accY;
  accZS = (String) accZ;
  gyroXS = (String) gyroX;
  gyroYS = (String) gyroY;
  gyroZS = (String) gyroZ;
  mvmt = (String) movement;
    
  if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
 
  postRerquest("inertia002%3AIMU6886", "internTmp|"+tempS+"|gyroX|"+gyroXS+"|gyroY|"+gyroYS+"|gyroZ|"+gyroZS+"|accX|"+accXS+"|accY|"+accYS+"|accZ|"+accZS);
  delay(300);

  postRerquest("motion001%3AAS312", "movement|"+mvmt);
  delay(2400);
 
  }else{
     Serial.println("Error in WiFi connection");   
  }
  //delay(5000);  //Send a request every 5 seconds
}

String httpRequest = "";
int httpResponseCode = 0;

void postRerquest(String sensorId, String measurement){

  HTTPClient http;   

  httpRequest = "https://fw-producer.smartaraucania.org/iota-north/iot/d?k=rdkdZrdvl2t2PlX3MdxgzhbI927L1uRm&i="+sensorId;
  //Serial.println(httpRequest);
  
  http.begin(httpRequest);  //Specify destination for HTTP request
  http.addHeader("Content-Type", "text/plain");    //Specify content-type header
   
  httpResponseCode = http.POST(measurement);   //Send the actual POST request
  //Serial.println(measurement);
  
  if(httpResponseCode>0){
    Serial.println(sensorId+" "+httpResponseCode);   //Print return code
  }else{
    Serial.print("Error on sending request: ");
    Serial.println(httpResponseCode);
  }
  http.end();  //Free resources
}

void motionSensor(){
  if(digitalRead(22)==1){
    Serial.println("Movimiento");
    movement = 1;
  }
  else{
    Serial.println("Nada");
    movement = 0;
  }
}

void imu6886sensor(){
  //if (IMU6886Flag == true){
        M5.IMU.getGyroData(&gyroX, &gyroY, &gyroZ);
        M5.IMU.getAccelData(&accX, &accY, &accZ);
        M5.IMU.getTempData(&temp);

        Serial.printf("%.2f,%.2f,%.2f o/s \r\n", gyroX, gyroY, gyroZ);
        Serial.printf("%.2f,%.2f,%.2f mg\r\n", accX * 1000, accY * 1000, accZ * 1000);
        Serial.printf("Temperature : %.2f C \r\n", temp);
   //}
}

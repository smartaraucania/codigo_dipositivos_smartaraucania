/*
    note: need add library Adafruit_BMP280 from library manage
    Github: https://github.com/adafruit/Adafruit_BMP280_Library
*/

#include <M5StickC.h>

#include <Wire.h> //The DHT12 uses I2C comunication.
#include <WiFi.h>

#include <HTTPClient.h>

#include "DHT12.h"
#include "Adafruit_Sensor.h"
#include <Adafruit_BMP280.h>
#include "bmm150.h"
#include "bmm150_defs.h"
DHT12 dht12;
BMM150 bmm = BMM150();
bmm150_mag_data value_offset;
Adafruit_BMP280 bme;

//------------------wifi credentials-------------------//
const char *ssid = "Kaqix";
const char *password = "2.4Gwifi332211.,";
//------------------wifi credentials-------------------//

void calibrate(uint32_t timeout)
{
  int16_t value_x_min = 0;
  int16_t value_x_max = 0;
  int16_t value_y_min = 0;
  int16_t value_y_max = 0;
  int16_t value_z_min = 0;
  int16_t value_z_max = 0;
  uint32_t timeStart = 0;

  bmm.read_mag_data();
  value_x_min = bmm.raw_mag_data.raw_datax;
  value_x_max = bmm.raw_mag_data.raw_datax;
  value_y_min = bmm.raw_mag_data.raw_datay;
  value_y_max = bmm.raw_mag_data.raw_datay;
  value_z_min = bmm.raw_mag_data.raw_dataz;
  value_z_max = bmm.raw_mag_data.raw_dataz;
  delay(100);

  timeStart = millis();

  while ((millis() - timeStart) < timeout)
  {
    bmm.read_mag_data();

    /* Update x-Axis max/min value */
    if (value_x_min > bmm.raw_mag_data.raw_datax)
    {
      value_x_min = bmm.raw_mag_data.raw_datax;
    }
    else if (value_x_max < bmm.raw_mag_data.raw_datax)
    {
      value_x_max = bmm.raw_mag_data.raw_datax;
    }

    /* Update y-Axis max/min value */
    if (value_y_min > bmm.raw_mag_data.raw_datay)
    {
      value_y_min = bmm.raw_mag_data.raw_datay;
    }
    else if (value_y_max < bmm.raw_mag_data.raw_datay)
    {
      value_y_max = bmm.raw_mag_data.raw_datay;
    }

    /* Update z-Axis max/min value */
    if (value_z_min > bmm.raw_mag_data.raw_dataz)
    {
      value_z_min = bmm.raw_mag_data.raw_dataz;
    }
    else if (value_z_max < bmm.raw_mag_data.raw_dataz)
    {
      value_z_max = bmm.raw_mag_data.raw_dataz;

      delay(1);
    }

    value_offset.x = value_x_min + (value_x_max - value_x_min) / 2;
    value_offset.y = value_y_min + (value_y_max - value_y_min) / 2;
    value_offset.z = value_z_min + (value_z_max - value_z_min) / 2;
  }

  //-----------------MRU6886---------------------//
  float accX = 0;
  float accY = 0;
  float accZ = 0;

  float gyroX = 0;
  float gyroY = 0;
  float gyroZ = 0;

  float temp = 0;
  //-----------------MRU6886---------------------//

  void setup()
  {
    // put your setup code here, to run once:
    M5.begin();
    Wire.begin(0, 26);
    M5.Lcd.setRotation(3);
    M5.Lcd.fillScreen(BLACK);

    //------------------environment-------------------//
    if (bmm.initialize() == BMM150_E_ID_NOT_CONFORM)
    {
      Serial.println("Chip ID can not read!");
      while (1)
        ;
    }
    else
    {
      Serial.println("Initialize done!");
    }
    if (!bme.begin(0x76))
    {
      Serial.println("Could not find a valid BMP280 sensor, check wiring!");
      while (1)
        ;
    }
    calibrate(10);
    //------------------environment-------------------//

    //------------------MPU6886-------------------//
    M5.MPU6886.Init();
    M5.Lcd.setTextSize(1);
    M5.Lcd.setCursor(40, 0);
    M5.Lcd.println("MPU6886 TEST");
    M5.Lcd.setCursor(0, 15);
    M5.Lcd.println("  X       Y       Z");
    //------------------MPU6886-------------------//

    //------------------wifi credentials-------------------//
    WiFi.begin(ssid, password);

    while (WiFi.status() != WL_CONNECTED)
    { //Check for the connection
      delay(500);
      Serial.println("Connecting...");
    }

    Serial.print("Connected to the WiFi network with IP: ");
    Serial.println(WiFi.localIP());
    //------------------wifi credentials-------------------//
  }

  uint8_t setup_flag = 1;

  void loop()
  {
    env();
    MRU6886();
    postData();
  }

  float tmp = 0;
  float hum = 0;
  float pressure = 0;
  float headingDegrees = 0;
  float xyHeadingDegrees = 0;
  float zxHeadingDegrees = 0;

  void env()
  {
    // put your main code here, to run repeatedly:
    tmp = dht12.readTemperature();
    hum = dht12.readHumidity();

    bmm150_mag_data value;
    bmm.read_mag_data();

    value.x = bmm.raw_mag_data.raw_datax - value_offset.x;
    value.y = bmm.raw_mag_data.raw_datay - value_offset.y;
    value.z = bmm.raw_mag_data.raw_dataz - value_offset.z;

    float xyHeading = atan2(value.x, value.y);
    float zxHeading = atan2(value.z, value.x);
    float heading = xyHeading;

    if (heading < 0)
      heading += 2 * PI;
    if (heading > 2 * PI)
      heading -= 2 * PI;
    headingDegrees = heading * 180 / M_PI;
    xyHeadingDegrees = xyHeading * 180 / M_PI;
    zxHeadingDegrees = zxHeading * 180 / M_PI;

    pressure = bme.readPressure();

    delay(500);
    calibrate(10);
  }

  void MRU6886()
  {
    M5.MPU6886.getGyroData(&gyroX, &gyroY, &gyroZ);
    M5.MPU6886.getAccelData(&accX, &accY, &accZ);
    M5.MPU6886.getTempData(&temp);

    M5.Lcd.setCursor(0, 30);
    M5.Lcd.printf("%.2f   %.2f   %.2f      ", gyroX, gyroY, gyroZ);
    M5.Lcd.setCursor(140, 30);
    M5.Lcd.print("o/s");
    M5.Lcd.setCursor(0, 45);
    M5.Lcd.printf("%.2f   %.2f   %.2f      ", accX * 1000, accY * 1000, accZ * 1000);
    M5.Lcd.setCursor(140, 45);
    M5.Lcd.print("mg");
    M5.Lcd.setCursor(0, 60);
    M5.Lcd.printf("Temperature : %.2f C", temp);

    delay(500);
  }

  String internTempS = "";
  String accXS = "";
  String accYS = "";
  String accZS = "";
  String gyroXS = "";
  String gyroYS = "";
  String gyroZS = "";
  String tmpS = "";
  String humS = "";
  String pressureS = "";
  String headingDegreesS = "";
  String xyHeadingDegreesS = "";
  String zxHeadingDegreesS = "";

  void postData()
  {

    internTempS = (String)temp;
    accXS = (String)accX;
    accYS = (String)accY;
    accZS = (String)accZ;
    gyroXS = (String)gyroX;
    gyroYS = (String)gyroY;
    gyroZS = (String)gyroZ;
    tmpS = (String)tmp;
    humS = (String)hum;
    pressureS = (String)pressure;
    headingDegreesS = (String)headingDegrees;
    xyHeadingDegreesS = (String)xyHeadingDegrees;
    zxHeadingDegreesS = (String)zxHeadingDegrees;

    if (WiFi.status() == WL_CONNECTED)
    { //Check WiFi connection status

      postRerquest("inertia001%3AIMU6886", "internTmp|" + internTempS + "|gyroX|" + gyroXS + "|gyroY|" + gyroYS + "|gyroZ|" + gyroZS + "|accX|" + accXS + "|accY|" + accYS + "|accZ|" + accZS);
      delay(300);

      postRerquest("environment003%3AStickHat", "temperature|" + tmpS + "|humidity|" + humS + "|pressure|" + pressureS + "|headingDegrees|" + headingDegreesS + "|xyHeadingDegrees|" + xyHeadingDegreesS + "|zxHeadingDegrees|" + zxHeadingDegreesS);
      delay(2400);
    }
    else
    {
      Serial.println("Error in WiFi connection");
    }
    //delay(5000);  //Send a request every 5 seconds
  }

  String httpRequest = "";
  int httpResponseCode = 0;

  void postRerquest(String sensorId, String measurement)
  {

    HTTPClient http;

    httpRequest = "https://fw-producer.smartaraucania.org/iota-north/iot/d?k=rdkdZrdvl2t2PlX3MdxgzhbI927L1uRm&i=" + sensorId;

    http.begin(httpRequest);                      //Specify destination for HTTP request
    http.addHeader("Content-Type", "text/plain"); //Specify content-type header

    httpResponseCode = http.POST(measurement); //Send the actual POST request

    if (httpResponseCode > 0)
    {
      Serial.println(sensorId + " " + httpResponseCode); //Print return code
    }
    else
    {
      Serial.print("Error on sending request: ");
      Serial.println(httpResponseCode);
    }
    http.end(); //Free resources
  }

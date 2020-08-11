//----------------Earth Sensor--------------
const int sensorPin = A1;
//----------------Earth Sensor--------------

//----------------RGB led-------------------
#include <Adafruit_NeoPixel.h>

// Which pin on the Arduino is connected to the NeoPixels?
// On a Trinket or Gemma we suggest changing this to 1
#define PIN            5
// How many NeoPixels are attached to the Arduino?
#define NUMPIXELS      3

// When we setup the NeoPixel library, we tell it how many pixels, and which pin to use to send signals.
// Note that for older NeoPixel strips you might need to change the third parameter--see the strandtest
// example for more information on possible values.
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);

int delayval = 150;
//----------------RGB led-------------------

//----------------RGB Color Unit-------------------
#include <Wire.h>
#include "Adafruit_TCS34725.h"

// set to false if using a common cathode LED
#define commonAnode true

// our RGB -> eye-recognized gamma color
byte gammatable[256];

static uint16_t color16(uint16_t r, uint16_t g, uint16_t b) {
  uint16_t _color;
  _color = (uint16_t)(r & 0xF8) << 8;
  _color |= (uint16_t)(g & 0xFC) << 3;
  _color |= (uint16_t)(b & 0xF8) >> 3;
  return _color;
}

Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_4X);
//----------------RGB Color Unit-------------------

//----------------IR Unit-------------------
int ir_recv_pin = 3;
int ir_send_pin = 4;
int last_recv_value = 0;
int cur_recv_value = 0;
//----------------IR Unit-------------------

 
void setup() {
   Serial.begin(115200);
   //----------------RGB led-------------------
   pixels.begin(); // This initializes the NeoPixel library.
   //----------------RGB led-------------------
   
  //----------------RGB Color Unit-------------------
   while(!tcs.begin()){
      Serial.println("No TCS34725 found ... check your connections");
      delay(1000);
  }
  tcs.setIntegrationTime(TCS34725_INTEGRATIONTIME_154MS);
  tcs.setGain(TCS34725_GAIN_4X);
  //----------------RGB Color Unit-------------------

  //----------------IR Unit-------------------
  pinMode(ir_recv_pin, INPUT);
  pinMode(ir_send_pin, OUTPUT);
  //send infrared light
  //now, you can see the infrared light through mobile phone camera
  digitalWrite(ir_send_pin, 1);
  //----------------IR Unit-------------------
}
 
void loop() 
{
   rgbled();   
   //earthSensorDigital();
   colorSensor();
   earthSensor();
   infrared();
   //la informacion se envia via serial con el formato
   //[Luz, Red, Green, Blue, Hex Color, Soil Moisture, Infrared]
   delay(1000);
}

int humedad = 0;

void earthSensor(){
  humedad = analogRead(sensorPin);
  //Serial.print("lectura analógica: ");
  //Serial.println(humedad);
  Serial.print(humedad);
  Serial.print(",");
  /*if(humedad < 500)
   {
      Serial.println("Encendido");  
      //hacer las acciones necesarias
   }*/
}

/*void earthSensorDigital(){
   int humedad = digitalRead(4);

   Serial.print("lectura digital: ");
   Serial.println(humedad);
   //mandar mensaje a puerto serie en función del valor leido
   if (humedad == HIGH)
   {
      Serial.println("Encendido");   
      //aquí se ejecutarían las acciones
   }
   delay(1000);
}*/

float r, g, b;
int ir, ig, ib;
String hexColor;

void colorSensor(){
  uint16_t clear, red, green, blue;

  delay(60);  // takes 50ms to read 
  
  tcs.getRawData(&red, &green, &blue, &clear);

  /*Serial.print("C:\t"); Serial.print(clear);
  Serial.print("\tR:\t"); Serial.print(red);
  Serial.print("\tG:\t"); Serial.print(green);
  Serial.print("\tB:\t"); Serial.print(blue);*/

  Serial.print(clear);
  Serial.print(",");
  Serial.print(red);
  Serial.print(",");
  Serial.print(green);
  Serial.print(",");
  Serial.print(blue);
  Serial.print(",");

  // Figure out some basic hex code for visualization
  uint32_t sum = clear;
  
  r = red; r /= sum;
  g = green; g /= sum;
  b = blue; b /= sum;
  r *= 256; g *= 256; b *= 256;
  //Serial.print("\t");
  Serial.print((int)r, HEX); Serial.print((int)g, HEX); Serial.print((int)b, HEX);
  Serial.print(",");
  
  /*ir = (int)r;
  ig = (int)g;
  ib = (int)b;

  hexColor = String(ir, HEX)+""+String(ig, HEX)+""+String(ib, HEX);*/

  //Serial.println();
  //Serial.println("Hexa Colors : "+hexColor);
  uint16_t _color = color16((int)r, (int)g, (int)b);
}

void infrared(){
  //now, once you press the button on a remote controller to send infrared light
  //the screen will display "detected!"
  cur_recv_value = digitalRead(ir_recv_pin);
  if(last_recv_value != cur_recv_value){
    if(cur_recv_value == 0){//0: detected 1: not detected
       //Serial.println("detected");
    }
    last_recv_value = cur_recv_value;
  }
  Serial.println(cur_recv_value);
  rgbled();
}

void rgbled(){
  pixels.setPixelColor(0, pixels.Color(100,0,0)); // Moderately bright red color.
  pixels.setPixelColor(1, pixels.Color(0,100,0)); // Moderately bright green color.
  pixels.setPixelColor(2, pixels.Color(0,0,100)); // Moderately bright blue color.

  pixels.show(); // This sends the updated pixel color to the hardware.
  delay(delayval); // Delay for a period of time (in milliseconds)

  pixels.setPixelColor(0, pixels.Color(0,100,0));
  pixels.setPixelColor(1, pixels.Color(0,0,100));
  pixels.setPixelColor(2, pixels.Color(100,0,0));
  pixels.show();
  delay(delayval);

  pixels.setPixelColor(0, pixels.Color(0,0,100));
  pixels.setPixelColor(1, pixels.Color(100,0,0));
  pixels.setPixelColor(2, pixels.Color(0,100,0));
  pixels.show();
  delay(delayval);
}

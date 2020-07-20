#include <M5Stack.h>
#include <Wire.h> //The DHT12 uses I2C comunication.
#include <WiFi.h>

#include <HTTPClient.h>
#include <Adafruit_BMP280.h>

#include "DHT12.h"
#include "Adafruit_Sensor.h"

//---TOF configuration
#define VL53L0X_REG_IDENTIFICATION_MODEL_ID         0xc0
#define VL53L0X_REG_IDENTIFICATION_REVISION_ID      0xc2
#define VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD   0x50
#define VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD 0x70
#define VL53L0X_REG_SYSRANGE_START                  0x00
#define VL53L0X_REG_RESULT_INTERRUPT_STATUS         0x13
#define VL53L0X_REG_RESULT_RANGE_STATUS             0x14
#define address 0x29

byte gbuf[16];
//---TOF configuration--

//---ENV configuration
DHT12 dht12; //Preset scale CELSIUS and ID 0x5c.
Adafruit_BMP280 bme;
//---ENV configuration--

//---env variables--
float tmp = 0;
float hum = 0;
float pressure = 0;
//ldr variables--
uint16_t ldrValue = 0;
//tof variables--
uint16_t acnt = 0;
uint16_t scnt = 0;
uint16_t dist = 0;
//---env variables--

//--wifi credentials--
const char* ssid     = "my-ssid";
const char* password = "2my-password";
//--wifi credentials--

void setup() {
  delay(100);
  Wire.begin();        // join i2c bus (address optional for master)
  Serial.begin(115200);  // start serial for output
  
  M5.begin();
  M5.Power.begin();

  //--env validation--
  while (!bme.begin(0x76)){  
      Serial.println("BMP280 sensor not found!");
      delay(500);
  }
  //--env validation--

  //--LDR stuff--
  dacWrite(25, 0); //disable the speak noise
  pinMode(26, INPUT);
  //--LDR stuff

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

void loop() {
  //---Tof data
  //Serial.println("//////////////////////////////////////////////////");
  test();
  //---Tof data
  
  //---environment sensor data
  //Serial.println("//////////////////////////////////////////////////");
  env();
  //---environment sensor data

  //---LDR data
  //Serial.println("//////////////////////////////////////////////////");
  ldr();
  //---LDR data   

  //---post request
  postData();
  //---post request
  delay(1000);
}

void postData(){
  String ldrValueS = (String) ldrValue;
  String tmpS = (String) tmp;
  String humS = (String) hum;
  String pressureS = (String) pressure;
  String distS = (String) dist;
    
  if(WiFi.status()== WL_CONNECTED){   //Check WiFi connection status
 
  postRerquest("light002%3AM5LDR", "ldr|"+ldrValueS);
  delay(300);
  
  postRerquest("environment002%3AM5environment", "temperature|"+tmpS+"|humidity|"+humS+"|pressure|"+pressureS);
  delay(300);

  postRerquest("tof001%3AVL53L0X", "distance|"+distS);
  delay(2400);
 
 }else{
     Serial.println("Error in WiFi connection");   
  }
  //delay(5000);  //Send a request every 5 seconds
}

void postRerquest(String sensorId, String measurement){

  HTTPClient http;   

  String httpRequest = "https://fw-producer.smartaraucania.org/iota-north/iot/d?k=rdkdZrdvl2t2PlX3MdxgzhbI927L1uRm&i="+sensorId;
  //Serial.println(httpRequest);
  
  http.begin(httpRequest);  //Specify destination for HTTP request
  http.addHeader("Content-Type", "text/plain");    //Specify content-type header
   
  int httpResponseCode = http.POST(measurement);   //Send the actual POST request
  //Serial.println(measurement);
  
  if(httpResponseCode>0){
    Serial.println(httpResponseCode);   //Print return code
  }else{
    Serial.print("Error on sending request: ");
    Serial.println(httpResponseCode);
  }
  http.end();  //Free resources
}

uint16_t digitalRead_value = 0;

void ldr(){
  //ldr data gathering
  ldrValue = analogRead(36);
}

void env(){
  //environment sensor data gathering
  tmp = dht12.readTemperature();
  hum = dht12.readHumidity();
  pressure = bme.readPressure();
  //Serial.printf("Temperatura: %2.2f*C  Humedad: %0.2f%%  Pressure: %0.2fPa\r\n", tmp, hum, pressure);
}

// tof data gathering
void test() {
  byte val1 = read_byte_data_at(VL53L0X_REG_IDENTIFICATION_REVISION_ID);

  val1 = read_byte_data_at(VL53L0X_REG_IDENTIFICATION_MODEL_ID);

  val1 = read_byte_data_at(VL53L0X_REG_PRE_RANGE_CONFIG_VCSEL_PERIOD);

  val1 = read_byte_data_at(VL53L0X_REG_FINAL_RANGE_CONFIG_VCSEL_PERIOD);

  write_byte_data_at(VL53L0X_REG_SYSRANGE_START, 0x01);

  byte val = 0;
  int cnt = 0;
  while (cnt < 100) { // 1 second waiting time max
    delay(10);
    val = read_byte_data_at(VL53L0X_REG_RESULT_RANGE_STATUS);
    if (val & 0x01) break;
    cnt++;
  }

  read_block_data_at(0x14, 12);
  acnt = makeuint16(gbuf[7], gbuf[6]);
  scnt = makeuint16(gbuf[9], gbuf[8]);
  dist = makeuint16(gbuf[11], gbuf[10]);
  byte DeviceRangeStatusInternal = ((gbuf[0] & 0x78) >> 3);
  
}

uint16_t bswap(byte b[]) {
  // Big Endian unsigned short to little endian unsigned short
  uint16_t val = ((b[0] << 8) & b[1]);
  return val;
}

uint16_t makeuint16(int lsb, int msb) {
    return ((msb & 0xFF) << 8) | (lsb & 0xFF);
}

void write_byte_data(byte data) {
  Wire.beginTransmission(address);
  Wire.write(data);
  Wire.endTransmission();
}

void write_byte_data_at(byte reg, byte data) {
  // write data word at address and register
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(data);
  Wire.endTransmission();
}

void write_word_data_at(byte reg, uint16_t data) {
  // write data word at address and register
  byte b0 = (data &0xFF);
  byte b1 = ((data >> 8) && 0xFF);
    
  Wire.beginTransmission(address);
  Wire.write(reg);
  Wire.write(b0);
  Wire.write(b1);
  Wire.endTransmission();
}

byte read_byte_data() {
  Wire.requestFrom(address, 1);
  while (Wire.available() < 1) delay(1);
  byte b = Wire.read();
  return b;
}

byte read_byte_data_at(byte reg) {
  //write_byte_data((byte)0x00);
  write_byte_data(reg);
  Wire.requestFrom(address, 1);
  while (Wire.available() < 1) delay(1);
  byte b = Wire.read();
  return b;
}

uint16_t read_word_data_at(byte reg) {
  write_byte_data(reg);
  Wire.requestFrom(address, 2);
  while (Wire.available() < 2) delay(1);
  gbuf[0] = Wire.read();
  gbuf[1] = Wire.read();
  return bswap(gbuf); 
}

void read_block_data_at(byte reg, int sz) {
  int i = 0;
  write_byte_data(reg);
  Wire.requestFrom(address, sz);
  for (i=0; i<sz; i++) {
    while (Wire.available() < 1) delay(1);
    gbuf[i] = Wire.read();
  }
}


uint16_t VL53L0X_decode_vcsel_period(short vcsel_period_reg) {
  // Converts the encoded VCSEL period register value into the real
  // period in PLL clocks
  uint16_t vcsel_period_pclks = (vcsel_period_reg + 1) << 1;
  return vcsel_period_pclks;
}

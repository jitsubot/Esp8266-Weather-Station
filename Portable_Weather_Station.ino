//Portable Weather Station ver 3
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include "Adafruit_VEML6070.h"
#include "ClosedCube_HDC1080.h"
#include "ccs811.h"  // CCS811 library

const char* ssid     = "TuxBot"; // Your ssid
const char* password = "YourPassword"; // Your Password
WiFiServer server(80);

Adafruit_BMP280 bmp; // I2C
Adafruit_VEML6070 uv = Adafruit_VEML6070();
ClosedCube_HDC1080 hdc1080;
CCS811 ccs811(D3); // nWAKE on D3

void setup() {
 Serial.begin(115200);
Serial.print("Connecting to ");
Serial.println(ssid);
WiFi.begin(ssid, password);
 
while (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");
}
 
Serial.println("");
Serial.println("WiFi is connected");
server.begin();
Serial.println("Server started");

// >>> SENSOR CCS 811 Starting   <<<<<<<<<<<<<<<<<<<.
  Wire.begin(); // ccs811 startup
  ccs811.set_i2cdelay(50); // Needed for ESP8266 because it doesn't handle I2C clock stretch correctly
  bool ok= ccs811.begin();
  if( !ok ) Serial.println("setup: CCS811 begin FAILED");
  // Start measuring
  ok= ccs811.start(CCS811_MODE_1SEC);
  if( !ok ) Serial.println("setup: CCS811 start FAILED");
// >>> SENSOR HDC 1080 Starting   <<<<<<<<<<<<<<<<<<<<
  // Default settings: 
  //  - Heater off
  //  - 14 bit Temperature and Humidity Measurement Resolutions
  hdc1080.begin(0x40);
// >>> SENSOR VEML 6070 Starting <<<<<<<<<<<<<<<<<<<<<<<,
  uv.begin(VEML6070_1_T);  // pass in the integration time constant
// >>> SENSOR BMP 280 Starting    <<<<<<<<<<<<<<<<<<<<,
  Serial.println(WiFi.localIP());
  if (!bmp.begin(0x76)) {
  Serial.println(F("Could not find a valid BMP280 sensor, check wiring!"));
  while (1);
  }
  /* Default settings from datasheet. */
  bmp.setSampling(Adafruit_BMP280::MODE_NORMAL,     /* Operating Mode. */
                  Adafruit_BMP280::SAMPLING_X2,     /* Temp. oversampling */
                  Adafruit_BMP280::SAMPLING_X16,    /* Pressure oversampling */
                  Adafruit_BMP280::FILTER_X16,      /* Filtering. */
                  Adafruit_BMP280::STANDBY_MS_500); /* Standby time. */
}

void BMP(){
Serial.begin(9600);
Serial.print(F("Temperature = "));
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");

    Serial.print(F("Pressure = "));
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");

    Serial.print(F("Approx altitude = "));
    Serial.print(bmp.readAltitude(1013.25)); /* Adjusted to local forecast! */
    Serial.println(" m");

    Serial.println();
    delay(2000);
}
void VEML6070(){
Serial.print("UV light level: "); Serial.println(uv.readUV());
  
  delay(1000);
}


void loop() {
    Serial.print(F("Temperature = "));
    Serial.print(bmp.readTemperature());
    Serial.println(" *C");

    Serial.print(F("Pressure = "));
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");

    Serial.print(F("Approx altitude = "));
    Serial.print(bmp.readAltitude(1013.25)); /* Adjusted to local forecast! */
    Serial.println(" m");

    Serial.println();
    delay(1000);
    
    Serial.print("UV light level: "); Serial.println(uv.readUV());
    delay(2000);

  Serial.print("T=");
  Serial.print(hdc1080.readTemperature());
  Serial.print("C, RH=");
  Serial.print(hdc1080.readHumidity());
  Serial.println("%");
  delay(3000);

// Read
  uint16_t eco2, etvoc, errstat, raw;
  ccs811.read(&eco2,&etvoc,&errstat,&raw); 
  
  // Print measurement results based on status
  if( errstat==CCS811_ERRSTAT_OK ) { 
    Serial.print("CCS811: ");
    Serial.print("Carbon Dioxide=");  Serial.print(eco2);     Serial.print(" ppm  ");
    Serial.print("Volatile Organic Compound="); Serial.print(etvoc);    Serial.print(" ppb  ");
    //Serial.print("raw6=");  Serial.print(raw/1024); Serial.print(" uA  "); 
    //Serial.print("raw10="); Serial.print(raw%1024); Serial.print(" ADC  ");
    //Serial.print("R="); Serial.print((1650*1000L/1023)*(raw%1024)/(raw/1024)); Serial.print(" ohm");
    Serial.println();
  } else if( errstat==CCS811_ERRSTAT_OK_NODATA ) {
    Serial.println("CCS811: waiting for (new) data");
  } else if( errstat & CCS811_ERRSTAT_I2CFAIL ) { 
    Serial.println("CCS811: I2C error");
  } else {
    Serial.print("CCS811: errstat="); Serial.print(errstat,HEX); 
    Serial.print("="); Serial.println( ccs811.errstat_str(errstat) ); 
  }
  
  // Wait
  delay(4000); 

  WiFiClient client = server.available();
client.println("HTTP/1.1 200 OK");
client.println("Content-Type: text/html");
client.println("Connection: close");  // the connection will be closed after completion of the response
client.println("Refresh: 5");  // update the page after 10 sec
client.println();
client.println("<!DOCTYPE HTML>");
client.println("<html>");
client.println("<style>html { font-family: Cairo; display: block; margin: 0px auto; text-align: center;color: #fcfdff; background-color: #2058b2;}");
client.println("body{margin-top: 50px;}");
client.println("h1 {margin: 50px auto 30px; font-size: 50px; text-align: center;}");
client.println(".side_adjust{display: inline-block;vertical-align: middle;position: relative;}");
client.println(".text1{font-weight: 180; padding-left: 15px; font-size: 50px; width: 220px; text-align: left; color: #fcfdff;}");
client.println(".data1{font-weight: 180; padding-left: 80px; font-size: 50px;color: #d8e85f;}");
client.println(".text2{font-weight: 180; padding-left: 15px; font-size: 50px; width: 250px; text-align: left; color: #fcfdff;}");
client.println(".data2{font-weight: 180; padding-left: 80px; font-size: 50px;color: #d8e85f;}");
client.println(".text3{font-weight: 180; padding-left: 15px; font-size: 50px; width: 220px; text-align: left; color: #fcfdff;}");
client.println(".data3{font-weight: 180; padding-left: 80px; font-size: 50px;color: #d8e85f;}");
client.println(".text4{font-weight: 180; padding-left: 15px; font-size: 50px; width: 220px; text-align: left; color: #fcfdff;}");
client.println(".data4{font-weight: 180; padding-left: 80px; font-size: 50px;color: #d8e85f;}");
client.println(".text5{font-weight: 180; padding-left: 15px; font-size: 50px; width: 250px; text-align: left; color: #fcfdff;}");
client.println(".data5{font-weight: 180; padding-left: 80px; font-size: 50px;color: #d8e85f;}");
client.println(".text6{font-weight: 180; padding-left: 15px; font-size: 50px; width: 250px; text-align: left; color: #fcfdff;}");
client.println(".data6{font-weight: 180; padding-left: 80px; font-size: 50px;color: #d8e85f;}");
client.println(".text7{font-weight: 180; padding-left: 15px; font-size: 50px; width: 250px; text-align: left; color: #fcfdff;}");
client.println(".data7{font-weight: 180; padding-left: 80px; font-size: 50px;color: #d8e85f;}");
//client.println(".text8{font-weight: 180; padding-left: 15px; font-size: 50px; width: 170px; text-align: left; color: #fcfdff;}");
//client.println(".data8{font-weight: 180; padding-left: 80px; font-size: 50px;color: #d8e85f;}");
client.println(".data{padding: 10px;}");
client.println("</style>");
client.println("</head>");
client.println("<body>");
client.println("<div id=\"webpage\">");   
client.println("<h1>ESP8266 Portable Weather Station server</h1>");
client.println("<div class=\"data\">");
client.println("<div class=\"side_adjust text1\">Temperature:</div>");
client.println("<div class=\"side_adjust data1\">");
client.print(bmp.readTemperature());
client.println("<div class=\"side_adjust text1\">C</div>");
client.println("</div>");
client.println("<div class=\"data\">");
client.println("<div class=\"side_adjust text2\">Air Pressure:</div>");
client.println("<div class=\"side_adjust data2\">");
client.print(bmp.readPressure());
client.println("<div class=\"side_adjust text2\">Pa</div>");
//client.print(bmp.readPressure());
//client.println("<div class=\"side_adjust text2\">Pa</div>");
client.println("</div>");
client.println("<div class=\"data\">");
client.println("<div class=\"side_adjust text3\">Approximate Altitude:</div>");
client.println("<div class=\"side_adjust data3\">");
client.print(bmp.readAltitude(1013.25));
client.println("<div class=\"side_adjust text3\">m</div>");
client.println("</div>");
client.println("<div class=\"side_adjust text4\">Relative Humidity:</div>");
client.println("<div class=\"side_adjust data4\">");
client.print(hdc1080.readHumidity());
client.println("<div class=\"side_adjust text4\">%</div>");
client.println("</div>");
client.println("<div class=\"side_adjust text5\">UV Light:</div>");
client.println("<div class=\"side_adjust data5\">");
client.print(uv.readUV());
client.println("<div class=\"side_adjust text5\"></div>");
client.println("</div>");
client.println("<div class=\"side_adjust text6\">Carbon Dioxide</div>");
client.println("<div class=\"side_adjust data6\">");
client.print(eco2);
client.println("<div class=\"side_adjust text6\">ppm</div>");
client.println("</div>");
client.println("<div class=\"side_adjust text7\">Volatile Organic Compound</div>");
client.println("<div class=\"side_adjust data7\">");
client.print(etvoc);
client.println("<div class=\"side_adjust text7\">ppb</div>");
client.println("</div>");
//client.println("<div class=\"side_adjust text8\">Temp:</div>");
//client.println("<div class=\"side_adjust data8\">");
//client.print(bmp.readTemperature());
//client.println("<div class=\"side_adjust text8\">C</div>");
//client.println("</div>");
client.println("</div>");
client.println("</body>");
client.println("</html>");
 delay(4000);
}

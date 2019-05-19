//Portable Weather Station ver 3.1
// Sending data to Thing Speak and displays data on OLED screen even not connected to internet. Sensors BMP280, CCS811, HDC1080, VEML1070.

#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include "Adafruit_VEML6070.h"
#include "ClosedCube_HDC1080.h"
#include "ccs811.h"  // CCS811 library
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
Adafruit_SSD1306 display(-1); // Reset pin # (or -1 if sharing Arduino reset pin)

const char* server1 = "api.thingspeak.com";
const char* ssid     = "TuxBot"; // Your ssid
const char* password = "*******"; // Your Password
WiFiServer server(80);
WiFiClient client;
Adafruit_BMP280 bmp; // I2C
Adafruit_VEML6070 uv = Adafruit_VEML6070();
ClosedCube_HDC1080 hdc1080;
CCS811 ccs811(D3); // nWAKE on D3


String apiKey = "USCU4BYE0XLJ7460"; // Write API key for Thing Speak.
String header;

void setup() {
 
Serial.begin(115200);
 
//delay(10);

display.begin(SSD1306_SWITCHCAPVCC, 0x3C); 
display.clearDisplay();



Serial.print("Connecting to ");
Serial.println(ssid);
WiFi.begin(ssid, password);
 
if (WiFi.status() != WL_CONNECTED) {
delay(500);
Serial.print(".");

}
 
Serial.println("");
Serial.println("WiFi is connected");


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

// Sends data to ThingSpeak.
void Online(){
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
    
 
    float U = uv.readUV();
    float T = bmp.readTemperature();
    float P = bmp.readPressure();
    float A = bmp.readAltitude(1013.25);
    float H = hdc1080.readHumidity();
    float C = eco2;
    float V = etvoc;

    if (client.connect(server1,80))   //   "184.106.153.149" or api.thingspeak.com
                      {  
                            
                             String postStr = apiKey;
                             postStr +="&field1=";
                             postStr += String(U);
                             postStr +="&field2=";
                             postStr += String(T);
                             postStr +="&field3=";
                             postStr += String(P);
                             postStr +="&field4=";
                             postStr += String(A);
                             postStr +="&field5=";
                             postStr += String(H);
                             postStr +="&field6=";
                             postStr += String(C);
                             postStr +="&field7=";
                             postStr += String(V);
                             
                             postStr += "\r\n\r\n";
 
                             client.print("POST /update HTTP/1.1\n");
                             client.print("Host: api.thingspeak.com\n");
                             client.print("Connection: close\n");
                             client.print("X-THINGSPEAKAPIKEY: "+apiKey+"\n");
                             client.print("Content-Type: application/x-www-form-urlencoded\n");
                             client.print("Content-Length: ");
                             client.print(postStr.length());
                             client.print("\n\n");
                             client.print(postStr);
 
                             Serial.print("UV Light: ");
                             Serial.print(U);
                             Serial.print(" light, Temperature: ");
        display.clearDisplay();
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(0,0);
        display.print("Jitsu Weather Station (Online)");
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(0,25);
        display.print("U: ");
        display.print(U);
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(0,25);
        display.print("      L");
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(0,25);
                             Serial.print(T);
                             Serial.print(" degrees Celcius, Air Pressure: ");
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(0,16);
        display.print("T: ");
        display.print(T);
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(0,16);
        display.print("       c");
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(0,16);
                             Serial.print(P);
                             Serial.print(" hPa, Altitude: ");
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(0,42);
        display.print("P: ");
        display.print(P);
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(0,42);
        display.print("             Pa");
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(0,42);
                             Serial.print(A);
                             Serial.print(" meters, Humidity: ");
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(0,33);
        display.print("A: ");
        display.print(A);
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(0,33);
        display.print("        m");
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(0,33);
                             Serial.print(H);
                             Serial.print(" %, Carbon Dioxide: ");
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(20,16);
        display.print("       H: ");
        display.print(H);
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(30,16);
        display.print("              %");
        display.setTextColor(WHITE);
        display.setTextSize(1);
                             Serial.print(C);
                             Serial.print(" ppm, Volatile Organic Compound: ");
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(20,33);
        display.print("       C: ");
        display.print(C);
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(30,33);
        display.print("             ppm");
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(20,33);
                             Serial.print(V);
        display.setCursor(20,25);
        display.print("       V: ");
        display.print(V);
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(30,25);
        display.print("             ppb");
        display.setTextColor(WHITE);
        display.setTextSize(1);
        display.setCursor(20,25);
        display.display();   
                             Serial.println(" ppb. Send to Thingspeak.");
                        
                        /*} else Offline();
                        {display.display();*/
                        }
          client.stop();
 
          Serial.println("Waiting...");
  
  // thingspeak needs minimum 15 sec delay between updates, i've set it to 30 seconds
  delay(10000);
 
 }
 // Offline Display.
void Offline(){
  display.clearDisplay();
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,0);
  display.println("Jitsu Weather Sensor (Offline)");

   
  display.setTextColor(WHITE);
  display.setTextSize(.5);
  display.setCursor(0,17);
  display.print("T: ");
  display.print(bmp.readTemperature());
  display.setTextColor(WHITE);
  display.setTextSize(.5);
  display.setCursor(0,17);
  display.print("        c");
  display.setTextColor(WHITE);
  display.setTextSize(.5);
  display.setCursor(0,17);
   
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,42);
  display.print("P: ");
  display.print(bmp.readPressure());
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,42);
  display.print("             Pa");
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,42);
  
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,33);
  display.print("A: ");
  display.print(bmp.readAltitude(1013.25));
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,33);
  display.print("        m");
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(0,33);
   
    
   
  display.setTextColor(WHITE);
  display.setTextSize(.5);
  display.setCursor(0,25);
  display.print("U: ");
  display.print(uv.readUV());
  display.setTextColor(WHITE);
  display.setTextSize(.5);
  display.setCursor(0,25);
  display.print("      L");
  display.setTextColor(WHITE);
  display.setTextSize(.5);
  display.setCursor(0,25);
   

  
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(20,17);
  display.print("       H: ");
  display.print(hdc1080.readHumidity());
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(30,17);
  display.print("              %");
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(20,17);
  

// Read CCS811
  uint16_t eco2, etvoc, errstat, raw;
  ccs811.read(&eco2,&etvoc,&errstat,&raw); 
  
  // Print measurement results based on status
  if( errstat==CCS811_ERRSTAT_OK ) { 
  
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(20,33);
  display.print("       C: ");
  display.print(eco2);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(30,33);
  display.print("             ppm");
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(20,33);
  
  display.setCursor(20,25);
  display.print("       V: ");
  display.print(etvoc);
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(30,25);
  display.print("             ppb");
  display.setTextColor(WHITE);
  display.setTextSize(1);
  display.setCursor(20,25);
   
  display.display();
  // Wait
  delay(5000);
}
}

void loop() {
 if (WiFi.status() != WL_CONNECTED) // Checks if Connected to Internet.
 {Offline(); // If not connected online displays Offline data.
 delay(1000);
 }
  else
  {Online(); // If connected, sends data to Thing Speak.
   delay(1000);

  }

 
}

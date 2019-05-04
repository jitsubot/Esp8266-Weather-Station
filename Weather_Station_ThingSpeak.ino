//Portable Weather Station ver 3
#include <ESP8266WiFi.h>
#include <Wire.h>
#include <SPI.h>
#include <Adafruit_BMP280.h>
#include "Adafruit_VEML6070.h"
#include "ClosedCube_HDC1080.h"
#include "ccs811.h"  // CCS811 library
const char* server1 = "api.thingspeak.com";
const char* ssid     = "TuxBot"; // Your ssid
const char* password = "YourPassword"; // Your Password
WiFiServer server(80);
WiFiClient client;
Adafruit_BMP280 bmp; // I2C
Adafruit_VEML6070 uv = Adafruit_VEML6070();
ClosedCube_HDC1080 hdc1080;
CCS811 ccs811(D3); // nWAKE on D3


String apiKey = "USCU4BYE0XLJ7460"; 
String header;

void setup() {
 Serial.begin(115200);
delay(10);
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



void loop() {
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
                             Serial.print(T);
                             Serial.print(" degrees Celcius, Air Pressure: ");
                             Serial.print(P);
                             Serial.print(" hPa, Altitude: ");
                             Serial.print(A);
                             Serial.print(" meters, Humidity: ");
                             Serial.print(H);
                             Serial.print(" %, Carbon Dioxide: ");
                             Serial.print(C);
                             Serial.print(" ppm, Volatile Organic Compound: ");
                             Serial.print(V);
                             Serial.println(" ppb. Send to Thingspeak.");
                        }
          client.stop();
 
          Serial.println("Waiting...");
  
  // thingspeak needs minimum 15 sec delay between updates, i've set it to 30 seconds
  delay(10000);
    
    
 


 WiFiClient client = server.available();   // Listen for incoming clients

  if (client) {                             // If a new client connects,
    Serial.println("New Client.");          // print a message out in the serial port
    String currentLine = "";                // make a String to hold incoming data from the client
    while (client.connected()) {            // loop while the client's connected
      if (client.available()) {             // if there's bytes to read from the client,
        char c = client.read();             // read a byte, then
        Serial.write(c);                    // print it out the serial monitor
        header += c;
        if (c == '\n') {                    // if the byte is a newline character
          // if the current line is blank, you got two newline characters in a row.
          // that's the end of the client HTTP request, so send a response:
          if (currentLine.length() == 0) {
            // HTTP headers always start with a response code (e.g. HTTP/1.1 200 OK)
            // and a content-type so the client knows what's coming, then a blank line:
            client.println("HTTP/1.1 200 OK");
            client.println("Content-type:text/html");
            client.println("Connection: close");
            client.println("Refresh: 5");  // update the page after 10 sec
            client.println();
            
            // Display the HTML web page
            client.println("<!DOCTYPE html><html>");
            client.println("<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1\">");
            client.println("<link rel=\"icon\" href=\"data:,\">");
            // CSS to style the table 
            client.println("<style>body { text-align: center; font-family: \"Trebuchet MS\", Arial;}");
            client.println("table { border-collapse: collapse; width:45%; margin-left:auto; margin-right:auto; }");
            client.println("th { padding: 12px; background-color: #0043af; color: white; }");
            client.println("tr { border: 1px solid #ddd; padding: 12px; }");
            client.println("tr:hover { background-color: #bcbcbc; }");
            client.println("td { border: none; padding: 12px; }");
            client.println(".sensor { color:white; font-weight: bold; background-color: #bcbcbc; padding: 5px; }");
            
            // Web Page Heading
            client.println("</style></head><body><h1>ESP8266 Portable Weather Station</h1>");
            client.println("<table><tr><th>MEASUREMENT</th><th>VALUE</th></tr>");
            client.println("<tr><td>Temp. Celsius</td><td><span class=\"sensor\">");
            client.println(bmp.readTemperature());
            client.println("   *C</span></td></tr>");  
            client.println("<tr><td>Temp. Fahrenheit</td><td><span class=\"sensor\">");
            client.println(1.8 * bmp.readTemperature() + 32);
            client.println("    *F</span></td></tr>");       
            client.println("<tr><td>Pressure</td><td><span class=\"sensor\">");
            client.println(bmp.readPressure() / 100.0F);
            client.println("      hPa</span></td></tr>");
            client.println("<tr><td>Approx. Altitude</td><td><span class=\"sensor\">");
            client.println(bmp.readAltitude(1013.25));
            client.println("      m</span></td></tr>"); 
            client.println("<tr><td>UV Light</td><td><span class=\"sensor\">");
            client.println(uv.readUV());
            client.println(" </span></td></tr>"); 
            client.println("<tr><td>Humidity</td><td><span class=\"sensor\">");
            client.println(hdc1080.readHumidity());
            client.println("     %</span></td></tr>"); 
            client.println("<tr><td>Carbon Dioxide</td><td><span class=\"sensor\">");
            client.println(eco2);
            client.println("     ppm</span></td></tr>"); 
            client.println("<tr><td>Volatile Organic Compound</td><td><span class=\"sensor\">");
            client.println(etvoc);
            client.println("     ppb</span></td></tr>"); 
            

            
            
            client.println("</body></html>");
            
            // The HTTP response ends with another blank line
            client.println();
            // Break out of the while loop
            break;
          } else { // if you got a newline, then clear currentLine
            currentLine = "";
          }
        } else if (c != '\r') {  // if you got anything else but a carriage return character,
          currentLine += c;      // add it to the end of the currentLine
        }
      }
    }
    // Clear the header variable
    header = "";
    // Close the connection
    client.stop();
    Serial.println("Client disconnected.");
    Serial.println("");
  }
}

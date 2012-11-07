
#include "Adafruit_WS2801.h"
#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

IPAddress ip(10,10,100, 45);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

int dataPin  = 2;    // Yellow wire on Adafruit Pixels
int clockPin = 3;    // Green wire on Adafruit Pixels

int ledcount = 6;
Adafruit_WS2801 strip = Adafruit_WS2801(ledcount, dataPin, clockPin);

int OFF = 0;
int SUCCESS = 1;
int FAILED = 2;
int DEMO = 3;

int setState = SUCCESS;
int state = DEMO;
boolean building = false;

void setup() {
  
  strip.begin();
  // Update LED contents, to start they are all 'off'
  strip.show();
  
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
   while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  // start the Ethernet connection and the server:
  Ethernet.begin(mac, ip);
  server.begin();
  Serial.print("server is at ");
  Serial.println(Ethernet.localIP());
}

int iL =0;
int jL =0;
long lastmilli=0;

uint32_t c1 = Color(0,0,0);
uint32_t c2 = Color(0,0,255);

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  
  if (building) {
    // We are in pulse mode, so do we have a tick of the clock 
    // so to move onto the next light
    if (millis() - lastmilli > 200) {
      if (iL == 0) {
        strip.setPixelColor(5, c2);
      }
      else {
        strip.setPixelColor(iL-1, c2);
      }
       
      strip.setPixelColor(iL, c1);
      strip.show();
      
      iL++;
      if (iL >= 6) iL=0;
      lastmilli = millis();
    }
  }    
  else {
     if (state == DEMO) {
        
        if (millis() - lastmilli > 20) {
            int i;
            for (i=0; i < strip.numPixels(); i++) {
              // tricky math! we use each pixel as a fraction of the full 96-color wheel
              // (thats the i / strip.numPixels() part)
              // Then add in j which makes the colors go around per pixel
              // the % 96 is to make the wheel cycle around
              
              // Simple rainbow
//              strip.setPixelColor(i, Wheel( (i + jL) % 255));

              // Offset complex rainbow
              strip.setPixelColor(i, Wheel( ((i * 256 / strip.numPixels()) + jL) % 256) );
            }  
            strip.show();   // write all the pixels out
         
            jL++;   
            if (jL >= 256) {
              jL = 0;
            }
            lastmilli = millis();
        }
      }
      else {
        colorWipe(c2, 50);
      }
  }
  
  if (client) {    
  
    Serial.println("new client");
    // an http request ends with a blank line
    boolean currentLineIsBlank = true;
    String line = String ();
    
    while (client.connected()) {
      if (client.available()) {
        char c = client.read();
        
        line = line+c;
        Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n') {
          
          if (line.startsWith("GET")) {
             if (line.indexOf("success") >= 0) {
               state = SUCCESS;
               c2 = Color(0, 255, 0);
             }
             else if (line.indexOf("failed") >= 0) {
               state = FAILED;
               c2 = Color(255, 0, 0);
             }
             else if (line.indexOf("demo") >= 0) {
               state = DEMO;
               c2 = Color(0, 0, 255);
             }
             else if (line.indexOf("off") >= 0) {
               state = OFF;
               c2 = Color(0, 0, 0);
               building = false;
             }
             
             if (line.indexOf("building") >= 0) {
               building = true;
             }
             else if (line.indexOf("complete") >= 0) {
               building = false;
             }
          }
          
          if (currentLineIsBlank) {
            
            // send a standard http response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connnection: close");
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html><body>");

            client.print("Build currently is");
            if (state == OFF) client.print(" Off");       
            if (state == SUCCESS) client.print(" SUCCESSFUL");       
            if (state == FAILED) client.print(" FAILED");       
            if (state == DEMO) client.print(" in DEMO mode");       
            
            if (building) client.print("<br/> and <b>is building</b>.");
            client.println("<br />");
            client.println("</body></html>");
            //updateStrip();
            break;
          }
        }
        if (c == '\n') {
          // you're starting a new line
          currentLineIsBlank = true;
        } 
        else if (c != '\r') {
          // you've gotten a character on the current line
          currentLineIsBlank = false;
        }
      }
    }
    // give the web browser time to receive the data
    delay(1);
    // close the connection:
    client.stop();
    Serial.println("client disconnected");
  }
}

// fill the dots one after the other with said color
// good for testing purposes
void colorWipe(uint32_t c, uint8_t wait) {
  int i;
  
  for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, c);
      strip.show();
      delay(wait);
  }
}

void colorWipeDown(uint32_t c, uint8_t wait) {
  int i;
  
  for (i=strip.numPixels(); i >0; i--) {
      strip.setPixelColor(i-1, c);
      strip.show();
      delay(wait);
  }
}

void colorChase(uint32_t c1, uint8_t wait) {
  colorChase(c1, Color(0, 0, 0), wait);
}

void colorChase(uint32_t c1, uint32_t c2, uint8_t wait) {
  int i;
  
  for (i=0; i < strip.numPixels(); i++) {
    if (i > 0) {
      strip.setPixelColor(i-1, c2);
    }
     
    strip.setPixelColor(i, c1);
    strip.show();
    delay(wait);
  }
  
  strip.setPixelColor(strip.numPixels()-1, c2);
}


void rainbow(uint8_t wait) {
  int i, j;
   
  for (j=0; j < 256; j++) {     // 3 cycles of all 256 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      strip.setPixelColor(i, Wheel( (i + j) % 255));
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}

// Slightly different, this one makes the rainbow wheel equally distributed 
// along the chain
void rainbowCycle(uint8_t wait) {
  int i, j;
  
  for (j=0; j < 256 * 5; j++) {     // 5 cycles of all 25 colors in the wheel
    for (i=0; i < strip.numPixels(); i++) {
      // tricky math! we use each pixel as a fraction of the full 96-color wheel
      // (thats the i / strip.numPixels() part)
      // Then add in j which makes the colors go around per pixel
      // the % 96 is to make the wheel cycle around
      strip.setPixelColor(i, Wheel( ((i * 256 / strip.numPixels()) + j) % 256) );
    }  
    strip.show();   // write all the pixels out
    delay(wait);
  }
}


/* Helper functions */

// Create a 24 bit color value from R,G,B
uint32_t Color(byte r, byte g, byte b)
{
  uint32_t c;
  c = r;
  c <<= 8;
  c |= g;
  c <<= 8;
  c |= b;
  return c;
}

//Input a value 0 to 255 to get a color value.
//The colours are a transition r - g -b - back to r
uint32_t Wheel(byte WheelPos)
{
  if (WheelPos < 85) {
   return Color(WheelPos * 3, 255 - WheelPos * 3, 0);
  } else if (WheelPos < 170) {
   WheelPos -= 85;
   return Color(255 - WheelPos * 3, 0, WheelPos * 3);
  } else {
   WheelPos -= 170; 
   return Color(0, WheelPos * 3, 255 - WheelPos * 3);
  }
}

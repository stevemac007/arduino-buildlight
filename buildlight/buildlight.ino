
#include "Adafruit_WS2801.h"
#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//IPAddress ip(10,10,100, 45);
IPAddress ip(192, 168, 1, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

int dataPin  = 2;    // Yellow wire on Adafruit Pixels
int clockPin = 6;    // Green wire on Adafruit Pixels

int ledcount = 6;
Adafruit_WS2801 strip = Adafruit_WS2801(ledcount, dataPin, clockPin);

int OFF = 0;
int SUCCESS = 1;
int FAILED = 2;
int DEMO = 3;

long lastmilli;

uint32_t c1[3] = {Color(0,0,0), Color(0,0,0), Color(0,0,0)};
uint32_t c2[3] = {Color(0,0,0), Color(0,0,0), Color(0,0,0)};

int setState[3] = {OFF, OFF, OFF};
int state[3] = {DEMO, DEMO, DEMO};
boolean building[3] = {false, false, false};
long lastAction[3] = {0, 0, 0};

int activeLed[3] = {0, 0, 0};
int activeCycle[3] = {0, 0, 0};

// Set 0 = 2,3
// Set 1 = 1,4
// Set 2 = 0,5
int leds[3][2] = { {2,3}, {1,4}, {0,5} };

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

void loop() {
  // listen for incoming clients
  EthernetClient client = server.available();
  
  if (millis() - lastmilli > 20) {
    eventFired(0);
    eventFired(1);
    eventFired(2);
    lastmilli = millis();
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
        //Serial.write(c);
        // if you've gotten to the end of the line (received a newline
        // character) and the line is blank, the http request has ended,
        // so you can send a reply
        if (c == '\n') {
          
          int stateL = -1;
          uint32_t c2L = Color(0, 0, 0);
          boolean buildingL = false;
          
          if (line.startsWith("GET")) {
             if (line.indexOf("success") >= 0) {
               stateL = SUCCESS;
               c2L = Color(0, 255, 0);
             }
             else if (line.indexOf("failed") >= 0) {
               stateL = FAILED;
               c2L = Color(255, 0, 0);
             }
             else if (line.indexOf("demo") >= 0) {
               stateL = DEMO;
               c2L = Color(0, 0, 255);
             }
             else if (line.indexOf("off") >= 0) {
               stateL = OFF;
               c2L = Color(0, 0, 0);
               buildingL = false;
             }
             
             if (line.indexOf("building") >= 0) {
               buildingL = true;
             }
             else if (line.indexOf("complete") >= 0) {
               buildingL = false;
             }
          }
          
          if (line.indexOf("S1") >= 0) {
            state[0] = stateL;
            c2[0] = c2L;
            building[0] = buildingL;
          }

          if (line.indexOf("S2") >= 0) {
            state[1] = stateL;
            c2[1] = c2L;
            building[1] = buildingL;
          }

          if (line.indexOf("S3") >= 0) {
            state[2] = stateL;
            c2[2] = c2L;
            building[2] = buildingL;
          }
          
          if (currentLineIsBlank) {
            
            // send a standard http response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connnection: close");
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html><body>");

            client.print("Segments are currently :<br/> <ol>");
            
            for (int i=0; i<3; i++) {
              client.print ("<li> Segment ");
              
              if (state[i] == OFF) client.print(" Off");       
              if (state[i] == SUCCESS) client.print(" SUCCESSFUL");       
              if (state[i] == FAILED) client.print(" FAILED");       
              if (state[i] == DEMO) client.print(" in DEMO mode");       
              
              if (building[i]) client.print("<br/> and <b>is building</b>.");
              client.println("</li>");
            }
            client.println("</ol> <br />");
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

void eventFired(int index) {

  if (building[index]) {
    
    if (millis() - lastAction[index] > 1000) {
      // We are in pulse mode, so do we have a tick of the clock 
      // so to move onto the next light
      if (activeLed[index] == 0) {
        strip.setPixelColor(ledIdx(index, 1), c2[index]);
      }
      else {
        strip.setPixelColor(ledIdx(index, activeLed[index]-1), c2[index]);
      }
       
      strip.setPixelColor(ledIdx(index, activeLed[index]), c1[index]);
      strip.show();
      
      activeLed[index]++;
      if (activeLed[index] >= 2) activeLed[index]=0;
      lastAction[index] = millis();
    }
  }    
  else {
    if (state[index] == DEMO) {
      for (int i=0; i < 2; i++) {
          // tricky math! we use each pixel as a fraction of the full 96-color wheel
          // (thats the i / strip.numPixels() part)
          // Then add in j which makes the colors go around per pixel
          // the % 96 is to make the wheel cycle around
          
          // Simple rainbow
//              strip.setPixelColor(i, Wheel( (i + jL) % 255));

          // Offset complex rainbow
        strip.setPixelColor(ledIdx(index, i), Wheel( ((i * 256 / strip.numPixels()) + activeCycle[index]) % 256) );
      }  
      strip.show();   // write all the pixels out
     
      activeCycle[index]++;
      if (activeCycle[index] >= 256) {
        activeCycle[index] = 0;
      }
    }
    else if (setState[index] != state[index]) {
      colorWipe2(leds[index], c2[index], 50);
      setState[index] = state[index];
    }
  }
}

// Set 0 = 2,3
// Set 1 = 1,4
// Set 2 = 0,5
int ledIdx(int set, int offset) {
  return leds[set][offset];
//  if (set == 0) {
//    if (offset == 0)  return 2; else return 3;
//  }
//  if (set == 1) {
//    if (offset == 0)  return 1; else return 4;
//  }
//  if (set == 2) {
//    if (offset == 0)  return 0; else return 5;
//  }
//  return 0;
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

void colorWipe2(int leds[], uint32_t c, uint8_t wait) {
  int i;
  
  for (i=0; i < 2; i++) {
      strip.setPixelColor(leds[i], c);
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

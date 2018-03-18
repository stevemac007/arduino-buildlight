#include "Adafruit_WS2801.h"
#include <SPI.h>
#include <Ethernet.h>

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


// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//IPAddress ip(10,10,100, 45);
IPAddress ip(192,168,1, 45);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

int dataPin  = 2;    // Yellow wire on Adafruit Pixels
int clockPin = 3;    // Green wire on Adafruit Pixels

int ledcount = 18;
Adafruit_WS2801 strip = Adafruit_WS2801(ledcount, dataPin, clockPin);


const int MODE_OFF = 0;
const int MODE_SUCCESS = 1;
const int MODE_FAILED = 2;
const int MODE_DEMO = 3;
const int MODE_UNK = 4;
const int MODE_PULSE = 5;

// Time between DEMO/PULSE color changes
const int FRAME_RATE = 10;
const int SEGMENT_COUNT = 6;

// Rate of change for building cycle
const int BUILDING_RATE = 1000/SEGMENT_COUNT;

const uint32_t COLOR_DEFAULT = Color(1,1,1);

const uint32_t COLOR_RED = Color(255,0,0);
const uint32_t COLOR_GREEN = Color(0,255,0);
const uint32_t COLOR_BLUE = Color(0,0,255);

const uint32_t COLOR_OFF = Color(0,0,0);

long lastmilli;

uint32_t c1[3] = {Color(0,0,0), Color(0,0,0), Color(0,0,0)};

int state[3] = {MODE_DEMO, MODE_PULSE, MODE_DEMO};
boolean building[3] = {false, false, false};
long lastAction[3] = {0, 0, 0};

int activeLed[3] =   {0, 0, 0};
int activeCycle[3] = {0, 0, 0};
int fadeAmount[3] =  {5, 5, 5};

int leds[3][SEGMENT_COUNT] = { {0,1,2,3,4,5}, {6,7,8,9,10,11}, {12,13,13,15,16,17} };

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
  
  if (millis() - lastmilli > FRAME_RATE) {
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
          uint32_t c2L = COLOR_DEFAULT;
          boolean buildingL = false;
          
          if (line.startsWith("GET")) {
             if (line.indexOf("success") >= 0) {
               stateL = MODE_SUCCESS;
               c2L = COLOR_GREEN;
               Serial.println("SUCCESS seen");
             }
             else if (line.indexOf("failed") >= 0) {
               stateL = MODE_FAILED;
               c2L = COLOR_RED;
               Serial.print("FAILED seen");               
             }
             else if (line.indexOf("unknown") >= 0) {
               stateL = MODE_UNK;
               c2L = COLOR_BLUE;
               Serial.print("UNKNOWN seen");               
             }
             else if (line.indexOf("demo") >= 0) {
               stateL = MODE_DEMO;
               c2L = COLOR_OFF;
               Serial.print("demo seen");               
             }
             else if (line.indexOf("pulse") >= 0) {
               stateL = MODE_PULSE;
               c2L = COLOR_OFF;
               Serial.print("pulse seen");               
             }
             else if (line.indexOf("off") >= 0) {
               stateL = MODE_OFF;
               c2L = COLOR_OFF;
               buildingL = false;
               Serial.print("off seen");               
             }
             
             if (line.indexOf("building") >= 0) {
               buildingL = true;
               Serial.println("building seen");
             }
             else if (line.indexOf("complete") >= 0) {
               buildingL = false;
               Serial.println("complete seen");
             }
          
            if (line.indexOf("S1") >= 0) {
              if (c2L != COLOR_DEFAULT) { 
                state[0] = stateL;
                c1[0] = c2L;
              }
              building[0] = buildingL;
              Serial.println("S1 seen");
            }
  
            if (line.indexOf("S2") >= 0) {
              if (c2L != COLOR_DEFAULT) { 
                state[1] = stateL;
                c1[1] = c2L;
              }
              building[1] = buildingL;
              Serial.println("S2 seen");
            }
  
            if (line.indexOf("S3") >= 0) {
              if (c2L != COLOR_DEFAULT) { 
                state[2] = stateL;
                c1[2] = c2L;
              }
              building[2] = buildingL;
              Serial.println("S3 seen");
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

            client.print("Segments are currently :<br/> <ol>");
            
            for (int i=0; i<3; i++) {
              client.print ("<li> Segment ");
              
              if (state[i] == MODE_OFF) client.print(" Off");       
              if (state[i] == MODE_SUCCESS) client.print(" SUCCESSFUL");       
              if (state[i] == MODE_FAILED) client.print(" FAILED");       
              if (state[i] == MODE_UNK) client.print(" UNKNOWN");       
              if (state[i] == MODE_PULSE) client.print(" in PULSE mode");       
              if (state[i] == MODE_DEMO) client.print(" in DEMO mode");       
              
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
          line = "";
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

    if (state[index] == MODE_DEMO) {
      for (int i=0; i < SEGMENT_COUNT; i++) {
        strip.setPixelColor(ledIdx(index, i), Wheel( ((i * 256 / strip.numPixels()) + activeCycle[index]) % 256) );
      }  
      strip.show();   // write all the pixels out
     
      activeCycle[index]++;
      if (activeCycle[index] >= 256) {
        activeCycle[index] = 0;
      }
    }
    else if (state[index] == MODE_PULSE) {
      activeCycle[index] += fadeAmount[index];
      
      if (activeCycle[index] >= 255) {
        fadeAmount[index] = fadeAmount[index] * -1;
        activeCycle[index] = 255;
      }
      if (activeCycle[index] <= 0) {
        fadeAmount[index] = fadeAmount[index] * -1;
        activeCycle[index] = 1;
      }
      
      for (int i=0; i < SEGMENT_COUNT; i++) {
        strip.setPixelColor(ledIdx(index, i), Color(0, activeCycle[index], 0));
      }  
      strip.show();   // write all the pixels out
        
    }
    else if (building[index]) {
      if (millis() - lastAction[index] > BUILDING_RATE) {
      // We are in pulse mode, so do we have a tick of the clock 
      // so to move onto the next light

      for (int i=0; i < SEGMENT_COUNT; i++) {
        if (activeLed[index] == i) {
          strip.setPixelColor(ledIdx(index, i), COLOR_OFF);
        }
        else {
          strip.setPixelColor(ledIdx(index, i), c1[index]);
        }
      }
      strip.show();
      
      activeLed[index]++;
      if (activeLed[index] >= SEGMENT_COUNT) activeLed[index]=0;
      lastAction[index] = millis();
    }
  }
  else {
    colorWipe2(leds[index], c1[index], 10);
  }
}

// Set 0 = 2,3
// Set 1 = 1,4
// Set 2 = 0,5
int ledIdx(int set, int offset) {
  return leds[set][offset];
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
  
  for (i=0; i < SEGMENT_COUNT; i++) {
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

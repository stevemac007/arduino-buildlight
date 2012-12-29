
#include "BuildSegment.h"
#include <SPI.h>
#include <Ethernet.h>

// Enter a MAC address and IP address for your controller below.
// The IP address will be dependent on your local network:
byte mac[] = { 
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

//IPAddress ip(10,10,100, 45);
IPAddress ip(192, 168, 1, 177);

// Initialize the Ethernet server library
// with the IP address and port you want to use 
// (port 80 is default for HTTP):
EthernetServer server(80);

int dataPin  = 2;    // Yellow wire on Adafruit Pixels
int clockPin = 6;    // Green wire on Adafruit Pixels

int ledcount = 25;
Adafruit_WS2801 strip = Adafruit_WS2801(ledcount, dataPin, clockPin);

// This is a pretty random pattern with 6 items
//int s1Led[6] = {0,3,1,5,2,4};

int s1Led[6] = {0,1,2,3,4,5};
int s2Led[3] = {6,7,8};
int s3Led[9] = {9,10,11,12,13,14,15,16,17};

BuildSegment s1 = BuildSegment(s1Led, 6, BACKFORWARD);
BuildSegment s2 = BuildSegment(s2Led, 3, LOOP);
BuildSegment s3 = BuildSegment(s3Led, 9, BACKFORWARD);

void setup() {

  strip.begin();
  // Update LED contents, to start they are all 'off'
  strip.show();

  s1.setBuilding(true);
  s1.setMode(FAILED);

  s2.setBuilding(true);
  s2.setMode(SUCCESS);

  s3.setBuilding(true);
  s3.setMode(DEMO);

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

  if (s1.hasChange()) {
    s1.tick(strip);
  }

  if (s2.hasChange()) {
    s2.tick(strip);
  }

  if (s3.hasChange()) {
    s3.tick(strip);
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

          buildmode stateL = OFF;
          boolean buildingL = false;

          if (line.startsWith("GET")) {
            if (line.indexOf("success") >= 0) {
              stateL = SUCCESS;
            }
            else if (line.indexOf("failed") >= 0) {
              stateL = FAILED;
            }
            else if (line.indexOf("demo") >= 0) {
              stateL = DEMO;
            }
            else if (line.indexOf("off") >= 0) {
              stateL = OFF;
              buildingL = false;
            }

            if (line.indexOf("building") >= 0) {
              buildingL = true;
            }
            else if (line.indexOf("complete") >= 0) {
              buildingL = false;
            }
          }

//          if (line.indexOf("S1") >= 0) {
//            s1.setBuilding(buildingL);
//            s1.setMode(stateL);
//          }

          //          if (line.indexOf("S2") >= 0) {
          //              s2.setBuilding(buildingL);
          //              s2.setMode(stateL);
          //          }

          //          if (line.indexOf("S3") >= 0) {
          //              s3.setBuilding(buildingL);
          //              s3.setMode(stateL);
          //          }

          if (currentLineIsBlank) {

            // send a standard http response header
            client.println("HTTP/1.1 200 OK");
            client.println("Content-Type: text/html");
            client.println("Connnection: close");
            client.println();
            client.println("<!DOCTYPE HTML>");
            client.println("<html><body>");

            client.print("Segments are currently :<br/> <ol>");

//            client.print ("<li> Segment ");
//            client.print(s1.getStatus());
//            client.println("</li>");
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


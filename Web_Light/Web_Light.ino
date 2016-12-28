#include "SPI.h"
#include "Ethernet.h"
#include "WebServer.h"
#include <EEPROM.h>

static uint8_t mac[6] = { 0x02, 0xAA, 0xBB, 0xCC, 0x00, 0x22 };
IPAddress ip(192, 168, 80, 5);
IPAddress myDns(192, 168, 80, 1);
IPAddress gateway(192, 168, 80, 1);
IPAddress subnet(255, 255, 0, 0);

#define PREFIX ""
#define MIN 1
#define DHCPREQ 30000 //ms delay
#define DHCPRES 5000 //ms delay
#define OUT_PIN 5
#define ON_PIN 6
#define L1ADDR 0 
#define L2ADDR 1 
#define L3ADDR 2 
#define L4ADDR 3
#define DELADDR 4  

WebServer webserver(PREFIX, 80);

unsigned long int val = 0;            //integer for brightness level
byte l1 = 1,l2 = 127,l3 = 191,l4=255;
unsigned int off_delay = 600; //interval for on in seconds
unsigned long int prev=0,cur=0;
unsigned long int prevval=val;
//-----------------------------------------------------------------------------------------------------------
void ctrlCmd(WebServer &server, WebServer::ConnectionType type, char *, bool)
{
  if (type == WebServer::POST)
  {
    bool repeat;
    char name[16], value[16];
    do
    {  repeat = server.readPOSTparam(name, 16, value, 16);
       if(strcmp(value, "") != 0)
       {
       if (strcmp(name, "val") == 0)
          {
          val = strtoul(value, NULL, 10);
          }
       if (strcmp(name, "l1") == 0)
          {
          int var=strtoul(value, NULL, 10);
          l1 = constrain(255*var/100,MIN,255);
          EEPROM.update(L1ADDR, l1);
          }
       if (strcmp(name, "l2") == 0)
          {
          int var=strtoul(value, NULL, 10);
          l2 = constrain(255*var/100,l1,255);
          EEPROM.update(L2ADDR, l2);
          }
       if (strcmp(name, "l3") == 0)
          {
          int var=strtoul(value, NULL, 10);
          l3 = constrain(255*var/100,l2,255);
          EEPROM.update(L3ADDR, l3);
          }
        if (strcmp(name, "l4") == 0)
          {
          int var=strtoul(value, NULL, 10);
          l4 = constrain(255*var/100,l3,255);
          EEPROM.update(L4ADDR, l4);
          }
        if (strcmp(name, "del") == 0)
          {
          off_delay = strtoul(value, NULL, 10);
          EEPROM.update(DELADDR, off_delay);
          }
        }
    } while (repeat);
    server.httpSeeOther(PREFIX);
    return;
  }
  server.httpSuccess();
  if (type == WebServer::GET)
  {
    /* store the HTML in program memory using the P macro */
    P(message) = 
"<!DOCTYPE html><html><head>"
  "<title>Light control</title>"
  "<link href='http://ajax.googleapis.com/ajax/libs/jqueryui/1.8.16/themes/base/jquery-ui.css' rel=stylesheet />"
  "<script src='http://ajax.googleapis.com/ajax/libs/jquery/1.6.4/jquery.min.js'></script>"
  "<script src='http://ajax.googleapis.com/ajax/libs/jqueryui/1.8.16/jquery-ui.min.js'></script>"
  "<script>"
  
//custom functions    
    "$(document).ready(function(){ $('.off').click( function(){$.post('/', { val: \"0\" } ); }); });"
    "$(document).ready(function(){ $('.btn1').click( function(){$.post('/', { val: \"1\" } ); }); });"
    "$(document).ready(function(){ $('.btn2').click( function(){$.post('/', { val: \"2\" } ); }); });"
    "$(document).ready(function(){ $('.btn3').click( function(){$.post('/', { val: \"3\" } ); }); });"
    "$(document).ready(function(){ $('.btn4').click( function(){$.post('/', { val: \"4\" } ); }); });"
    "$(document).ready(function(){ $('.cfg').click( function(){$(location).attr('href','/cfg'); }); });"
  "</script>"
"</head>"
"<body>"
  "<div>"
    "<button type=\"button\" class=\"off\" style='font-size:200%;height:100px;width:100px\'>0%</button>"
  "</div>"
  "<div>"
  "<button type=\"button\" class=\"btn1\" style='font-size:200%;height:100px;width:100px\'>25%</button>"
  "</div>"
  "<div>"
  "<button type=\"button\" class=\"btn2\" style='font-size:200%;height:100px;width:100px\'>50%</button>"
  "</div>"
  "<div>"
  "<button type=\"button\" class=\"btn3\" style='font-size:200%;height:100px;width:100px\'>75%</button>"
  "</div>"
  "<div>"
  "<button type=\"button\" class=\"btn4\" style='font-size:200%;height:100px;width:100px\'>100%</button>"
  "</div>"
  "<div>"
  "<button type=\"button\" class=\"cfg\" style='font-size:200%;height:100px;width:100px\'>cfg</button>"
  "</div>"
"</body>"
"</html>";

    server.printP(message);
  }
}
//-----------------------------------------------------------------------------------------------------------
void cfgCmd(WebServer &server, WebServer::ConnectionType type, char *, bool)
{
  if (type == WebServer::POST)
  {
    bool repeat;
    char name[16], value[16];
    do
    {
      /* readPOSTparam returns false when there are no more parameters
       * to read from the input.  We pass in buffers for it to store
       * the name and value strings along with the length of those
       * buffers. */
      repeat = server.readPOSTparam(name, 16, value, 16);

      /* this is a standard string comparison function.  It returns 0
       * when there's an exact match.  We're looking for a parameter
       * named red/green/blue here. */
      if (strcmp(name, "val") == 0)
      {
  /* use the STRing TO Unsigned Long function to turn the string
   * version of the color strength value into our integer red/green/blue
   * variable */
        val = strtoul(value, NULL, 10);
      }
    } while (repeat);
    
    // after procesing the POST data, tell the web browser to reload
    // the page using a GET method. 
    server.httpSeeOther("cfg");
    return;
  }

  /* for a GET or HEAD, send the standard "it's all OK headers" */
  server.httpSuccess();

  /* we don't output the body for a HEAD request */
  if (type == WebServer::GET)
  {
    /* store the HTML in program memory using the P macro */
    P(message) = 
"<!DOCTYPE html><html><head>"
  "<title>Light control config</title>"
"</head>"
"<body>"
  "<form action=\"/\" method=\"post\">"
    "delay:"
    "<input type=\"text\" name=\"del\" value=off_delay ><br>"
    "l1:"
    "<input type=\"text\" name=\"l1\" value=l1 /><br>"
    "l2:"
    "<input type=\"text\" name=\"l2\" /><br>"
    "l3:"
    "<input type=\"text\" name=\"l3\" /><br>"
    "l4:"
    "<input type=\"text\" name=\"l4\" /><br>"
    "<input type=\"submit\" value=\"Set\">"
  "</form>"
"</body>"
"</html>";

    server.printP(message);
  }
}
//-----------------------------------------------------------------------------------------------------------

void setup()
{
  //pins setup
  pinMode(OUT_PIN, OUTPUT);
  analogWrite(OUT_PIN, MIN);
  pinMode(ON_PIN, OUTPUT);
  analogWrite(ON_PIN, 0);

  //get settings from EEPROM
  l1=constrain(EEPROM.read(L1ADDR),MIN,255);
  l2=constrain(EEPROM.read(L2ADDR),l1,255);
  l3=constrain(EEPROM.read(L3ADDR),l2,255);
  l4=constrain(EEPROM.read(L4ADDR),l3,255);
  off_delay=EEPROM.read(DELADDR);
  
  //Open serial
  Serial.begin(115200);
  
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}
  
  Serial.print("light levels from config");
  Serial.print(" l1=");
  Serial.print((l1*100)/255);
  Serial.print("% l2=");
  Serial.print((l2*100)/255);
  Serial.print("% l3=");
  Serial.print((l3*100)/255);
  Serial.print("% l4=");
  Serial.print((l4*100)/255);
  Serial.print("% Delay");
  Serial.print(off_delay);
  Serial.println("s ");
  if (Ethernet.begin(mac,DHCPREQ,DHCPRES) == 0) 
    {
    Serial.println("Failed to  configure Ethernet using DHCP");
    Ethernet.begin(mac, ip);
    }
  printIPAddress();
  webserver.setDefaultCommand(&ctrlCmd);
  webserver.addCommand("cfg", &cfgCmd);
  webserver.begin();
  prev=millis();
  }
//-----------------------------------------------------------------------------------------------------------
void loop()
  {
  webserver.processConnection();
  //Ethernet.maintain();
  cur=millis();
//-------------------Log light start with parameters if it starts
  if ((val!=prevval)&&(val!=0))
    { 
     Serial.print("Turning light on for ");
     Serial.print(off_delay);
     Serial.print("s ");
     }
  if(val!=prevval){
    prev=millis();
    cur=millis();
    prevval=val;
  }
  else{
    if((val!=0)&&((cur-prev)>=(off_delay*1000))){
      val=0;
      prevval=val;
      prev=millis();
      cur=millis();
    }
  }
  switch(val){
    case 0:
       analogWrite(ON_PIN,0);
       analogWrite(OUT_PIN, MIN);
     break;
    case 1:
      analogWrite(OUT_PIN, l1);
      digitalWrite(ON_PIN,HIGH);
     break;
    case 2:
      analogWrite(OUT_PIN, l2);
      digitalWrite(ON_PIN,HIGH);
     break;
    case 3:
      analogWrite(OUT_PIN, l3);
      digitalWrite(ON_PIN,HIGH);
     break;
    case 4:
      analogWrite(OUT_PIN, l4);
      digitalWrite(ON_PIN,HIGH);
     break;
  }
  
}

void printIPAddress()
{
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
  }

  Serial.println();
}

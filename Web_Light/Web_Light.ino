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
unsigned long int prevval=val;
unsigned int l[6]= {0,1,127,191,255,600}; //val,l1,l2,l3,l4,off_delay

unsigned int off_delay = 600; //interval for on in seconds
unsigned long int prev=0,cur=0; //timestamps

//-----------------------------------------------------------------------------------------------------------
void ctrlCmd(WebServer &server, WebServer::ConnectionType type, char *, bool)
{
  if (type == WebServer::POST)
  {
 //   bool repeat;
 //   char p_name[16], value[16];
//------------------------Eeprom update config-----------------------
   /* do
    {  repeat = server.readPOSTparam(p_name, 16, value, 16);
       if(strcmp(value, "") != 0)
       {
       if (strcmp(p_name, "l0") == 0)
          {
          val = strtoul(value, NULL, 10);
          l[0]=val;
          }
       if (strcmp(p_name, "l1") == 0)
          {
          int var=strtoul(value, NULL, 10);
          l[1] = constrain(255*var/100,MIN,255);
          EEPROM.update(L1ADDR, l[1]);
          }
       if (strcmp(p_name, "l2") == 0)
          {
          int var=strtoul(value, NULL, 10);
          l[2] = constrain(255*var/100,l[1],255);
          EEPROM.update(L2ADDR, l[2]);
          }
       if (strcmp(p_name, "l3") == 0)
          {
          int var=strtoul(value, NULL, 10);
          l[3] = constrain(255*var/100,l[2],255);
          EEPROM.update(L3ADDR, l[3]);
          }
        if (strcmp(p_name, "l4") == 0)
          {
          int var=strtoul(value, NULL, 10);
          l[4] = constrain(255*var/100,l[3],255);
          EEPROM.update(L4ADDR, l[4]);
          }
        if (strcmp(p_name, "del") == 0)
          {
          off_delay = strtoul(value, NULL, 10);
          l[5]=off_delay;
          EEPROM.update(DELADDR, off_delay);
          }
        }
    } while (repeat);*/
 /*unsigned short i=0;
 Serial.println("Eeprom config upgrade");
      do{
      Serial.print("l");
      Serial.print(i);
      Serial.print("=");
      Serial.print((100*l[i])/255);
      Serial.print("");
      
      i++;
      }while(i<6);
      Serial.println();*/
      
      server.httpSeeOther(PREFIX);
    return;
  }
  server.httpSuccess();
  
//-----------------------------web push button-------------------  
  if (type == WebServer::GET)
  {
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
  "<table><tr>"
  "<td width=15% align=center>"
    "<button type=\"button\" class=\"off\" style='font-size:200%;height:100px;width:100px\'>0%</button>"
  "</td>"
  "<td width=15% align=center>"
  "<button type=\"button\" class=\"btn1\" style='font-size:200%;height:100px;width:100px\'>25%</button>"
  "</td>"
  "<td width=15% align=center>"
  "<button type=\"button\" class=\"btn2\" style='font-size:200%;height:100px;width:100px\'>50%</button>"
  "</td>"
  "<td width=15% align=center>"
  "<button type=\"button\" class=\"btn3\" style='font-size:200%;height:100px;width:100px\'>75%</button>"
  "</td>"
  "<td width=15% align=center>"
  "<button type=\"button\" class=\"btn4\" style='font-size:200%;height:100px;width:100px\'>100%</button>"
  "</td>"
  "<td width=15% align=center>"
  "<button type=\"button\" class=\"cfg\" style='font-size:200%;height:100px;width:100px\'>cfg</button>"
  "</td></tr></table>";
  server.printP(message);
  l[0]=val;
  server.print("<table><tr><td align=center width=50%>current level=");
  server.print((100*l[0])/255);
  server.print("</td><td width=50% align=center> timeout=");
  server.println(l[5]);
  server.print("</td><tr></table>");
  
  
  server.print("<table width=100% border=1><tr>");
  server.print("Light levels from config: ");
  for (short int i=1;i<=4;i++)
  {
  server.print("<td align=center width=20%>");
  server.print("l");
  server.print(i);
  server.print("=");
  server.print((l[i]*100)/255);
  server.print("% ");
  server.print("</td>");
  
  }
  server.print("<td align=center width=20%>");
  server.print(" Delay");
  server.print("=");
  server.print(l[6]);
  server.println("s ");
  server.print("</td>");
  server.print("</table>");
  server.print("</body></html>");
  }
}
//--------------------------Configuration page---------------------------------------------------------------------------------
void cfgCmd(WebServer &server, WebServer::ConnectionType type, char *, bool)
  {
    
  if (type == WebServer::POST)
    {
    bool repeat;
    char p_name[16],value[16];
    unsigned int var;
    unsigned short int vi=0;
    Serial.println("Config form read");
    do
      {
      repeat = server.readPOSTparam(p_name, 16, value, 16);
      
      unsigned short i=0;
      do{
      Serial.print(p_name[i]);i++;}while(p_name[i]);
      Serial.print("=");
      i=0;
      do{Serial.print(value[i]);i++;}while(value[i]);
      Serial.println();
      
      //if(vi>0)
       // {    
        var=strtol(value, NULL, 10);
        Serial.print("var=");
        Serial.print(var);
        l[vi+1] = constrain(var,0,100);
        Serial.print(" Writing eeprom l");
        Serial.print(vi+1);
        Serial.print("=");
        Serial.print(l[vi+1]);
        Serial.println();
        EEPROM.update(vi,l[vi+1]);
      //  }
      vi++;
      if (strcmp(p_name, "val") == 0)
        {
        val = strtoul(value, NULL, 10);
        }
      } while (repeat);
   
    server.httpSeeOther("cfg");
    return;
    }
  server.httpSuccess();
  
  if (type == WebServer::GET)
    {
    server.print("<!DOCTYPE html><html><head><title>Light control config</title></head><body><form action=\\cfg method=post>");
    l[0]=0;
    for(unsigned short int i=1;i<=4;i++){
    server.print("l");server.print(i);server.print(":<input type=text name=l");server.print(i);server.print(" value=");server.print((l[i]*100)/255);server.print(">%<br>");
    }
    server.print("delay:<input type=text name=del value=");server.print(l[5]);server.print(" ><br>");
    server.print("<input type=submit value=set></form>");
    server.print("<form action=\\><button type=submit>Home</button></form>");
    server.print("</body></html>");
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
  l[1]=constrain(EEPROM.read(L1ADDR),MIN,255);
  l[2]=constrain(EEPROM.read(L2ADDR),l[2],255);
  l[3]=constrain(EEPROM.read(L3ADDR),l[3],255);
  l[4]=constrain(EEPROM.read(L4ADDR),l[4],255);
  off_delay=EEPROM.read(DELADDR);
  l[5]=off_delay;
  //Open serial
  Serial.begin(115200);
  
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}
  Serial.print("light levels from config:");
  for(short int i=1;i<=4;i++)
    {
  Serial.print(" l");
  Serial.print(i);
  Serial.print("=");
  Serial.print((l[i]*100)/255);
  Serial.print("%");
    }
  Serial.print(" Delay");
  Serial.print("=");
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
     Serial.println("s ");
     }
//------------------- Remember Current State     
  if(val!=prevval){
    prev=millis();
    cur=millis();
    prevval=val;
  }
//------------------- If Remembered state - check timeout delay  
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
      analogWrite(OUT_PIN, l[1]);
      digitalWrite(ON_PIN,HIGH);
     break;
    case 2:
      analogWrite(OUT_PIN, l[2]);
      digitalWrite(ON_PIN,HIGH);
     break;
    case 3:
      analogWrite(OUT_PIN, l[3]);
      digitalWrite(ON_PIN,HIGH);
     break;
    case 4:
      analogWrite(OUT_PIN, l[4]);
      digitalWrite(ON_PIN,HIGH);
     break;
  }
  
}

void printIPAddress()
  {
  Serial.print("My IP address: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) 
    {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
    }

  Serial.println();
  }

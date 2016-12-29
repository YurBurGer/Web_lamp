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
#define L1ADDR 1 
#define L2ADDR 2 
#define L3ADDR 3 
#define L4ADDR 4
#define DELADDR 5  

WebServer webserver(PREFIX, 80);
const char print_td[]="<td align=center width=15%>";
unsigned long int val = 0;            //integer for brightness level
unsigned long int prevval=val;
unsigned long int l[6]= {0,0,0,0,255,600}; //l0,l1,l2,l3,l4,off_delay
unsigned long int prev=0,cur=0,period=0; //timestamps

//-----------------------------------------------------------------------------------------------------------
void ctrlCmd(WebServer &server, WebServer::ConnectionType type, char *, bool)
  {
  if (type == WebServer::POST)
    {
    bool repeat;
    
//---------------------read post parameters & set val--------    
    char name[16], value[16];
    do
      {
      repeat = server.readPOSTparam(name, 16, value, 16);
      if(strcmp(value, "") != 0)
        {
        if (strcmp(name, "val") == 0)
          {
          val = strtoul(value, NULL, 10);
          }
        }
      } while (repeat);
      server.httpSeeOther(PREFIX);
    return;
    }
  server.httpSuccess();
  
//-----------------------------web push button print-------------------  
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
    "$(document).ready(function(){ $('.btn0').click( function(){$.post('/', { val: \"0\" } ); }); });"
    "$(document).ready(function(){ $('.btn1').click( function(){$.post('/', { val: \"1\" } ); }); });"
    "$(document).ready(function(){ $('.btn2').click( function(){$.post('/', { val: \"2\" } ); }); });"
    "$(document).ready(function(){ $('.btn3').click( function(){$.post('/', { val: \"3\" } ); }); });"
    "$(document).ready(function(){ $('.btn4').click( function(){$.post('/', { val: \"4\" } ); }); });"
    "$(document).ready(function(){ $('.btn5').click( function(){$(location).attr('href','/cfg'); }); });"
  "</script>"
"</head>"
"<body>"
"<table width=100%><tr><td width=15% align=center>"
    "<button type=\"button\" class=\"btn0\" style='font-size:200%;height:100px;width:100px\'>L0</button>"
  "</td>"
  "<td width=15% align=center>"
  "<button type=\"button\" class=\"btn1\" style='font-size:200%;height:100px;width:100px\'>L1</button>"
  "</td>"
  "<td width=15% align=center>"
  "<button type=\"button\" class=\"btn2\" style='font-size:200%;height:100px;width:100px\'>L2</button>"
  "</td>"
  "<td width=15% align=center>"
  "<button type=\"button\" class=\"btn3\" style='font-size:200%;height:100px;width:100px\'>L3</button>"
  "</td>"
  "<td width=15% align=center>"
  "<button type=\"button\" class=\"btn4\" style='font-size:200%;height:100px;width:100px\'>L4</button>"
  "</td>"
  "<td width=15% align=center>"
  "<button type=\"button\" class=\"btn5\" style='font-size:200%;height:100px;width:100px\'>cfg</button>"
  "</td>"
"</tr>";

  server.printP(message);
//  server.print("<table width=100% border=1><tr height=100>"); 
 /* for (short int i=0;i<=5;i++)
  {
  server.print("<td width=15% align=center><button type=\"button\" class=btn");server.print(i); server.print("style= \' font-size:200%;height:100px;width:100px \' >");server.print((100*l[i])/255);server.print("</button></td>");
  }*/
  server.print("<tr>");
//-----------------------------print data from array----------------------  
  for (short int i=0;i<=4;i++)
  {
  server.print(print_td);
  server.print("L");
  server.print(i);
  server.print("=");
  server.print(l[i]);
  server.print("% ");
  server.print("</td>");
  
  }
  server.print(print_td);
  server.print(" Delay");
  server.print("=");
  server.print(l[5]);
  server.println("s ");
  server.print("</td></tr></table></body></html>");
 
  }
  
  
}
//--------------------------Configuration page---------------------------------------------------------------------------------
void cfgCmd(WebServer &server, WebServer::ConnectionType type, char *, bool)
  {
    
  if (type == WebServer::POST)
    {
    bool repeat;
    char p_name[16],value[16];
    unsigned int var; //value varaible for conversion
    unsigned short int vi=1; //local counter 
    Serial.println("Config form read");
//---------------------------parse post data -----------------------   
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
//-------------------------------print parse log to serial && update eeprom if any difference      
      if(vi>0&&vi<=5)
       {    
        var=strtol(value, NULL, 10);
        Serial.print("var=");
        Serial.print(var);
        if(vi<=4){ Serial.print("%");}
        
        if((vi)<5){l[vi] = constrain(var,0,100);}
        if((vi)==5){l[vi] = constrain(var,0,255);}
        
        Serial.print(" Writing eeprom l");
        Serial.print(vi);
        Serial.print("=");
        Serial.print(l[vi]);
        Serial.print("(");
        if((vi)<5){Serial.print(l[vi]);}
        if(vi<=4){Serial.print("%)");}
        else {Serial.print("s)");}
        Serial.println();
        EEPROM.update(vi,l[vi]);
       }
      vi++;
 /*     if (strcmp(p_name, "val") == 0)
        {
        val = strtoul(value, NULL, 10);
        }*/
      } while (repeat);
    val=0;
    server.httpSeeOther("cfg");
    return;
    }
  server.httpSuccess();
//---------------------------------print config buttons to config web page ---------------------------------------------  
  if (type == WebServer::GET)
    {
    server.print("<!DOCTYPE html><html><head><title>Light control config</title></head><body><form action=\\cfg method=post>");
    l[0]=0;
    for(unsigned short int i=1;i<=4;i++){
    server.print("l");server.print(i);server.print(":<input type=text name=l");server.print(i);server.print(" value=");server.print(l[i]);server.print(">%<br>");
    }
    server.print("delay:<input type=text name=del value=");server.print(l[5]);server.print(" ><br>");
    server.print("<input type=submit value=save></form>");
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
  l[2]=constrain(EEPROM.read(L2ADDR),0,255);
  l[3]=constrain(EEPROM.read(L3ADDR),0,255);
  l[4]=constrain(EEPROM.read(L4ADDR),0,255);
  l[5]=EEPROM.read(DELADDR);
  //Open serial
  Serial.begin(115200);
  
  //while (!Serial) {
  //  ; // wait for serial port to connect. Needed for native USB port only
  //}
  Serial.print("light levels from config:");
  for(short int i=0;i<=4;i++)
    {
  Serial.print(" l");
  Serial.print(i);
  Serial.print("=");
  Serial.print(l[i]);
  Serial.print("%");
    }
  Serial.print(" Delay");
  Serial.print("=");
  Serial.print(l[5]);
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
  cur=millis();
//Ethernet.maintain();
//------------------- Remember Current Times if val changes and Show timeout delay if light turns on  (val!=0)  
  if(val!=prevval)
  {
    if (val!=0)
      {
      period=l[5]*1000;
      Serial.print("on for ");
      Serial.print(l[5]);
      Serial.print("s ");
      Serial.print(" (");
      Serial.print(period);
      Serial.println("ms)");
      }
    prev=millis();
    cur=millis();
    prevval=val;
    Serial.print("val=");
    Serial.println(val);
    }
//------------------- If Remembered state - check timeout delay  
  else{

    if((val!=0)&&((cur-prev)>period)){
      Serial.print("LOFF Timeout:");
      Serial.print(cur-prev/1000);
      Serial.println("s");
      val=0;
   //   prevval=val;
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
      analogWrite(OUT_PIN, ceil((100*l[1])/255));
      digitalWrite(ON_PIN,HIGH);
     break;
    case 2:
      analogWrite(OUT_PIN,ceil((100*l[2])/255));
      digitalWrite(ON_PIN,HIGH);
     break;
    case 3:
      analogWrite(OUT_PIN, ceil((100*l[3])/255));
      digitalWrite(ON_PIN,HIGH);
     break;
    case 4:
      analogWrite(OUT_PIN, ceil((100*l[4])/255));
      digitalWrite(ON_PIN,HIGH);
     break;
  } 
}

void printIPAddress()
  {
  Serial.print("IP: ");
  for (byte thisByte = 0; thisByte < 4; thisByte++) 
    {
    // print the value of each byte of the IP address:
    Serial.print(Ethernet.localIP()[thisByte], DEC);
    Serial.print(".");
    }

  Serial.println();
  }

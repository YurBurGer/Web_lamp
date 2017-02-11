#include "SPI.h"
#include "Ethernet.h"
//#define WEBDUINO_SERIAL_DEBUGGING 1 //uncomment to see recieved requests
//#define WEBDUINO_SERIAL_DEBUGGING 5 //uncomment to see full debug
#include "WebServer.h"
#include <EEPROM.h>

static uint8_t mac[6] = { 0x02, 0xAA, 0xBB, 0xCC, 0x00, 0x22 };
IPAddress ip(192, 168, 80, 5);
IPAddress myDns(192, 168, 80, 1);
IPAddress gateway(192, 168, 80, 1);
IPAddress subnet(255, 255, 0, 0);


#define SERIAL_DEBUGGING 2 //uncomment for full output
//#define SERIAL_DEBUGGING 1 //uncomment for address outpout

#define PREFIX ""
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
byte val = 0;            //integer for brightness level
byte prevval=val;
unsigned int l[6]= {0,0,0,0,255,600}; //l0,l1,l2,l3,l4,off_delay
unsigned long int lamp_start = 0, cur = 0,period = 0; //timestamps


void printCmdPage(WebServer & server){
    P(head) = 
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
    "<button type=\"button\" class=\"btn0\" style='font-size:200%;height:150px;width:150px\'>0%</button>"
  "</td>"
  "<td width=15% align=center>"
  "<button type=\"button\" class=\"btn1\" style='font-size:200%;height:150px;width:150px\'>";
    P(p1)="% </button>"
  "</td>"
  "<td width=15% align=center>"
  "<button type=\"button\" class=\"btn2\" style='font-size:200%;height:150px;width:150px\'>";
    P(p2)="% </button>"
  "</td>"
  "<td width=15% align=center>"
  "<button type=\"button\" class=\"btn3\" style='font-size:200%;height:150px;width:150px\'>";
    P(p3)="% </button>"
  "</td>"
  "<td width=15% align=center>"
  "<button type=\"button\" class=\"btn4\" style='font-size:200%;height:150px;width:150px;\'>";
    P(p4)="% </button>"
  "</td>"
  "<td width=15% align=center>"
  "<button type=\"button\" class=\"btn5\" style='font-size:200%;height:150px;width:150px\'>cfg</button>"
  "</td>"
"</tr>"
"</table></body></html>";
    server.printP(head);
    server.print(l[1]);
    server.printP(p1);
    server.print(l[2]);
    server.printP(p2);
    server.print(l[3]);
    server.printP(p3);
    server.print(l[4]);
    server.printP(p4);   
}

void getValParam(WebServer &server, byte & result){
    char name[16], value[16];
    bool repeat = true;
    do{
      repeat = server.readPOSTparam(name, 16, value, 16);      
      if (!strcmp(name, "val"))
        result = strtoul(value, NULL, 10);   
    } while (repeat);
}

//------------------------------------------------------------------------------
void ctrlCmd(WebServer &server, WebServer::ConnectionType type, char *, bool){
  if (type == WebServer::POST){
    getValParam(server, val);
    server.httpSeeOther(PREFIX);    
  }
  else{  
      server.httpSuccess();
      printCmdPage(server);
  }
}

void printConfigPage(WebServer & server){
    P(head)=
    "<!DOCTYPE html><html><head><title>Light control config</title></head>"
      "<body><form action=\\cfg method=post>";
    P(tail)=
    " >sec<br>"
    "<input type=submit value=save></form>"
    "<form action=\\><button type=submit>Home</button></form>"
    "</body></html>";
    P(input)=":<input type=text name=l";
    server.printP(head);
    for(unsigned short int i=1;i<=4;i++){    
      server.print("l");
      server.print(i);
      server.printP(input);
      server.print(i);
      server.print(" value=");
      server.print(l[i]);
      server.print(">%<br>");
    }
    server.print("delay:<input type=text name=del value=");
    server.print(l[5]);
    server.printP(tail);
}

byte pname_to_vi(char * p_name){
    if (!strcmp(p_name, "l1")
        return 1;
    if (!strcmp(p_name, "l2")
        return 2;
    if (!strcmp(p_name, "l3")
        return 3;
    if (!strcmp(p_name, "l4")
        return 4;
    if (!strcmp(p_name, "del")
        return 5;
    return -1;
}

void getLParams(WebServer & server,unsigned int * l ){
    bool repeat = true;
    do {
      repeat = (server.readPOSTparam(p_name, 16, value, 16));
      #if SERIAL_DEBUGGING > 1
      Serial.print(p_name);      
      Serial.print("=");
      Serial.println(value);
      #endif
      unsigned byte vi = pname_to_vi(p_name);      
      if(vi != -1){    
        var=strtol(value, NULL, 10);
        #if SERIAL_DEBUGGING > 1
          Serial.print("var=");
          Serial.println(var);
        #endif
        
        if(vi<5)
          l[vi] = constrain(var,0,100);
        else if(vi==5)
          l[vi] = constrain(var,0,255);
      }    
    } while(repeat);
}

void update_eeprom(){
  #if SERIAL_DEBUGGING > 1
  Serial.print("Writing to eeprom:");
  #endif
  
  for (int vi = 1; vi <=5; vi++){        
      EEPROM.update(vi, l[vi]);
      #if SERIAL_DEBUGGING > 1
      Serial.print(vi);
      Serial.print("=");
      Serial.print(l[vi]);
      Serial.print(" ");
      #endif
    }

}

//--------------------------Configuration page----------------------------------
void cfgCmd(WebServer &server, WebServer::ConnectionType type, char *, bool){  
  if (type == WebServer::POST){
    char p_name[16],value[16];
    unsigned int var; //value varaible for conversion
    unsigned short int vi=1; //local counter 
    #if SERIAL_DEBUGGING > 1
      Serial.println("Config form read");
    #endif
    getLParams(l);
    update_eeprom();
    val=0;
    server.httpSeeOther("cfg");

  } else{
    server.httpSuccess();  
    printConfigPage();
  }
}
void print_levels(){
f   or(short int i=1;i<=4;i++){
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
}
//-----------------------------------------------------------------------------------------------------------
void setup(){
  //pins setup
  pinMode(OUT_PIN, OUTPUT);
  analogWrite(OUT_PIN, 0);
  pinMode(ON_PIN, OUTPUT);
  analogWrite(ON_PIN, 0);

  //get settings from EEPROM
  for (int addr =L1ADDR; addr <= DELADDR; i++)
    l[addr] = EEPROM.read(addr);

  //Open serial
  Serial.begin(115200);

  #if SERIAL_DEBUGGING > 1
    Serial.print("light levels from config:");
    print_levels();
  #endif
  
  if (Ethernet.begin(mac,DHCPREQ,DHCPRES) == 0){
    #if SERIAL_DEBUGGING > 0
      Serial.println("Failed to  configure Ethernet using DHCP");
    #endif
    Ethernet.begin(mac, ip);
  }
  
  #if SERIAL_DEBUGGING > 0
    Serial.print("IP:");
    Serial.println(Ethernet.localIP());
  #endif
  
  webserver.setDefaultCommand(&ctrlCmd);
  webserver.addCommand("cfg", &cfgCmd);
  webserver.begin();
  
  prev=millis();
}

void sendCurrentLampCommand(){
  if (val == 0){
      analogWrite(ON_PIN, 0);
      digitalWrite(OUT_PIN, 0);   
  } else if (val < 5){
      analogWrite(OUT_PIN, ceil(l[val] * 255 / 100));
      digitalWrite(ON_PIN,HIGH);
  }
}

void processLamps(){
  cur=millis();
  
  if(val != prevval){ // пользователь изменил состояние?
    if (val > 0) // пользователь включил лампу?
      period = l[5]*1000;
    lamp_start = cur;
    prevval = val;
  }
  else // обычная работа
    if( val && ((cur - lamp_start)>period))
      prevval = val = 0;
  
  sendCurrentLampCommand();
}

//-----------------------------------------------------------------------------------------------------------
void loop(){
  webserver.processConnection();
  processLamps();
}

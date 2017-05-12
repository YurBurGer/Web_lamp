#include <SPI.h>
#include <Ethernet.h>
//#define WEBDUINO_SERIAL_DEBUGGING 1 //uncomment to see recieved requests
//#define WEBDUINO_SERIAL_DEBUGGING 5 //uncomment to see full debug
#include <WebServer.h>
#include <EEPROM.h>

#include <avr/pgmspace.h>
#include "config.html.h"
#include "index.html.h"

static uint8_t mac[6] = { 0x02, 0xAA, 0xBB, 0xCC, 0x00, 0x22 };
IPAddress default_ip(192, 168, 80, 5);
IPAddress myDns(192, 168, 80, 1);
IPAddress gateway(192, 168, 80, 1);
IPAddress subnet(255, 255, 0, 0);
const int localUdpPort = 5555;
EthernetUDP Udp;


//#define SERIAL_DEBUGGING 2 //uncomment for full output
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
#define DELAYADDR 5 

WebServer webserver(PREFIX, 80);

byte val = 0;            //integer for brightness level
byte prevval=val;

unsigned int l[6]= {0,0,0,0,255,600}; //l0,l1,l2,l3,l4,off_delay

unsigned long int lamp_start = 0, last_restart, cur = 0,period = 0; //timestamps

// обработка шаблонов
void replace_template(String & str){
  char  ibuffer[10];
      
  itoa(l[1], ibuffer, 10);
  str.replace("{{l1}}", ibuffer);
  itoa(l[2], ibuffer, 10);
  str.replace("{{l2}}", ibuffer);
  itoa(l[3], ibuffer, 10);
  str.replace("{{l3}}", ibuffer);
  itoa(l[4], ibuffer, 10);
  str.replace("{{l4}}", ibuffer);
  itoa(l[5], ibuffer, 10);
  str.replace("{{delay}}", ibuffer);
  
}

void printCmdPage(WebServer & server){
    char buffer[100];
    for (int i = 0; i < index_html_size; i ++){
      strcpy_P(buffer, (char*)pgm_read_word(&(index_html[i])));
      String str(buffer);
      replace_template(str);      
      server.print(str.c_str()); 
    }
}


//------------------------------------------------------------------------------
void getValParam(WebServer &server, byte & result){
    char name[16], value[16];
    bool repeat = true;
    do{
      repeat = server.readPOSTparam(name, 16, value, 16);      
      if (!strcmp(name, "val"))
        result = strtoul(value, NULL, 10);   
    } while (repeat);
}

unsigned char pname_to_vi(char * p_name){
    if (!strcmp(p_name, "l1"))
        return 1;
    if (!strcmp(p_name, "l2"))
        return 2;
    if (!strcmp(p_name, "l3"))
        return 3;
    if (!strcmp(p_name, "l4"))
        return 4;
    if (!strcmp(p_name, "delay"))
        return 5;
    return -1;
}

void getLParams(WebServer & server,unsigned int * l ){
    bool repeat = true;
    char p_name[16], value[16];
    unsigned int var = 0;
    do {
      repeat = (server.readPOSTparam(p_name, 16, value, 16));
      #if SERIAL_DEBUGGING > 1
      Serial.print(p_name);      
      Serial.print("=");
      Serial.println(value);
      #endif
      unsigned char vi = pname_to_vi(p_name);      
      if(vi != -1){    
        var=strtol(value, NULL, 10);
        #if SERIAL_DEBUGGING > 1
          Serial.print("var=");
          Serial.println(var);
        #endif
        
        if(vi<5)
          l[vi] = constrain(var,0,100);
        else if(vi==5)
          l[vi] = constrain(var,0,3600);
      }    
    } while(repeat);
}

void ctrlCmd(WebServer &server, WebServer::ConnectionType type, char *, bool){
  if (type == WebServer::POST){
    getValParam(server, val);
    server.httpSeeOther("/");    
  }
  else{  
      server.httpSuccess();
      printCmdPage(server);
  }
}

void printConfigPage(WebServer & server){
    char buffer[100];
    for (int i = 0; i < config_html_size; i ++){
      strcpy_P(buffer, (char*)pgm_read_word(&(config_html[i])));
      String str(buffer);
      replace_template(str);      
      server.print(str.c_str()); 
    }
}




void update_eeprom(){
  #if SERIAL_DEBUGGING > 1
  Serial.print("Writing to eeprom:");
  #endif  
  for (int vi = 1; vi <=5; vi++){              
      EEPROM.update((vi-1) * 2 + 1, l[vi] & 255);
      EEPROM.update((vi-1) * 2 + 2, (l[vi]>>8) & 255);
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
    #if SERIAL_DEBUGGING > 1
      Serial.println("Config form read");
    #endif
    getLParams(server, l);
    update_eeprom();
    val=0;
    server.httpSeeOther("cfg");

  } else{
    server.httpSuccess();  
    printConfigPage(server);
  }
}
void print_levels(){
    for(short int i=1;i<=4;i++){
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

char ANNOUNCE []= "announce";
void announce(){
    
          
    IPAddress address = ~Ethernet.subnetMask() | Ethernet.gatewayIP();
    #ifdef SERIAL_DEBUGGING
      Serial.print("announcing ");
      Serial.println(address);
    #endif  
    Udp.begin(localUdpPort);
    Udp.beginPacket( address, 5432);    
    Udp.write(ANNOUNCE);    
    Udp.endPacket();
    Udp.stop();
    #ifdef SERIAL_DEBUGGING
    Serial.println("done announcing");
    #endif   
}

void restart_server(){
  if (Ethernet.begin(mac, DHCPREQ, DHCPRES) == 0){
    #if SERIAL_DEBUGGING > 0
      Serial.println("Failed to  configure Ethernet using DHCP");
    #endif
    Ethernet.begin(mac, default_ip);
        
  }
  last_restart = millis();
    
  announce();
}
//-----------------------------------------------------------------------------------------------------------
void setup(){
  //pins setup
  pinMode(OUT_PIN, OUTPUT);
  analogWrite(OUT_PIN, 0);
  pinMode(ON_PIN, OUTPUT);
  analogWrite(ON_PIN, 0);

  //get settings from EEPROM
  for (int addr=L1ADDR; addr <= DELAYADDR; addr++){
    l[addr] = EEPROM.read((addr-L1ADDR)* 2 + L1ADDR +1);
    l[addr]<<=8;
    l[addr]+= EEPROM.read((addr-L1ADDR)* 2 + L1ADDR);
  }

  //Open serial
  Serial.begin(115200);

  #if SERIAL_DEBUGGING > 1
    Serial.print("light levels from config:");
    print_levels();
  #endif
  
  webserver.setDefaultCommand(&ctrlCmd);
  webserver.addCommand("cfg", &cfgCmd);
  restart_server();
  
  #if SERIAL_DEBUGGING > 0
    Serial.print("IP:");
    Serial.println(Ethernet.localIP());
  #endif

  
}

void processLamps(){    
  if(val != prevval){ // пользователь изменил состояние?
    // пользователь включил лампу?
    if (val > 0) {
      period = l[5]*1000;
      int value = l[val];
      int pwm_value = 0;
      if (value < 50)
         pwm_value = map(value, 0, 50, 0, 50);
      else
         pwm_value = map(value, 50,100, 50, 255);
      #if SERIAL_DEBUGGING > 1
        Serial.print("Sending ");
        Serial.println(pwm_value);
      #endif
      analogWrite(OUT_PIN, pwm_value);
      digitalWrite(ON_PIN,HIGH);
    }
    lamp_start = cur;
    prevval = val;
  }
  else if( val && ((cur - lamp_start)>period))
      prevval = val = 0;
  
  if (val == 0){
      digitalWrite(ON_PIN, LOW);
      digitalWrite(OUT_PIN, LOW);   
  };
}

//-----------------------------------------------------------------------------------------------------------
void loop(){
  cur = millis();  
  webserver.processConnection();
  processLamps();
  if (cur - last_restart > 15000){
    restart_server();
  }
}

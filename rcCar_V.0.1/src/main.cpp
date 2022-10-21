/***********************************************
 * Firmware for car 13.
 * Device is developed for rc toys capable cending data ower wifi by broadcasting mesages 
 * V1 receiver consists of oled screen hbridge servo motor and dc motor
 * devive will receive pocets of data from v1 transmiter
 * 
 * 
 */


#include <esp_now.h>
#include <WiFi.h>

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>


#define SCR_WIDTH 128 // oled width in px
#define SCR_HEIGHT 64 // oled height in px

//creating screen object  
//cone wire protocol comunications
//seting size of the screen
Adafruit_SSD1306 display(SCR_WIDTH, SCR_HEIGHT, &Wire, -1);


uint8_t broadcastAddress[] = {0x84, 0x0D, 0x8E, 0xE6, 0x78, 0x74}; //protoCtrl
//same data structure as on controler
//Message object prototype
typedef struct struct_message {
  //variables holding values of sensor readings
  //{ping,comms 0 or 1, battery, a1 val, a2 val, sw val, temp val,proces time master, proces time slave}
  unsigned int dataForOLED[9];//number data values 
  char character[10];//10 simbol word datauset for controling oject identity (in future iidentity will be based on mac)
  
} struct_message;

//creating new wariable of masage for later use
struct_message message;




//***********FUNCTIONS***************

//function wifi initialization/////////////////////////
void init_wifi(){
  WiFi.mode(WIFI_STA);//Wifi mode station
  Serial.println(WiFi.macAddress());//printing boards mac address
  if (esp_now_init() != ESP_OK) {//wifi init
    Serial.println("Error initializing ESP-NOW");
    return;
  }
}


//function oled init
void initOLED(){
  // check display is conected othervise mesage and loop forever
  if(!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3D for 128x64
    Serial.println(F("SSD1306 allocation failed"));
    //replace for(;;) with: if screen failed get value and define later 
    //in the code to not talk with screen but insted print to terminal
    for(;;);
  }
  //short delay before using screen
  delay(2000);
  //  initialy clering creen
  display.clearDisplay();
}


//function draw shapes on screan to create tables bordes and feals for data
void drawShapes(){
  //horizontals
  display.drawLine(0,0,126,0,SSD1306_WHITE);//1
  display.drawLine(0,10,127,10,SSD1306_WHITE);//2
    
  //verticals
  display.drawLine(0,1,0,9,SSD1306_WHITE);//1
  display.drawLine(38,1,38,9,SSD1306_WHITE);//2
  display.drawLine(76,1,76,9,SSD1306_WHITE);//3
  display.drawLine(90,1,90,9,SSD1306_WHITE);//4
  display.drawLine(127,1,127,9,SSD1306_WHITE);//5
  display.drawLine(63,11,63,63,SSD1306_WHITE);//6

  //display.drawLine(50,6,100,6,SSD1306_WHITE);
  
  
}

//function displaying data in oled
void writeToOled(){
  display.setTextSize(1);
  display.setTextColor(WHITE);

  //top display part
  display.setCursor(2, 2);
  display.printf("%dms", message.dataForOLED[0]);
  display.setCursor(40,2);
  display.print(message.character);
  display.setCursor(78,2);
  display.print(message.dataForOLED[1]);
  display.setCursor(92,2);
  display.printf("%d %", message.dataForOLED[2]);

  //left bottom part
  display.setCursor(2, 20);
  display.printf("A1 %d%", message.dataForOLED[3]);
  display.setCursor(2,35);
  display.printf("A2 %d%", message.dataForOLED[4]);
  display.setCursor(2,50);
  display.printf("SW %d", message.dataForOLED[5]);
  //right bottom part
  display.setCursor(65, 20);
  display.printf("Temp %dc", message.dataForOLED[6]);
  display.setCursor(65,35);
  display.printf("PTM %dms", message.dataForOLED[7]);
  display.setCursor(65,50);
  display.printf("PTS %dms", message.dataForOLED[8]);
}

//cal back fuction will let us know abaut delivery status++++++++++++++++++++++++++
void data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  Serial.print("\r\nStatus of Last Message Sent:\t");
  Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  Serial.println("   callBack message");
}

//data receive function like call back function 
void data_receive(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&message, incomingData, sizeof(message));
  Serial.println("message received");
  /*
  Serial.print("Bytes received: ");
  Serial.println(len);
  Serial.print("Char: ");
  Serial.println(message.character);
  Serial.print("a1: ");
  Serial.println(message.dataForOLED[3]);
  Serial.print("a2: ");
  Serial.println(message.dataForOLED[4]);
  Serial.print("sw: ");
  Serial.println(message.dataForOLED[5]);
  Serial.println();
  */

  //send back mesage to controler
  //when mesage received mote mesage id
  //record mesage id and send back to controler 
  esp_now_send( broadcastAddress, (uint8_t *) &message, sizeof(message));
  Serial.println("after send mesage back");
}






void setup()
{
  //serial monitor setup
  Serial.begin(115200); 
  Serial.println("Serial coms initialized");

  
  //filing mesage with dummy data
  //fill mesage with word will be replaced in future and not used debg purpoce
  strcpy(message.character, "Car 13");
  message.dataForOLED[0] = 0;
  message.dataForOLED[1] = 0;
  message.dataForOLED[2] = 0;
  message.dataForOLED[3] = 0;
  message.dataForOLED[4] = 0;
  message.dataForOLED[5] = 0;
  message.dataForOLED[6] = 0;
  message.dataForOLED[7] = 0;
  message.dataForOLED[8] = 0;
  Serial.println("data filed to struct");
  
  Serial.println("cal back ok");
  // oled init
  initOLED();
  // wifi init
  init_wifi();
  //regirter receive callback function
  esp_err_t regOutcomeR = esp_now_register_recv_cb(data_receive);
  //registering calback function
  esp_err_t regOutcomeS = esp_now_register_send_cb(data_sent);
  Serial.print(regOutcomeR);
  Serial.print(regOutcomeS);

  
  
}

void loop()
{
  //draw ui and print data to oled
  display.clearDisplay();
  drawShapes();
  writeToOled();
  display.display();
  
  //
  delay(100);
}
/*****************************************
 * Firmavare for controler by PM
 * Device is developed for rc toys capable cending data ower wifi by broadcasting mesages 
 * V1 consists of oles display esp32 mcu analog controler (includes 2 potentiometers and push switch)
 * ? trasmition speed size per mesage max distace delay
 * 
 */


#include <esp_now.h>
#include <WiFi.h>

#include <Arduino.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#include <Wire.h>


//definitions for screen size
#define SCR_WIDTH 128 // oled width in px
#define SCR_HEIGHT 64 // oled height in px

//pin selection
#define ANALOG_1 33 // analog potetiometer 1
#define ANALOG_2 32 // analog potetiometer 2
#define SW_1 35 // swith

//creating screen object  
//cone wire protocol comunications
//seting size of the screen
Adafruit_SSD1306 display(SCR_WIDTH, SCR_HEIGHT, &Wire, -1);

//Mac address list
uint8_t broadcastAddress[] = {0xBC, 0xDD, 0xC2, 0xD0, 0xE5, 0xD4}; //protoCar
//uint8_t broadcastAddress[] = {0x84, 0x0D, 0x8E, 0xE6, 0x78, 0x74}; //protoCtrl


//variables holding values of sensor readings
int aRead = 0, bRead = 0, sRead = 0, tSend = 0, tReceive = 0 ;//2x analog 1x digital
//Message object prototype
typedef struct struct_message {
  //variables holding values of sensor readings
  //{ping,comms 0 or 1, battery, a1 val, a2 val, sw val, temp val,proces time master, proces time slave}
  unsigned int dataForOLED[9];//number data values 
  char character[10];//10 simbol word datauset for controling oject identity (in future iidentity will be based on mac)
  
} struct_message;

//creating message object for return message
struct_message message;

//creating peer object (the device seting to which controler will conect)
esp_now_peer_info_t peerInfo;

//***********FUNCTIONS***************

//function wifi initialization/////////////////////////
void init_wifi(){
  WiFi.mode(WIFI_STA);//setuping board to station mode
  Serial.println(WiFi.macAddress());//printing boards mac adres
  if (esp_now_init() != ESP_OK) { //initializing esp now protocol
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

//function pairing modules//////////////////
void addPeer(){
  memcpy(peerInfo.peer_addr, broadcastAddress, 6);//peering two esp boards
  peerInfo.channel = 0;  
  peerInfo.encrypt = false;     
  if (esp_now_add_peer(&peerInfo) != ESP_OK){// add peer
    Serial.println("Failed to add peer");
    return;
  }
}

//function read sensors//////////////////////////
void readSensors(){
  // read controler
  aRead = analogRead(ANALOG_1);
  message.dataForOLED[3] = map(aRead, 0, 4095, 0, 100);
  bRead = analogRead(ANALOG_2);
  message.dataForOLED[4] = map(bRead, 0, 4095, 0, 100);
  message.dataForOLED[5] = digitalRead(SW_1);
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

void calcPing(){
  message.dataForOLED[0] = tReceive - tSend;
}
//cal back fuction will let us know abaut delivery status++++++++++++++++++++++++++
void data_sent(const uint8_t *mac_addr, esp_now_send_status_t status) {
  //Serial.print("\r\nStatus of Last Message Sent:\t");
  //Serial.print(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
  //Serial.println("   callBack message");
  //read time on send
  tSend = millis();
  message.dataForOLED[7] = millis();
}

//receive callback fuction will receive mesage and check mesage id 
//after confirming send and receive message id's 
//controler calculates time taked for mesage
void data_receive(const uint8_t * mac, const uint8_t *incomingData, int len) {
  memcpy(&message, incomingData, sizeof(message));

  //read time on reveive
  Serial.println("mesage received");
  message.dataForOLED[0] = millis() - tSend;
  message.dataForOLED[8] = millis();
}
//***************SETUP!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
void setup()
{
  //serial monitor setup
  Serial.begin(115200); 
  Serial.println("Serial coms initialized");
  //oled init
  initOLED();
  // wifi init
  init_wifi();
  //registering calback function
  esp_now_register_send_cb(data_sent);
  esp_err_t regOutcome = esp_now_register_recv_cb(data_receive);
  //peering two esp boards
  addPeer();
  //seting i/o directions
  pinMode(ANALOG_1, INPUT);
  pinMode(ANALOG_2, INPUT);
  pinMode(SW_1, INPUT);
}


//******************RUN
void loop()
{
  //reading sensors
  readSensors();
  calcPing();

  //fill mesage with word will be replaced in future and not used debg purpoce
  strcpy(message.character, "Car 13");

  //sending message and storing status
  esp_err_t outcome = esp_now_send( broadcastAddress, 
                                    (uint8_t *) &message, 
                                    sizeof(message));

  //cheching status of the mesage delivery
  if (outcome == ESP_OK) {
    //Serial.println("Mesage sent successfully!  loop mesage");
  }
  else {
    //Serial.println("Error sending the message  loop mesage");
  }

  //calculate pin by usng send and delivery status
  

  //draw ui and print data to oled
  display.clearDisplay();
  drawShapes();
  writeToOled();
  display.display();

  //
  delay(100);
}
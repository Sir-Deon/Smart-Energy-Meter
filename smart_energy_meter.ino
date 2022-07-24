#include<SPI.h>
#include<SD.h>
#include <SoftwareSerial.h>
#include <Wire.h> 
#include <LiquidCrystal_I2C.h>
#include <virtuabotixRTC.h>    
//Wiring SCLK -> 6, I/O -> 7, CE -> 8
//Or CLK -> 6 , DAT -> 7, Reset -> 8                                                                          

LiquidCrystal_I2C lcd(0x27,20,4);  // set the LCD address to 0x27 for a 20 chars and 4 line display

//Create software serial object to communicate with SIM800L
SoftwareSerial mySerial(10, 11); //SIM800L Tx & Rx is connected to Arduino #9 & #5
virtuabotixRTC myRTC(6, 7, 8); // Creation of the Real Time Clock Object
File myFile;

const int chipSelect = 10;

// defines variables
int index = 0;
String number = "";
String message = "";

char incomingByte;
String incomingData;
bool atCommand = true;

int relay1 = 2;
int relay2 = 3;
int relay3 = 14;
float energy;
int power_led = 1;
float Energy = 0;

double sensorValue1 = 0;
double sensorValue2 = 0;
int crosscount = 0;
int climb_flag = 0;
int val[100];
int max_v = 0;
double VmaxD = 0;
double VeffD = 0;
double Veff = 0;

double C_sensorValue1 = 0;
double C_sensorValue2 = 0;
int C_crosscount = 0;
int C_climb_flag = 0;
int C_val[100];
int C_max_v = 0;
float C_maxD = 0;
float C_effD = 0;
float C_eff = 0;

void setup()  {
  Serial.begin(9600);
  mySerial.begin(9600); //Begin serial communication with Arduino and SIM800L

  pinMode(relay1,OUTPUT);
  pinMode(relay2,OUTPUT);
  pinMode(relay3,OUTPUT);
  pinMode(power_led,OUTPUT);

  digitalWrite(relay1,HIGH);
  digitalWrite(relay2,LOW);
  digitalWrite(relay3,LOW);
  digitalWrite(power_led,HIGH);
  
  lcd.init(); // initialize the lcd 
  lcd.backlight();
  lcd.setCursor(6,0);
  lcd.print("Welcome");
  delay(2000);
  lcd.clear();
  lcd.setCursor(4,1);
  lcd.print("Smart Energy");
  lcd.setCursor(8,2);
  lcd.print("Meter");
  delay(2500);
  lcd.clear();
  lcd.print("Initializing.");
  delay(500);
  lcd.clear();
  lcd.print("Initializing..");
  delay(500);
  lcd.clear();
  lcd.print("Initializing...");
  delay(500);
  lcd.clear();
  lcd.print("Initializing.");
  delay(500);
  lcd.clear();
  lcd.print("Initializing..");
  delay(500);
  lcd.clear();
  lcd.print("Initializing...");
  delay(500);
  lcd.clear();


  // Check if you're currently connected to SIM800L
  while (!mySerial.available()) {
    mySerial.println("AT");
    delay(1000);
    Serial.println("connecting....");
  }

  Serial.println("Connected..");

  mySerial.println("AT+CMGF=1");  //Set SMS Text Mode
  delay(1000);
  mySerial.println("AT+CNMI=1,2,0,0,0");  //procedure, how to receive messages from the network
  delay(1000);
  mySerial.println("AT+CMGL=\"REC UNREAD\""); // Read unread messages

  Serial.println("Ready to received Commands..");
 
  
  while (!Serial) {
     // wait for serial port to connect. Needed for Leonardo only
  }
  Serial.print("Initializing SD card...");
  
  if (!SD.begin()) {
    Serial.println("initialization failed!");
    return; 
    }
  Serial.println("initialization done.");
  delay(100);
}

void loop() {
  myRTC.updateTime(); // This allows for the update of variables for time or accessing the individual elements.
  digitalWrite(power_led,HIGH);

  if(readingCurrent()>= 0.08){
    energy = readingEnergy();
   }

  // Start printing elements as individuals                                                                //|   
  Serial.print("Current Date / Time: ");                                                                 //| 
  Serial.print(myRTC.dayofmonth);                                                                        //| 
  Serial.print("/");                                                                                     //| 
  Serial.print(myRTC.month);                                                                             //| 
  Serial.print("/");                                                                                     //| 
  Serial.print(myRTC.year);                                                                              //| 
  Serial.print("  ");                                                                                    //| 
  Serial.print(myRTC.hours);                                                                             //| 
  Serial.print(":");                                                                                     //| 
  Serial.print(myRTC.minutes);                                                                           //| 
  Serial.print(":");                                                                                     //| 
  Serial.println(myRTC.seconds);

  lcd.setCursor(0,0);
  lcd.print("Date ");
  lcd.print(myRTC.dayofmonth);
  lcd.print("/");
  lcd.print(myRTC.month);
  lcd.print("/");
  lcd.print(myRTC.year);
  lcd.setCursor(0,1);
  lcd.print("Voltage: ");
  lcd.print(readingVoltage());
  lcd.print(" V");
  lcd.setCursor(0,2);
  lcd.print("Current: ");
  lcd.print(readingCurrent());
  lcd.print(" A");
  lcd.setCursor(0,3);
  lcd.print("Energy: ");
  lcd.print(energy, 4);
  lcd.print(" KWh");
 

  Serial.print("Current Date / Time: ");                                                                 //| 
  Serial.print(myRTC.dayofmonth);                                                                        //| 
  Serial.print("/");                                                                                     //| 
  Serial.print(myRTC.month);                                                                             //| 
  Serial.print("/");                                                                                     //| 
  Serial.print(myRTC.year);                                                                              //| 
  Serial.print("  ");                                                                                    //| 
  Serial.print(myRTC.hours);                                                                             //| 
  Serial.print(":");                                                                                     //| 
  Serial.print(myRTC.minutes);                                                                           //| 
  Serial.print(":");                                                                                     //| 
  Serial.println(myRTC.seconds);
  
  Serial.print("Voltage: ");
  Serial.print(readingVoltage());
  Serial.println(" V");
  Serial.print("Current: ");
  Serial.print(readingCurrent(),4);
  Serial.println(" A");
  Serial.print("Energy: ");
  Serial.print(energy, 4);
  Serial.println(" KWh");

  if (mySerial.available()) {
    delay(100);

    // Serial buffer
    while (mySerial.available()) {
      incomingByte = mySerial.read();
      incomingData += incomingByte;
    }

    delay(10);
    if (atCommand == false) {
      receivedMessage(incomingData);
    } else {
      atCommand = false;
    }

    //delete messages to save memory
    if (incomingData.indexOf("OK") == -1) {
      mySerial.println("AT+CMGDA=\"DEL ALL\"");
      delay(1000);
      atCommand = true;
    }

    incomingData = "";
  }

  

          
  storingData();

  delay(2000);
}

void receivedMessage(String inputString) {

  //Get The number of the sender
  index = inputString.indexOf('"') + 1;
  inputString = inputString.substring(index);
  index = inputString.indexOf('"');
  number = inputString.substring(0, index);
  Serial.println("Number: " + number);

  //Get The Message of the sender
  index = inputString.indexOf("\n") + 1;
  message = inputString.substring(index);
  message.trim();
  Serial.println("Message: " + message);
  message.toUpperCase(); // uppercase the message received


  //turn Device 1 ON
  if (message.indexOf("RELAY OFF") > -1) {
        digitalWrite(relay3,HIGH);
        Serial.println("Relay Off");
        
        mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
        delay(500);
        mySerial.println("AT+CMGS=\"+237651788492\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
        delay(500);
        mySerial.print("Relay OFF"); //text content
        delay(500);
        mySerial.write(26);
      
    }else if(message.indexOf("RELAY ON") > -1){
        digitalWrite(relay3,LOW);
        Serial.println("Relay On"); 
        
        mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
        delay(500);
        mySerial.println("AT+CMGS=\"+237651788492\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
        delay(500);
        mySerial.print("Relay ON"); //text content
        delay(500);
        mySerial.write(26);
      
    }else if(message.indexOf("READINGS") > -1){
          mySerial.println("AT"); //Handshaking with SIM900
        updateSerial();
      
      mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
      delay(200);
      mySerial.println("AT+CMGS=\"+237651788492\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
      delay(200);
      mySerial.print(myRTC.dayofmonth); //text content
      mySerial.print("/"); //text content
      delay(200);
      mySerial.print(myRTC.month); //text content
      mySerial.print("/"); //text 
      delay(200);
      mySerial.print(myRTC.year); //text content
      mySerial.print("    "); //text contentcontent
      delay(200);
      mySerial.print(myRTC.hours); //text content
      mySerial.print(":"); //text content
      delay(200);
      mySerial.print(myRTC.minutes); //text content
      mySerial.print(":"); //text content
      delay(200);
      mySerial.println(myRTC.seconds); //text content
      mySerial.println(); //text 
      delay(200);

      mySerial.print("Voltage: "); //text content
      mySerial.print(readingVoltage()); //text content
      mySerial.println(" V"); //text content
      mySerial.println(); //text 
      delay(200);
      mySerial.print("Current: "); //text content
      mySerial.print(readingCurrent()); //text content
      mySerial.println(" A"); //text content
      mySerial.println(); //text 
      delay(200);
      mySerial.print("Energy: "); //text content
      mySerial.print(energy, 4); //text content
      mySerial.println(" KWh"); //text content
      mySerial.println(); //text 
      delay(200);
      updateSerial();
    
      mySerial.write(26);
        
        Serial.println("Sent"); 
    }

  delay(50);// Added delay between two readings
}

void updateSerial()
{
  delay(100);
  while (Serial.available()) {
    mySerial.write(Serial.read());//Forward what Serial received to Software Serial Port
  }
  while(mySerial.available()) {
    Serial.write(mySerial.read());//Forward what Software Serial received to Serial Port
  }
}

void sendingSMS(){
    mySerial.println("AT"); //Once the handshake test is successful, it will back to OK
    updateSerial(); 
    mySerial.println("AT+CMGF=1"); // Configuring TEXT mode
    updateSerial();
    mySerial.println("AT+CMGS=\"+237651788492\"");//change ZZ with country code and xxxxxxxxxxx with phone number to sms
    updateSerial();
    mySerial.print(myRTC.dayofmonth); //text content
    mySerial.print("/"); //text content
    mySerial.print(myRTC.month); //text content
    mySerial.print("/"); //text content
    mySerial.print(myRTC.year); //text content
    mySerial.print("    "); //text content
    mySerial.print(myRTC.hours); //text content
    mySerial.print(":"); //text content
    mySerial.print(myRTC.minutes); //text content
    mySerial.print(":"); //text content
    mySerial.println(myRTC.seconds); //text content
    mySerial.println(); //text content
    
    mySerial.print("Voltage: "); //text content
    mySerial.print(readingVoltage()); //text content
    mySerial.println(" V"); //text content
    mySerial.print("Current: "); //text content
    mySerial.print(readingCurrent()); //text content
    mySerial.println(" A"); //text content
    mySerial.print("Energy: "); //text content
    mySerial.print(energy, 4); //text content
    mySerial.println(" KWh"); //text content
    updateSerial();
    mySerial.write(26);
  }
  
void storingData(){
  // open the file. note that only one file can be open at a time,
  // so you have to close this one before opening another.
  myFile = SD.open("meter_reading.txt", FILE_WRITE);

  // if the file opened okay, write to it:
  if (myFile) {
    Serial.print("Writing to meter_reading.txt...");
    myFile.print("Date");
    myFile.print(myRTC.dayofmonth);
    myFile.print("/");
    myFile.print(myRTC.month);
    myFile.print("/");
    myFile.println(myRTC.year);
 
    myFile.print("Voltage: ");
    myFile.print(readingVoltage());
    myFile.println(" V");
    myFile.print("Current: ");
    myFile.print(readingCurrent());
    myFile.println(" A");
    myFile.print("Energy: ");
    myFile.print(energy, 4);
    myFile.println(" KWh");
    // close the file:
    myFile.close();
    Serial.println("done.");
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening meter_reading.txt");
  }

  // re-open the file for reading:
  myFile = SD.open("meter_reading.txt");
  if (myFile) {
    Serial.println("meter_reading.txt: ");

    // read from the file until there's nothing else in it:
    while (myFile.available()) {
      Serial.write(myFile.read());
    }
    // close the file:
    myFile.close();
  } else {
    // if the file didn't open, print an error:
    Serial.println("error opening meter_reading.txt");
  }
 }
 
float readingVoltage(){
   for ( int i = 0; i < 100; i++ ) {
    sensorValue1 = analogRead(A0);
    if (analogRead(A0) > 511) {
      val[i] = sensorValue1;
    }
    else {
      val[i] = 0;
    }
    delay(1);
  }

  max_v = 0;

  for ( int i = 0; i < 100; i++ ){
    if ( val[i] > max_v )
    {
      max_v = val[i];
    }
    val[i] = 0;
  }
  
  if (max_v != 0) {
    VmaxD = max_v;
    VeffD = VmaxD / sqrt(2);
    Veff = (((VeffD - 420.76) / -90.24) * -210.2) + 210.2;
  }
  else {
    Veff = 0;
  }
  
  return Veff;
  VmaxD = 0;
 }

float readingCurrent(){
    for ( int i = 0; i < 100; i++ ) {
    C_sensorValue1 = analogRead(A1);
    if (analogRead(A1) > 511) {
      C_val[i] = C_sensorValue1;
    }
    else {
      C_val[i] = 0;
    }
    delay(1);
  }

  C_max_v = 0;

  for ( int i = 0; i < 100; i++ )
  {
    if ( C_val[i] > C_max_v )
    {
      C_max_v = C_val[i];
    }
    C_val[i] = 0;
  }
  
  if (C_max_v != 0) {
    C_maxD = C_max_v;
    C_effD = C_maxD / sqrt(2);
    C_eff = (((C_effD - 420.76) / -90.24) * -5.2) + 5.2;
    C_eff = C_eff-1.9;
    
    if (C_eff < 0.0001){
    C_eff = 0;
    }
  }
 
  else {
    C_eff = 0;
  }
   C_eff= C_eff,4;

  return C_eff;
  //C_maxD = 0;
 }

 float readingEnergy(){
  float Power = (readingCurrent() * readingVoltage()) / 1000;
  float hr;
  float m = myRTC.minutes ;
      hr = m / 60;
  Energy = Energy + (Power * hr);
  Serial.println(m);
  Serial.println(hr,4);
  Serial.println(Energy);
  return Energy;
  delay(1000);
  }

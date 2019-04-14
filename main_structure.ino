
//BUTTONS require no libraries for now

//LCD LIBARIES
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display

//TEMP SENSOR LIBRARIES
#include <OneWire.h>
#include <DallasTemperature.h>

//GPS LIBRARIES
#include <SoftwareSerial.h>
#include <TinyGPS.h>

//SDCARD LIBRARIES
#include <SPI.h>
#include <SD.h>

//PIN NUMBERS
const int GSM_TDR = 0;
const int temp_sen = 1;
const int GSM_RX = 2;
const int GST_TX = 3;
const int SDcard_CS = 4;
const int GPS_RX = 5;
const int GPS_TX = 6;
const int buttonPin_left = 7; // the number of the pushbutton pin
const int buttonPin_right = 8;
const int buttonPin_set = 9;
const int buttonPin_reset = 10;
const int SDcard_MOSI = 11;
const int SDcard_MISO = 12;
const int SDcard_SCK = 13;
const int pH_sen = A0;
const int LED1 = A1; //blink when taking data
const int LED2 = A2; //blink when water BAD
const int LED3 = A3; //blink when text message sent
const int LCD_SDA = A4;
const int LCD_SCL = A5;


//VARIABLES and DEFINITIONS

int buttonState_left = 0;   // variable for reading the pushbutton status
int buttonState_right = 0;
int buttonState_set = 0;
int buttonState_reset = 0;

unsigned long startMillis;
unsigned long currentMillis;
unsigned long elapsedMillis;
float time_interval; //between 1 second and 59min,59sec

SoftwareSerial GPSSerial(6,5); //(rx, tx) on MCU
TinyGPS gps;
void gpsdump(TinyGPS &gps);
long lat, lon;
unsigned long age;

int lat_int, lon_int; //get digits before decimal (there will be six digits after decimal!)
long lat_int_small, lon_int_small; //get digits after decimal


//PH vars
#define Offset 0.2 //deviation compensate
#define samplingInterval 20
#define ArrayLenth 40 //times of collection
int pHArray[ArrayLenth]; //Store the average value of the sensor
int pHArrayIndex=0;

//temperature vars
#define ONE_WIRE_BUS 1
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors_temp(&oneWire);
float temp = 0; //CELCIUS

File dataFile;


void setup() {
  Serial.begin(9600);

  lcd.init(); // initialize the lcd
  lcd.backlight();
  lcd.setCursor(0,0);
  lcd.print("Time interval:");
  delay(500);
  lcd.clear(); //clear screen (apparently this is the fastest way to do it)
  lcd.print("00:00");
  // initialize the LED pin as an output:
  Serial.begin(9600); 
  // initialize the pushbutton pin as an input:
  pinMode(buttonPin_left, INPUT);
  pinMode(buttonPin_right, INPUT);
  pinMode(buttonPin_set, INPUT);
  pinMode(buttonPin_reset, INPUT);
  
  int digit10 = 0;
  int digit4 = 0;
  int phone_number[10] = {0, 0, 0, 0, 0, 0, 0, 0, 0, 0};
  int time_number[4] = {0, 0, 0, 0};

  //TIME
  while(digit4 <= 3){
    buttonState_left = digitalRead(buttonPin_left);
    buttonState_right = digitalRead(buttonPin_right);
    buttonState_set = digitalRead(buttonPin_set);
    buttonState_reset = digitalRead(buttonPin_reset);

    if(buttonState_left == HIGH) {
      Serial.print("l ");
      buttonState_left = LOW; 
      delay(1500);
      time_number[digit4]--;
      if(digit4 == 0 || digit4 == 2) { //check for tenths digit
        if(time_number[digit4] < 0) time_number[digit4] = 5; //cannot have > 60 minutes in an hr/min        
      } else {
        if(time_number[digit4] < 0) time_number[digit4] = 9;
      }

      if(digit4 <= 1) { //first two digits
        lcd.setCursor(digit4,0);
        lcd.print(time_number[digit4]);
      } else { //second two digits following the ':'
        lcd.setCursor(digit4+1,0);
        lcd.print(time_number[digit4]);        
      }
    }
    if(buttonState_right == HIGH) {
      Serial.print("r ");
      time_number[digit4]++;
      buttonState_right = LOW;
      delay(1500);
      if(digit4 == 0 || digit4 == 2) { //check for tenths digit
        time_number[digit4] %= 6; //cannot have > 60 minutes in an hr/min
      } else {
        time_number[digit4] %= 10;
      }      

      if(digit4 <= 1) { //first two digits
        lcd.setCursor(digit4,0);
        lcd.print(time_number[digit4]);
      } else { //second two digits following the ':'
        lcd.setCursor(digit4+1,0);
        lcd.print(time_number[digit4]);        
      }
    }  
    if(buttonState_set == HIGH){
      Serial.println("s ");
      digit4++;
      buttonState_set = LOW;
      delay(1000);
    }
    if(buttonState_reset == HIGH){
      Serial.println("reset ");
      digit4 = 0;
      for (int i = 0; i < 4; i++) {
        time_number[i] = 0; //reset value for each index
      }
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("RESET");
      delay(500);
      lcd.clear();
      lcd.print("00:00");
      lcd.setCursor(0,0);
      buttonState_reset = LOW;
      delay(1000);
    }
    
    buttonState_left = LOW;
    buttonState_right = LOW;
    buttonState_set = LOW;
    buttonState_reset = LOW;
  }
  
  Serial.println("Time interval: ");
  for (int j = 0; j < 4; j++){
    Serial.print(time_number[j]);
  }
  Serial.println();



  //PHONE
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Phone Number:");
  delay(500);
  lcd.clear(); //clear screen (apparently this is the fastest way to do it)
  lcd.print("000-000-0000");
  
  buttonState_left = LOW;
  buttonState_right = LOW;
  buttonState_set = LOW;
  buttonState_reset = LOW;

  while(digit10 <= 9) {  
    buttonState_left = digitalRead(buttonPin_left);
    buttonState_right = digitalRead(buttonPin_right);
    buttonState_set = digitalRead(buttonPin_set);
    buttonState_reset = digitalRead(buttonPin_reset); 
    
    if(buttonState_left == HIGH) {
      Serial.print("l ");
      phone_number[digit10]--;
      if(phone_number[digit10] < 0) phone_number[digit10] = 9;
      delay(1500);
    }
    if(buttonState_right == HIGH) {
      Serial.print("r ");
      phone_number[digit10]++;
      phone_number[digit10] %= 10; 
      delay(1500);
    }
    
    //012-456-89 10 11
    if(digit10 <= 2) { //first three digits XXX-...-....
      lcd.setCursor(digit10,0);
      lcd.print(phone_number[digit10]);
      } 
    else if (digit10>2 && digit10<6){ //...-XXX-....
      lcd.setCursor(digit10+1,0);
      lcd.print(phone_number[digit10]);
    }     
    else { //last 4 digits    ...-...-XXXX
      lcd.setCursor(digit10+2,0);
      lcd.print(phone_number[digit10]);        
    }
      
    if(buttonState_set == HIGH){
      Serial.println("s ");
      digit10++;
      delay(1000);
    }
    if(buttonState_reset == HIGH){
      Serial.println("reset ");
      digit10 = 0;
      for (int i = 0; i < 10; i++) {
        phone_number[i] = 0; //reset value for each index
      }
      delay(1000);      
      buttonState_reset = LOW;
      lcd.clear();
      lcd.setCursor(0,0);
      lcd.print("RESET");
      delay(500);
      lcd.clear();
      lcd.print("000-000-0000");
      lcd.setCursor(0,0);
    }
    buttonState_left = LOW;
    buttonState_right = LOW;
    buttonState_set = LOW;
    buttonState_reset = LOW;
  }  

  
  Serial.println("Phone number: ");
  for (int j = 0; j < 10; j++){
    Serial.print(phone_number[j]);
  }
  Serial.println();
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Setup Complete");
  delay(1000);
  lcd.clear();
  lcd.setCursor(0,0);


  
  time_interval = 1000.0*(60.0*(10.0*time_number[0] + time_number[1]) + (10.0*time_number[2]) + time_number[3]); //in milliseconds
  Serial.print(time_interval);
  Serial.print(" ms");
  
  pinMode(LED1, OUTPUT); //initialize LEDs
  pinMode(LED2, OUTPUT);
  pinMode(LED3, OUTPUT);
  
  GPSSerial.begin(9600);

  sensors_temp.begin();

  SD.begin(4); //digital pin 4 on our MCU
  dataFile = SD.open("data.txt", FILE_WRITE);
  
  startMillis = millis();
}

void loop() {
  currentMillis = millis();
  elapsedMillis = currentMillis - startMillis;
  
    
  //collect GPS /////////////////////////////////////////////
  bool newdata = false;
  if (GPSSerial.available()) {
    char c = GPSSerial.read();
    if (gps.encode(c)) {
      newdata = true;      
    }
  } 
  
  if (newdata) {
    Serial.println("Acquired GPS Data");
    lat_int = lat/1000000;
    lat_int_small = lat - (lat_int * 1000000);
    
    lon_int = lon/1000000;
    lon_int_small = lon - (lon_int * 1000000);
    
    gps.get_position(&lat, &lon, &age);
    Serial.print("Lat/Long(10^-5 deg): "); Serial.print(lat); Serial.print(", "); Serial.print(lon);
    Serial.println("-------------");
  }

  /// COLLECT PH //////////////////

 static unsigned long samplingTime = millis();
 static float pHValue,voltage;
 if(millis()-samplingTime > samplingInterval)
 {
   pHArray[pHArrayIndex++]=analogRead(pH_sen);
   if(pHArrayIndex==ArrayLenth) pHArrayIndex=0;
   voltage = avergearray(pHArray, ArrayLenth)*5.0/1024;
   pHValue = 3.5*voltage+Offset;
   samplingTime=millis();
   
   lcd.setCursor(0, 1);
   lcd.print(phValue);
 }

 //COLLECT TEMP////////////////////
 sensors_temp.requestTemperatures();
 temp = sensors_temp.getTempCByIndex(0);







  if (elapsedMillis >= time_interval){
    Serial.print("-TIME-");

    analogWrite(LED1, 1024); //signal that data is being collected!



    Serial.print("Voltage:");
    Serial.print(voltage,2);
    Serial.print(" pH value: ");
    Serial.println(pHValue,2);

    Serial.print(temp);
    Serial.print(" C");


    dataFile.println("WATER QUALITY: BAD");
    dataFile.print("Temp: ");
    dataFile.print("pH: ");
    dataFile.print("GPS: ");
 
    
    //
    startMillis = currentMillis; //RESET COUNTDOWN
    analogWrite(LED1, 0);
  }


  
}


double avergearray(int* arr, int number){
 int i;
 int max,min;
 double avg;
 long amount=0;
 if(number<=0){
   Serial.println("Error number for the array to avraging!/n");
   return 0;
 } 
 if(number<5){ //less than 5, calculated directly statistics
   for(i=0;i<number;i++){
    amount+=arr[i];
   }
  avg = amount/number;
  return avg;
 } else {
 if(arr[0]<arr[1]){
  min = arr[0];max=arr[1];
 } else{
  min=arr[1];max=arr[0];
 }
 for(i=2;i<number;i++){
   if(arr[i]<min){
     amount+=min; //arr<min
     min=arr[i];
   } else {
     if(arr[i]>max){
       amount+=max; //arr>max
       max=arr[i];
     } else{
      amount+=arr[i]; //min<=arr<=max
     }
   }
 }
 avg = (double)amount/(number-2);
 }
 return avg;
} 

/* PSEUDOCODE ----------------------------
 * -import required libraries
 * -assign pins for each compoment
 * 
 * SETUP
 * -initialization of buttons                            DONE
 * -initialization of LCD display                        DONE
 * -display welcome message                              DONE
 * -collect time interval from user via buttons          DONE
 * -collect phone number via buttons                     DONE
 * -initialization of GSM
* -initialization of GPS                                 DONE
 * -initialization of pH, temp sensors
 * -initialization of LED's
 *  
 * PROGRAM LOOP
 * -check countdown
 * --collect temp
 * --collect pH
 * --collect GPS                                         DONE
 * --evaluate pH data, check for 6.5-8.5 range
 * ---write above data to SDcard along with 'GOOD' or 'BAD'
 * ----if BAD, send SMS message
 * -reset countdown                                      DONE
 */


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
const int LED1 = A1;
const int LED2 = A2;
const int LED3 = A3;
const int LCD_SDA = A4;
const int LCD_SCL = A5;


//VARIABLES and DEFINITIONS
#define ONE_WIRE_BUS 6

int buttonState_left = 0;   // variable for reading the pushbutton status
int buttonState_right = 0;
int buttonState_set = 0;
int buttonState_reset = 0;

unsigned long startMillis;
unsigned long currentMillis;
unsigned long elapsedMillis;
unsigned long time_interval; //between 1 second and 59min,59sec

SoftwareSerial GPSSerial(6,5); //(rx, tx) on MCU
TinyGPS gps;
void gpsdump(TinyGPS &gps);
long lat, lon;
unsigned long age;

int lat_int, lon_int; //get digits before decimal (there will be six digits after decimal!)
long lat_int_small, lon_int_small; //get digits after decimal



OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors_temp(&oneWire);
float temp = 0; //CELCIUS

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


  time_interval = 1000*(60*(10*time_number[0] + time_number[1]) + (10*time_number[2]) + time_number[3]); //in milliseconds
  Serial.println(time_interval);
  GPSSerial.begin(9600);
  startMillis = millis();
}

void loop() {
  currentMillis = millis();
  elapsedMillis = currentMillis - startMillis;
  if (elapsedMillis >= time_interval){
    Serial.print("-TIME-");
    
    //collect GPS /////////////////////////////////////////////
    bool newdata = false;
    if (GPSSerial.available()) {
      char c = GPSSerial.read();
      if (gps.encode(c)) {
        newdata = true;      
      }
    } else {
      Serial.print("serial not available \n");
    }
    
    if (newdata) {
      Serial.println("Acquired GPS Data");
      lat_int = lat/1000000;
      lat_int_small = lat - (lat_int * 1000000);
      
      lon_int = lon/1000000;
      lon_int_small = lon - (lon_int * 1000000);
      
      gps.get_position(&lat, &lon, &age);
      Serial.print("Lat/Long(10^-5 deg): "); Serial.print(lat); Serial.print(", "); Serial.print(lon);
      //gpsdump(gps);
      Serial.println("-------------");
    }



    //
    startMillis = currentMillis; //RESET COUNTDOWN
  }


  
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
 * -initialization of GPS
 * -initialization of pH, temp sensors
 * -initialization of LED's
 *  
 * PROGRAM LOOP
 * -check countdown
 * --collect temp
 * --collect pH
 * --collect GPS
 * --evaluate pH data, check for 6.5-8.5 range
 * ---write above data to SDcard along with 'GOOD' or 'BAD'
 * ----if BAD, send SMS message
 * -reset countdown
 */

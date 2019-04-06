/*
   by Samuel Hung, Nelson Lao, Junik Kim
*/

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27,16,2); // set the LCD address to 0x27 for a 16 chars and 2 line display

// constants won't change. They're used here to
// set pin numbers:
const int buttonPin_left = 7; // the number of the pushbutton pin
const int buttonPin_right = 8;
const int buttonPin_set = 9;
const int buttonPin_reset = 10;

// variables will change:
int buttonState_left = LOW;   // variable for reading the pushbutton status
int buttonState_right = LOW;
int buttonState_set = LOW;
int buttonState_reset = LOW;


void setup() {

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
  
}


void loop() {

}

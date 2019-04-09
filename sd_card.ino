#include <SPI.h>
#include <SD.h>

File dataFile;
int tempcount = 0;

void setup() {
  // put your setup code here, to run once:
  SD.begin(4); //digital pin 4 on our MCU
  dataFile = SD.open("data.txt", FILE_WRITE);
}

void loop() {
  // put your main code here, to run repeatedly:
  dataFile.println("test");
  delay(1000);
  tempcount++;
  if(tempcount == 8){
    dataFile.close(); //where do we put this?
  }

}

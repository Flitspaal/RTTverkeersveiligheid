#include <Wire.h> 
#include <LiquidCrystal_I2C.h> //LCD i2c lib

#define POTMETER A0

float rr = 0;
float ss = 0;

double remWeg(double);
double potmeter();
void controllLCD(float s, float r);

LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  Serial.begin(9600);

  lcd.begin();  
  lcd.backlight();
  lcd.print("INFO");
  lcd.setCursor(0,1);
  lcd.print("Screen");
  
}

void loop(){
  double p = potmeter(); // waarde van de potmeter
  double r = remWeg(p); // remweg berekenen
  Serial.print(r);
  Serial.println(" m");
  controllLCD(p,r);
  delay(1000);
}

double remWeg(double v){ //calculates remweg
 double temp = (((v/10)*(v/10))/2);
 return temp;
}

double potmeter(){
  double v = analogRead(POTMETER);
  double temp = v / 6.83;
  Serial.print(temp);
  Serial.println(" km/u");
  return temp;
}

void controllLCD(float s, float r) {
  if(ss != s || rr != r){
    ss = s;
    rr = r;
  lcd.clear();// clear previous values from screen
  lcd.print("KM/U");
  lcd.setCursor(7,0);
  lcd.print(ss);
  lcd.setCursor(0,1);
  lcd.print("Remweg");
  lcd.setCursor(7,1);
  lcd.print(rr);
  lcd.setCursor(14,1);
  lcd.print("M");
    if(r>30){ // als remafstand groter is dan 30 geef dan waarschuwing
      lcd.setCursor(0,2);
      lcd.print("---!!---");
    }
  }
  delay(200);
}

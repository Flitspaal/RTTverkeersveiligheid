#include <Wire.h> 
#include <LiquidCrystal_I2C.h> //LCD i2c lib

#define POTMETER A0
#define warning 30
#define knopPin D4

//Defines voor de stoplicht pins
// SPECIFIEK VOOR DE WEMOS, AANPASSEN DUS

//VERKEERSLICHTEN
int groenLicht = 12;
int oranjeLicht = 13;
int roodLicht = 15;

//fases verkeerslichten
bool uitgeschakeld = false;

bool roodFase = false;
int groenTijd = 6000 ;
int oranjeTijd = 4500;        //tijd van oranje knipperen, 80km/h weg = 4.5 sec
int roodTijd = 6000;
int cyclusTijd = groenTijd + oranjeTijd + roodTijd;
const int knipperInterval = 500;

//variabelen verkeerslichten
int oranjeState = LOW;
unsigned long laatsteKnipper = 0;
unsigned long timer;

//PROTOTYPES
double remWeg(double);
double potmeter();
void controllLCD(float s, float r);
void stoplicht();
void uitStand();

LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  Serial.begin(9600);

  lcd.begin();  
  lcd.backlight();
  lcd.print("INFO");
  lcd.setCursor(0,1);
  lcd.print("Screen");

  timer = millis();

  pinMode(groenLicht, OUTPUT);
  pinMode(oranjeLicht, OUTPUT);
  pinMode(roodLicht, OUTPUT);

  pinMode(knopPin, INPUT_PULLUP);
}

void loop(){
  double p = potmeter(); // waarde van de potmeter
  double r = remWeg(p); // remweg berekenen
  controllLCD(p,r);

  boolean knopIn = !digitalRead(knopPin);
  if(knopIn){
    drukKnop();
    Serial.println("knop ingedrukt");
  }
  
  if (!uitgeschakeld) {
    stoplicht();
  } else {
    uitStand();
  }
  if (millis() - timer > cyclusTijd) {
    timer = millis();
    
  }
  
}

double remWeg(double v){ //calculates remweg
 double temp = (((v/10)*(v/10))/2);
 return temp;
}

double potmeter(){
  double v = analogRead(POTMETER);
  double temp = v / 6.83;
  return temp;
}

void controllLCD(float s, float r) {

  lcd.clear();// clear previous values from screen
  lcd.print("KM/U");
  lcd.setCursor(7,0);
  lcd.print(s);
  lcd.setCursor(0,1);
  lcd.print("Remweg");
  lcd.setCursor(7,1);
  lcd.print(r);
  lcd.setCursor(14,1);
  lcd.print("M");
  
    if(r>warning && roodFase == true || uitgeschakeld == true){ // als remafstand groter is dan 30 geef dan waarschuwing
        lcd.setCursor(0,2);
        lcd.print("---!!---");
    }
  delay(200);
}

void stoplicht() {

//lichten op groen
  if (millis() - timer < groenTijd) { 
    digitalWrite(groenLicht, HIGH);
    digitalWrite(oranjeLicht, LOW);
    digitalWrite(roodLicht, LOW);
    roodFase = false;
  }

//lichten op oranje
  if (millis() - timer > groenTijd && millis() - timer < (groenTijd + oranjeTijd)) {
    digitalWrite(groenLicht, LOW);
    digitalWrite(oranjeLicht, HIGH);
    digitalWrite(roodLicht, LOW);
    roodFase = true;
  }

//lichten op rood
  if (millis() - timer > (groenTijd + oranjeTijd) && millis() - timer < cyclusTijd) {
    digitalWrite(groenLicht, LOW);
    digitalWrite(oranjeLicht, LOW);
    digitalWrite(roodLicht, HIGH);
    roodFase = true; 
  }
}

//situatie waarbij de verkeerslichten knipperen (uit/storing)
void uitStand() {
  digitalWrite(groenLicht, LOW);
  digitalWrite(roodLicht, LOW);
  unsigned long currentMillis = millis(); //onthoud huidige tijd

  //check of huidige tijd langer is dan knipperinterval, zo ja flip dan de ledstatus
  if (currentMillis - laatsteKnipper >= knipperInterval) {
    laatsteKnipper = currentMillis; //verander oude tijd naar huidige tijd
    if (oranjeState == LOW) {
      oranjeState = HIGH;
    } else {
      oranjeState = LOW;
    }
    digitalWrite(oranjeLicht, oranjeState);
  }
}

void drukKnop(){
  if(uitgeschakeld){
    uitgeschakeld = false;
  } else {
    uitgeschakeld = true;
  }
  
}

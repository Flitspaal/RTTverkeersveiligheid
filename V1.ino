#include <Wire.h>
#include <LiquidCrystal_I2C.h> //LCD i2c lib

// HET VERKEERSLICHT DAT JE SIMULEERT IS HET STOPLICHT VAN DE PERSOON DIE AANGEREDEN KAN WORDEN!!! niet van degene die door rood gaat rijden dus

//TODO
//Wiskundig model verder implementeren
//vochtigheid/temp meerekenen in formule
//helling vd weg meerekenen als variabelen
//vertraging berekenen aan de hand van afstandssensor (2x2 of meer meten en afstand over tijd berekenen)
//distance meerekenen in de door rood rij kans
//2e stoplicht die de rood fase bepaalt voor het te waarschuwen stoplicht
//variabele die de snelheid vd weg aangeeft/geeltijd aanpast

//VERANDER DEZE PINNEN NAAR DE JUISTE VOOR JOU MICROCONTROLLER
//S = slachtoffer
//W = Wegpiraat aka die door rood gaat rijden
#define POTMETER A0
#define warning 30
#define knopPin D4
#define groenLichtS 12
#define oranjeLichtS 13
#define roodLichtS 15
#define echoPin 13 // attach pin D7 Arduino to pin Echo of HC-SR04
#define trigPin 15 //attach pin D8 Arduino to pin Trig of HC-SR04

//fases verkeerslichten
bool uitgeschakeld = false;
bool risicoFase = false;
int groenTijd = 6000 ;
int oranjeTijd = 4500;        //tijd van oranje knipperen, 80km/h weg = 4.5 sec
int roodTijd = 6000;
int cyclusTijd = groenTijd + oranjeTijd + roodTijd;
const int knipperInterval = 500;

//variabelen verkeerslichten
int ledState = LOW;
unsigned long laatsteKnipper = 0;
unsigned long timer;

//variabelen afstandssensor
long duration;  // variable for the duration of sound wave travel
int distance;   // variable for the distance measurement in cm

//PROTOTYPES
double remWeg(double);
double potmeter();
void controllLCD(float s, float r);
void stoplicht();
void uitStand();
void knipper(int led, int interval);
void checkAfstand();

LiquidCrystal_I2C lcd(0x27, 20, 4);

void setup() {
  Serial.begin(9600);

  //LCD setup
  lcd.begin();
  lcd.backlight();
  lcd.print("INFO");
  lcd.setCursor(0, 1);
  lcd.print("Screen");

  //setup timer and pins voor verkeerslichten
  timer = millis();
  pinMode(groenLichtS, OUTPUT);
  pinMode(oranjeLichtS, OUTPUT);
  pinMode(roodLichtS, OUTPUT);

  //setup knop voor switchen tussen verkeerslicht modus
  pinMode(knopPin, INPUT_PULLUP);

  //setup pins voor afstandssensor
  pinMode(trigPin, OUTPUT); // Sets the trigPin as an OUTPUT
  pinMode(echoPin, INPUT); // Sets the echoPin as an INPUT
}

void loop() {
  checkAfstand();


  double p = potmeter(); // waarde van de potmeter
  double r = remWeg(p); // remweg berekenen
  controllLCD(p, r);

  boolean knopIn = !digitalRead(knopPin);
  if (knopIn) {
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

double remWeg(double v) { //calculates remweg
  double temp = (((v / 10) * (v / 10)) / 2);
  return temp;
}

double potmeter() {
  double v = analogRead(POTMETER);
  double temp = v / 6.83;
  return temp;
}

void controllLCD(float s, float r) {
  lcd.clear();// clear previous values from screen
  lcd.print("KM/U");
  lcd.setCursor(7, 0);
  lcd.print(s);
  lcd.setCursor(0, 1);
  lcd.print("Remweg");
  lcd.setCursor(7, 1);
  lcd.print(r);
  lcd.setCursor(14, 1);
  lcd.print("M");

  if (r > warning && risicoFase == true) { // als remafstand groter is dan 30 geef dan waarschuwing
    lcd.setCursor(0, 2);
    lcd.print("---!!---");
    knipper(roodLichtS, 200);
  }
  if (uitgeschakeld && risicoFase == false) {
    lcd.setCursor(0, 2);
    lcd.print("uit");
  }
  delay(200);
}

void stoplicht() {
  //lichten op groen
  if (millis() - timer < groenTijd) {
    digitalWrite(groenLichtS, HIGH);
    digitalWrite(oranjeLichtS, LOW);
    digitalWrite(roodLichtS, LOW);
    risicoFase = true;
  }

  //lichten op oranje
  if (millis() - timer > groenTijd && millis() - timer < (groenTijd + oranjeTijd)) {
    digitalWrite(groenLichtS, LOW);
    digitalWrite(oranjeLichtS, HIGH);
    digitalWrite(roodLichtS, LOW);
    risicoFase = false;
  }

  //lichten op rood
  if (millis() - timer > (groenTijd + oranjeTijd) && millis() - timer < cyclusTijd) {
    digitalWrite(groenLichtS, LOW);
    digitalWrite(oranjeLichtS, LOW);
    digitalWrite(roodLichtS, HIGH);
    risicoFase = false;
  }
}

//situatie waarbij de verkeerslichten knipperen (uit/storing)
void uitStand() {
  digitalWrite(groenLichtS, LOW);
  digitalWrite(roodLichtS, LOW);
  knipper(oranjeLichtS, knipperInterval);
}

//functie om led te laten knipperen
void knipper(int led, int interval) {
  unsigned long currentMillis = millis(); //onthoud huidige tijd

  //check of huidige tijd langer is dan knipperinterval, zo ja flip dan de ledstatus
  if (currentMillis - laatsteKnipper >= interval) {
    laatsteKnipper = currentMillis; //verander oude tijd naar huidige tijd
    if (ledState == LOW) {
      ledState = HIGH;
    } else {
      ledState = LOW;
    }
    digitalWrite(led, ledState);
  }
}

void drukKnop() {
  if (uitgeschakeld) {
    uitgeschakeld = false;
  } else {
    uitgeschakeld = true;
    risicoFase = false;
  }
}

//https://create.arduino.cc/projecthub/abdularbi17/ultrasonic-sensor-hc-sr04-with-arduino-tutorial-327ff6
void checkAfstand() {
  // Clears the trigPin condition
  digitalWrite(trigPin, LOW);
  delayMicroseconds(2);
  // Sets the trigPin HIGH (ACTIVE) for 10 microseconds
  digitalWrite(trigPin, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigPin, LOW);
  // Reads the echoPin, returns the sound wave travel time in microseconds
  duration = pulseIn(echoPin, HIGH);
  // Calculating the distance
  distance = duration * 0.034 / 2; // Speed of sound wave divided by 2 (go and back)
}

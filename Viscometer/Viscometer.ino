int StartWertMessung;                                              //Definiert die Variabel StartWertMessung und setzt den Wert auf X
int TolleranzWert = 5;                                                  //Definiert die Variable TolleranzWert (ist der Wert - in der der Viscositätswert schwanken darf...)
int eepromBeckenTemp;
int BeckenMinTemp;
int mittelwert;

#include <EEPROM.h>

//Start Display
#include <SPI.h>                                                        //Lade Extension für OLED
#include <Wire.h>                                                       //Lade Extension für OLED
#include <Adafruit_GFX.h>                                               //Lade Extension für OLED
#include <Adafruit_SSD1306.h>                                           //Lade Extension für OLED
#define OLED_MOSI   39                                                  //Data port zum OLED Display
#define OLED_CLK   37                                                   //CLK port zum OLED Display
#define OLED_DC    35                                                   //DC Port zum OLED DISPLAY
#define OLED_CS    31                                                   //CS Port zum OLED Display
#define OLED_RESET 33                                                   //RST Port zum OLED Display
Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);            //Initialisiere das OLED Display
#define NUMFLAKES 10
#define XPOS 0
#define YPOS 1
#define DELTAY 2
//Ende Display

//LED Leiste
#include <Adafruit_NeoPixel.h>                                          //Lade Extension für LED-Leiste WS
#ifdef __AVR__
#include <avr/power.h>                                                  //Lade Extension für LED-Leiste WS
#endif
#define PIN 45                                                          //Definiert den digital PIN der LED Leiste  
#define NUMPIXELS 8                                                     //Definiert die Anzahl der LED´s am Port
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);       //Initialisiere die Lib. für die LED-Leiste
//Ende LED Leiste


//Taster
const int buttonPinPlus = 24;                                           //Definiert den digitalen PIN für den Plus Taster
const int buttonPinMinus = 25;                                          //Definiert den digitalen PIN für den Minus Taster
int buttonState = 0;                                                    //Definiiert die Variable buttonState und setzt sie auf 0;
int SollWert = StartWertMessung;                                        //Definiert die Variable SollWert und weißt Ihr einen definierten Startwert zu
//ENDE TASTER


//RELAY
const int relaisPin = 26;                                               //Definiert einen PWM Port für das schalten des Relais.
//ENDE RELAY


// Variablen zum Glätten
int FiltVal = StartWertMessung;                                       //Definiert Variable zum glätten der Werte
int FF;                                                               //Definiert den Glättungsfaktor 1=werte werden 1:1 übernommen 9=es werden nur 10% veränderung übernommen
int NewVal = StartWertMessung;                                        //Definiert Variable zum glätten der Werte
//Ende Variablen zum Glätten

//BEGIN MOTOR BOARD
//Channel B
int directionPin = 13;                                                //Definiert für Channel B am Motorshield den Port
int pwmPin = 11;                                                      //Definiert für Channel B am Motorshield den Port
int brakePin = 8;                                                     //Definiert für Channel B am Motorshield den Port
int currentPin = A15;                                                  //Definiert für Channel B am Motorshield den Port
//ENDE MOTOR

//START DS TEMP
#include <OneWire.h>                                                  //Lade Lib für den wasserdichten Beckentemperaturfühler
#include <DallasTemperature.h>                                        //Lade Lib für den wasserdichten Beckentemperaturfühler

#define ONE_WIRE_BUS 41                                               //Definiert den PIN den wasserdichten Beckentemperaturfühler
OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
//END DS TEMP

//START DHT SENSOR TEMP/HYM                                           //Lade Lib für den kombinierten Raumluft Temp/Feuchtigkeits Sensor
#include "DHT.h"                                                      //Lade Lib für den kombinierten Raumluft Temp/Feuchtigkeits Sensor            
#define DHTPIN 22                                                     //Definiert den PIN für den Raumluft Temp/Feuchtigkeits Sensor
#define DHTTYPE DHT22                                                 //Definiere TYP für den den Raumluft Temp/Feuchtigkeits Sensor
DHT dht(DHTPIN, DHTTYPE);
//END DHT SENSOR



void setup() {
  Serial.begin(9600);                                                 //Initialisiere Serielle-Schnittstell


  //LED LEISTE
#if defined (__AVR_ATtiny85__)
  if (F_CPU == 16000000) clock_prescale_set(clock_div_1);
#endif
  pixels.begin();                                                     //Initialisiere NeoPixel LED Leiste
  //ENDE LED LEISTE

  //RELAYS
  pinMode(relaisPin, OUTPUT);                                          // Den PWM PIN "relaisPin" als Ausgangssignal setzen.
  //RELAYS END

  //TASTER
  pinMode(buttonPinPlus, INPUT);                                      //Setzt den Taster Port als Input
  pinMode(buttonPinMinus, INPUT);                                     //Setzt den Taster Port als Input
  //ENDE TASTER

  // Start MOTOR
  pinMode(directionPin, OUTPUT);                                      //Initiates Motor Channel B pin
  pinMode(brakePin, OUTPUT);                                          //Initiates Brake Channel B pin
  //ENDE MOTOR

  //START DS TEMP
  sensors.begin();                                                        //Initialisiere Beckentemperatur
  //END DS TEMP

  //DHT BEGIN
  dht.begin();                                                            //Initialisiere Raumfeuchtigkeit & Raumtemperatur Sensor
  //DHT END

  //START DISPLAY
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  // init done
  // Clear the buffer.
  display.clearDisplay();                                                 //Leert das Display
  //Ende DISPLAY


  BeckenMinTemp = EEPROM.read(0);                                        //EEPROM AUSLESEN Beckentemp = 0
  SollWert = EEPROM.read(1);                                             //EEPROM AUSLESEN Viscositätssollwert = 1
  //SollWert = 325;
}


void loop() {

  


  //START DS TEMP
  sensors.requestTemperatures();                                          //fragt Beckentemperatur ab
  //ENDE DS TEMP

  //START DHT
  float h = dht.readHumidity();                                           //Luftfeuchte auslesen
  float t = dht.readTemperature();                                        //Temperatur auslesen
  //END DHT

  //temp abfrage des Beckens -- start des Motor

  //Temp unter XX°C kein start und led auf blau

  if (sensors.getTempCByIndex(0) < BeckenMinTemp) {                         //Wenn Temp zu Kalt:

    if (digitalRead(buttonPinPlus) == 0) {                                  //Taster für BeckenTempAnpassung
      BeckenMinTemp = BeckenMinTemp + 1;
      EEPROM.write(0, BeckenMinTemp);
    }
    if (digitalRead(buttonPinMinus) == 0) {                                 //Taster für BeckenTempAnpassung
      BeckenMinTemp = BeckenMinTemp - 1;
      EEPROM.write(0, BeckenMinTemp);                    //Daten in EEPROM Schreiben
    }

    //Relais
    digitalWrite(relaisPin, HIGH);                                           //Relais aus
    //Ende Relais

    //START DISPLAY AUSGABE
    //display.invertDisplay(false);                                          // Display wird invertiert angezeig d.h. weißer Hintergrund - schwarze Schrift
    display.setTextSize(2);
    display.setTextColor(WHITE);
    display.setCursor(0, 2);
    display.println(" ACHTUNG!");                                         //Fehler ausgabe auf dem Display
    display.setTextSize(1);
    display.println("  Leim zu kalt");                                    //Fehler ausgabe auf dem Display
    display.setTextSize(2);
    display.println(" ");                                                 //LeerZeile
    display.print(" "); display.print(sensors.getTempCByIndex(0)); display.print("/"); display.print(BeckenMinTemp); //Aktueller Wert (aktuellerwert/sollwert)
    display.display();                                                    //Display ausgabe
    display.clearDisplay();                                               //Leert das Display
    //ENDE DISPLAY AUSGABE

    //MOTOR AUS!
    digitalWrite(directionPin, HIGH);                                        //Schaltet den Motor aus
    digitalWrite(brakePin, HIGH);                                            //Schaltet den Motor aus
    analogWrite(pwmPin, 0);                                                  //Motor geschwindikeit aus
    //ENDE MOTOR AUS!

    // LED AUF BLAU!
    for (int i = 0; i < NUMPIXELS; i++) {
      pixels.setPixelColor(i, pixels.Color(0, 0, 255));                    // BLAUE LED
      pixels.show();                                                        // This sends the updated pixel color to the hardware.
    }
    //ENDE LED AUF BLAU

  }
  else {


    // Eigentlicher Start!!!

    //TASTER SOLL WERT
    if (digitalRead(buttonPinPlus) == 0) {
      SollWert = SollWert + 1;
      EEPROM.write(1, SollWert);
    }
    if (digitalRead(buttonPinMinus) == 0) {
      SollWert = SollWert - 1;
      EEPROM.write(1, SollWert);
    }
    //ENDE TASTER SOLLWERT

    //Relais
    digitalWrite(relaisPin, HIGH);                                           //Relais aus
    //Ende Relais

    //START MOTOR
    //backward @ half speed
    digitalWrite(directionPin, HIGH);                                           //Establishes forward direction of Channel A
    digitalWrite(brakePin, LOW);                                                //Disengage the Brake for Channel A
    analogWrite(pwmPin, 255);                                                   //Spins the motor on Channel A at full speed
    //ENDE  MOTOR

    //glaetten der werte
    NewVal = analogRead(currentPin);
    FF = 5;
    FiltVal = ((FiltVal * FF) + NewVal) / (FF + 1.0);
    
    //FiltVal = analogRead(currentPin);         //Ausgabe des original Werts

    display.setCursor(0, 0);                                                  //Setzt für das OLED DISPLAY den Beginn auf 0.0 und die Schriftfarbe auf Weiß.
    display.setTextColor(WHITE);

    //Tolleranz abfrage
    if (FiltVal > (SollWert + TolleranzWert)) {
      //display.invertDisplay(true);
      digitalWrite(relaisPin, LOW);                                          //Relais an
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.println("Mehr Wasser");                                      // Leere Zeile
      display.setTextSize(2);
      //display.setTextColor(BLACK, WHITE);                                   // 'inverted' text
      display.print(FiltVal); display.print("/"); display.println(SollWert);

      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(255, 0, 0));                   // Orange ROT
        pixels.show();                                                        // This sends the updated pixel color to the hardware.
      }

    }
    else if (FiltVal < (SollWert - TolleranzWert)) {
     // display.invertDisplay(true);
      digitalWrite(relaisPin, HIGH);                                          //Relais aus
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.println("Weniger Wasser");                                      // Leere Zeile
      display.setTextSize(2);
      //display.setTextColor(BLACK, WHITE);                                   // 'inverted' text
      display.print(FiltVal); display.print("/"); display.println(SollWert);

      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(255, 102, 0));                   // Orange LED
        pixels.show();                                                        // This sends the updated pixel color to the hardware.
      }
    }
    else {
      display.invertDisplay(false);
      digitalWrite(relaisPin, HIGH);                                           //Relais aus
      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.println("O.K.");                                                // Leere Zeile
      display.setTextSize(2);
      display.print(FiltVal); display.print("/"); display.println(SollWert);

      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(0, 150, 0));                     // Grüne LED
        pixels.show();                                                        // This sends the updated pixel color to the hardware.
      }
    }
    //Ende Tolleranz

    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println(" ");                                                     // Leere Zeile
    display.print(sensors.getTempCByIndex(0));
    display.println(" Leimtemp.");                                            //Beckentemperatur
    display.print(h);
    display.println(" Luftfeuch.");                                           // Luftfeuchtigkeit
    display.print(t);
    display.println(" Raumtemp.");                                            // Ungebungslufttemperatur
    display.display();

    display.clearDisplay();
  }

  //ENDE DISPLAY AUSGABE

  Serial.print(FiltVal);                                               //Geglätteter Wert des Current Signals
  Serial.print("\t");
  Serial.print(h);                                                            //Raumfeuchtigkeit
  Serial.print("\t"); 
  Serial.print(t);                                                            //Raumtemperatur
  Serial.print("\t");
  Serial.print(sensors.getTempCByIndex(0));                                 //Beckentemperatur
  Serial.print("\t");
  Serial.print(SollWert + TolleranzWert);                                   //Ausgabe des Tolleranzwertes(max) im Plotter
  Serial.print("\t");
  Serial.println(SollWert - TolleranzWert);                                 //Ausgabe des Tolleranzwertes(min) im Plotter
}




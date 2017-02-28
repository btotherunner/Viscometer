int StartWertMessung = 50;                                              //Definiert die Variabel StartWertMessung und setzt den Wert auf X
int TolleranzWert = 5;                                                  //Definiert die Variable TolleranzWert (ist der Wert - in der der Viscositätswert schwanken darf...)
int BeckenMinTemp = 20;                                                 //Definiert die mindest Temp des Leim Beckens

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
#define PIN 45                                                           //Definiert den digital PIN der LED Leiste  
#define NUMPIXELS      4                                                 //Definiert die Anzahl der LED´s am Port
Adafruit_NeoPixel pixels = Adafruit_NeoPixel(NUMPIXELS, PIN, NEO_GRB + NEO_KHZ800);       //Initialisiere die Lib. für die LED-Leiste
//Ende LED Leiste


//Taster
const int buttonPinPlus = 24;                                           //Definiert den digitalen PIN für den Plus Taster
const int buttonPinMinus = 25;                                          //Definiert den digitalen PIN für den Minus Taster
int buttonState = 0;                                                    //Definiiert die Variable buttonState und setzt sie auf 0;
int SollWert = StartWertMessung;                                        //Definiert die Variable SollWert und weißt Ihr einen definierten Startwert zu
//ENDE TASTER


//RELAY
const int relaisPin = 10;                                               //Definiert einen PWM Port für das schalten des Relais.
//ENDE RELAY


// Variablen zum Glätten
int FiltVal = StartWertMessung;                                       //Definiert Variable zum glätten der Werte
int FF;                                                               //Definiert den Glättungsfaktor 1=werte werden 1:1 übernommen 9=es werden nur 10% veränderung übernommen
int NewVal = StartWertMessung;                                        //Definiert Variable zum glätten der Werte

int FiltVal2 = StartWertMessung;                                      //Definiert Variable zum glätten der Werte
int FF2;                                                              //Definiert den Glättungsfaktor 1=werte werden 1:1 übernommen 9=es werden nur 10% veränderung übernommen
int NewVal2 = StartWertMessung;                                       //Definiert Variable zum glätten der Werte

int FiltVal3 = StartWertMessung;                                      //Definiert Variable zum glätten der Werte
int FF3;                                                              //Definiert den Glättungsfaktor 1=werte werden 1:1 übernommen 9=es werden nur 10% veränderung übernommen
int NewVal3 = StartWertMessung;                                       //Definiert Variable zum glätten der Werte

int mittelFitalVal = StartWertMessung;                                //definiert die Variable mittelFitalVal und weißt den geglätteten Wert zu.
//Ende Variablen zum Glätten

//BEGIN MOTOR BOARD
//Channel B
int directionPin = 13;                                                //Definiert für Channel B am Motorshield den Port
int pwmPin = 11;                                                      //Definiert für Channel B am Motorshield den Port
int brakePin = 8;                                                     //Definiert für Channel B am Motorshield den Port
int currentPin = A1;                                                  //Definiert für Channel B am Motorshield den Port
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
  Serial.begin(9600);                                                 //Initialisiere Serielle-Schnittstelle

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

  //Temp unter 25°C kein start und led auf blau
  if (sensors.getTempCByIndex(0) < BeckenMinTemp) {                         //Wenn Temp zu Kalt:

    //START DISPLAY AUSGABE
      display.invertDisplay(true);                                          // Display wird invertiert angezeig d.h. weißer Hintergrund - schwarze Schrift
      display.setTextSize(2);
      display.setTextColor(WHITE);
      display.setCursor(0, 2);
      display.println(" ACHTUNG!");                                         //Fehler ausgabe auf dem Display
      display.setTextSize(1);
      display.println("  Leim zu kalt");                                    //Fehler ausgabe auf dem Display                        
      display.setTextSize(2);
      display.println(" ");                                                 //LeerZeile
      display.print(" ");display.print(sensors.getTempCByIndex(0)); display.print("/"); display.print(BeckenMinTemp); //Aktueller Wert (aktuellerwert/sollwert)
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
      pixels.setPixelColor(i, pixels.Color(51, 0, 255));                    // BLAUE LED
      pixels.show();                                                        // This sends the updated pixel color to the hardware.
    }
    //ENDE LED AUF BLAU

  }
  else {


    // Eigentlicher Start!!!

    //TASTER SOLL WERT
    if (digitalRead(buttonPinPlus) == 0) {
      SollWert = SollWert + 1;
    }
    if (digitalRead(buttonPinMinus) == 0) {
      SollWert = SollWert - 1;
    }
    //ENDE TASTER SOLLWERT

    //START MOTOR
    //backward @ half speed
    digitalWrite(directionPin, HIGH);                                           //Establishes forward direction of Channel A
    digitalWrite(brakePin, LOW);                                                //Disengage the Brake for Channel A
    analogWrite(pwmPin, 255);                                                   //Spins the motor on Channel A at full speed
    //ENDE  MOTOR


    //glaetten der werte
    NewVal = analogRead(currentPin);
    FF = 9;
    FiltVal = ((FiltVal * FF) + NewVal) / (FF + 1.0);
    //zweite glaettung
    NewVal2 = FiltVal;
    FF2 = 9;
    FiltVal2 = ((FiltVal2 * FF2) + NewVal2) / (FF2 + 1.0);
    //dritte glaettung
    NewVal3 = FiltVal2;
    FF3 = 9;
    FiltVal3 = ((FiltVal3 * FF3) + NewVal3) / (FF2 + 1.0);

    //mitteln der werte
    mittelFitalVal = 0;

    mittelFitalVal = FiltVal3;
    //ende glaetten


    display.setCursor(0, 0);                                                  //Setzt für das OLED DISPLAY den Beginn auf 0.0 und die Schriftfarbe auf Weiß.
    display.setTextColor(WHITE);

    //Tolleranz abfrage
    if (mittelFitalVal > (SollWert + TolleranzWert)) {
      display.invertDisplay(true);                                            // Display wird invertiert angezeig d.h. weißer Hintergrund - schwarze Schrift
      digitalWrite(relaisPin, LOW);                                           // Relais aus
      display.setTextSize(1);                                                 // Schriftgroeße 1 (klein)
      display.println("Mehr Wasser");                                         // Leere Zeile
      display.setTextSize(2);                                                 // Schriftgroeße 2 (mittel)
      display.print(mittelFitalVal); display.print("/"); display.println(SollWert);         // Ausgabe des aktuellen Viskositaetswerts und den SollWert (bsp:  45/50)
      display.clearDisplay();                                                 // Loescht das Display

      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(255, 0, 0));                     // Rote LED
        pixels.show(); // This sends the updated pixel color to the hardware.
      }
    }
    else if (mittelFitalVal < (SollWert - TolleranzWert)) {
      display.invertDisplay(true);
      digitalWrite(relaisPin, HIGH);                                          //Relais an
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.println("Weniger Wasser");                                      // Leere Zeile
      display.setTextSize(2);
      //display.setTextColor(BLACK, WHITE);                                   // 'inverted' text
      display.print(mittelFitalVal); display.print("/"); display.println(SollWert);

      for (int i = 0; i < NUMPIXELS; i++) {
        pixels.setPixelColor(i, pixels.Color(255, 102, 0));                   // Orange LED
        pixels.show();                                                        // This sends the updated pixel color to the hardware.
      }
    }
    else {
      display.invertDisplay(false);
      digitalWrite(relaisPin, LOW);                                           //Relais aus
      display.setTextColor(WHITE);
      display.setTextSize(1);
      display.setTextColor(WHITE);
      display.println("O.K.");                                                // Leere Zeile
      display.setTextSize(2);
      display.print(mittelFitalVal); display.print("/"); display.println(SollWert);

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

  Serial.print(analogRead(currentPin));                                       //Ausgabe des Analogen CurrentSignals
  Serial.print("\t");
  Serial.print(mittelFitalVal);                                               //Geglätteter Wert des Current Signals
  Serial.print("\t");
  Serial.print(h);                                                            //Raumfeuchtigkeit
  Serial.print("\t");
  Serial.print(t);                                                            //Raumtemperatur
  Serial.print("\t");
  Serial.println(sensors.getTempCByIndex(0));                                 //Beckentemperatur

  delay(333);
}

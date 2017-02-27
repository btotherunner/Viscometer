
//Start Display
  #include <SPI.h>
  #include <Wire.h>
  #include <Adafruit_GFX.h>
  #include <Adafruit_SSD1306.h>
  
  // If using software SPI (the default case):
  #define OLED_MOSI   39  // - data (9)
  #define OLED_CLK   37  // - clk (10)
  #define OLED_DC    35   // - dc (11)
  #define OLED_CS    31 //  (12)
  #define OLED_RESET 33 //  (13)
  
  Adafruit_SSD1306 display(OLED_MOSI, OLED_CLK, OLED_DC, OLED_RESET, OLED_CS);
  
  
  #define NUMFLAKES 10
  #define XPOS 0
  #define YPOS 1
  #define DELTAY 2
//Ende Display

int StartWertMessung = 50;

//Taster
  const int buttonPinPlus = 24;
  const int buttonPinMinus = 25;
  int buttonState = 0;
  int SollWert = StartWertMessung;
//ENDE TASTER


//RELAY
  const int relaisPin = 10;
//ENDE RELAY


// Werte zum Mitteln
  int FiltVal = StartWertMessung;
  int FF;
  int NewVal = StartWertMessung;
  
  int FiltVal2 = StartWertMessung;
  int FF2;
  int NewVal2 = StartWertMessung;
  
  int FiltVal3 = StartWertMessung;
  int FF3;
  int NewVal3 = StartWertMessung;
  
  int sumFiltVal = StartWertMessung;
  int mittelFitalVal = StartWertMessung;

//Ende Werte zum Mitteln

//BEGIN MOTOR BOARD
  //Channel B
  int directionPin = 13;
  int pwmPin = 11;
  int brakePin = 8;
  int currentPin = A1;


  // constants
  float volt_per_amp = 1.65; // resolution according to hardware page
  
  // variables
  float currentRaw; // the raw analogRead ranging from 0-1023
  float currentVolts; // raw reading changed to Volts
  float currentAmps; // Voltage reading changed to Amps
  float SumcurrentAmps; //SUMME MITTELWERT ERRECHNUNG
  float MittelwertcurrentAmps; //MITTELWERT ERRECHNUNG
//ENDE MOTOR

//START DS TEMP
  //Include libraries
  #include <OneWire.h>
  #include <DallasTemperature.h>
  
  // Data wire is plugged into pin 2 on the Arduino
  #define ONE_WIRE_BUS 41
  // Setup a oneWire instance to communicate with any OneWire devices (not just Maxim/Dallas temperature ICs)
  OneWire oneWire(ONE_WIRE_BUS);
  // Pass our oneWire reference to Dallas Temperature.
  DallasTemperature sensors(&oneWire);
//END DS TEMP

//START DHT SENSOR TEMP/HYM
  #include "DHT.h"
  #define DHTPIN 22
  #define DHTTYPE DHT22 //DHT11, DHT21, DHT22
  DHT dht(DHTPIN, DHTTYPE);
//END DHT SENSOR


void setup()   {
  Serial.begin(9600);

  //RELAYS
  pinMode(relaisPin, OUTPUT); // Den PWM PIN "relaisPin" als Ausgangssignal setzen.
  //RELAYS END
  
  //TASTER
  pinMode(buttonPinPlus, INPUT);
  pinMode(buttonPinMinus, INPUT);

  //ENDE TASTER

  // Start MOTOR
  pinMode(directionPin, OUTPUT); //Initiates Motor Channel B pin
  pinMode(brakePin, OUTPUT); //Initiates Brake Channel B pin
  //ENDE MOTOR

  //START DS TEMP
  sensors.begin();
  //END DS TEMP

  //DHT BEGIN
  dht.begin();
  //DHT END

  //START DISPLAY
  // by default, we'll generate the high voltage from the 3.3v line internally! (neat!)
  display.begin(SSD1306_SWITCHCAPVCC);
  // init done

  // Show image buffer on the display hardware.
  // Since the buffer is intialized with an Adafruit splashscreen
  // internally, this will display the splashscreen.

  //display.display();
  //delay(1000);

  display.drawPixel(10, 10, WHITE);
  // Clear the buffer.
  display.clearDisplay();
  //display.invertDisplay(true);


  
  // text display tests
  //display.setTextSize(1);
  //display.setTextColor(WHITE);
  //display.setCursor(0,0);
  //display.println("Hello, world!");
  //display.setTextColor(BLACK, WHITE); // 'inverted' text
  //display.println(3.141592);
  //display.setTextSize(2);
  //display.setTextColor(WHITE);
  //display.print("0x"); display.println(0xDEADBEEF, HEX);
  //display.display();
  //delay(2000);
  //display.clearDisplay();
}
//Ende DISPLAY

void loop() {
  
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
  digitalWrite(directionPin, HIGH); //Establishes forward direction of Channel A
  digitalWrite(brakePin, LOW);   //Disengage the Brake for Channel A
  analogWrite(pwmPin, 255);   //Spins the motor on Channel A at full speed
  //ENDE  MOTOR

  //START DS TEMP
  sensors.requestTemperatures();
  //ENDE DS TEMP

  //START DHT
  float h = dht.readHumidity();     //Luftfeuchte auslesen
  float t = dht.readTemperature();  //Temperatur auslesen
  //END DHT

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
  sumFiltVal = 0;


  mittelFitalVal = FiltVal3;
  //ende glaetten

  //START DISPLAY AUSGABE
  display.setCursor(0, 0);
  display.setTextColor(WHITE);

  //Tolleranz abfrage
  if (mittelFitalVal > (SollWert + 5)) {
    display.invertDisplay(true);
    digitalWrite(relaisPin, LOW); //Relais aus
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("Mehr Wasser");    // Leere Zeile
    display.setTextSize(2);
   // display.setTextColor(BLACK, WHITE); // 'inverted' text
    display.print(mittelFitalVal); display.print("/"); display.println(SollWert);
  }
  else if (mittelFitalVal < (SollWert - 5)) {
    display.invertDisplay(true);
    digitalWrite(relaisPin, HIGH); //Relais an
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("Weniger Wasser");    // Leere Zeile
    display.setTextSize(2);
    //display.setTextColor(BLACK, WHITE); // 'inverted' text
    display.print(mittelFitalVal); display.print("/"); display.println(SollWert);
  }
  else {
    display.invertDisplay(false);
    digitalWrite(relaisPin, LOW); //Relais aus
    display.setTextColor(WHITE);
    display.setTextSize(1);
    display.setTextColor(WHITE);
    display.println("O.K.");    // Leere Zeile
    display.setTextSize(2);
    display.print(mittelFitalVal); display.print("/"); display.println(SollWert);
  }
  //Ende Tolleranz


  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.println(" ");    // Leere Zeile
  display.print(sensors.getTempCByIndex(0));
  display.println(" Leimtemp.");  //Beckentemperatur
  display.print(h);
  display.println(" Luftfeuch.");    // Luftfeuchtigkeit
  display.print(t);
  display.println(" Raumtemp.");   // Ungebungslufttemperatur
  display.display();

  display.clearDisplay();
//ENDE DISPLAY AUSGABE

  Serial.print(analogRead(currentPin));     //Ausgabe des Analogen CurrentSignals
  Serial.print("\t");
  Serial.print(mittelFitalVal);             //Geglätteter Wert des Current Signals
  Serial.print("\t");
  Serial.print(h);                           //Raumfeuchtigkeit
  Serial.print("\t");
  Serial.print(t);                           //Raumtemperatur
  Serial.print("\t");
  Serial.println(sensors.getTempCByIndex(0));  //Beckentemperatur

  delay(333);
}

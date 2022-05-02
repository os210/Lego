// Libraries einbinden

#include <Servo.h>
#include <Wire.h>
#include <Adafruit_MotorShield.h>
#include "utility/Adafruit_MS_PWMServoDriver.h"
#include "Adafruit_TCS34725.h"
#include <Ultrasonic.h>

//Bauteile initialisieren
Adafruit_TCS34725 tcs = Adafruit_TCS34725(TCS34725_INTEGRATIONTIME_50MS, TCS34725_GAIN_1X);
Adafruit_MotorShield AFMS = Adafruit_MotorShield(96);
Adafruit_DCMotor *motor_rechts = AFMS.getMotor(3);
Adafruit_DCMotor *motor_links = AFMS.getMotor(1);
//Anschluss 2 defekt.


Ultrasonic ultrasonic (11);
const int PIN_EIN_AUS = 10;


//Set Speed
int speed_1 = 100;
int speed_2 = 30;

//Zustaende
enum zustand_type {Z_stopp, Z_links, Z_rechts};
enum zustand_type zustand;

//Ereigniss
bool E_rot;
bool E_taste = true;
bool E_taste_tmp = 0;
bool E_distance_low = false;


void stopp() {

    motor_links->run(BACKWARD);
    motor_links->setSpeed(0);
    motor_rechts->run(BACKWARD);
    motor_rechts->setSpeed(0);
    
    Serial.print("stopp");

    if((E_taste == 0) && (E_rot == 0)) {
      zustand = Z_rechts;
    }
    else if((E_taste == 0) && (E_rot == 1)) {
      zustand = Z_links;
    }
  }

void links_fahren() {

    motor_links->run(FORWARD);
    motor_links->setSpeed(speed_2);
    motor_rechts->run(BACKWARD);
    motor_rechts->setSpeed(speed_1);

    Serial.print("links");
    
    if((E_taste == 0) && (E_rot == 0)) {
      zustand = Z_rechts;
    }
    else if((E_taste == 0) && (E_rot == 1)) {
      zustand = Z_links;
    }
    else if((E_taste == 1)) {
      zustand = Z_stopp;
    }

  }

void rechts_fahren() {
  
    motor_links->run(BACKWARD);
    motor_links->setSpeed(speed_1);
    motor_rechts->run(FORWARD);
    motor_rechts->setSpeed(speed_2);


     if((E_taste == 0) && (E_rot == 0)) {
      zustand = Z_rechts;
    }
    else if((E_taste == 0) && (E_rot == 1)) {
      zustand = Z_links;
    }
    else if((E_taste == 1) || (E_distance_low = true)) {
      zustand = Z_stopp;
    }
}

void ereignisse() {


  //Distanzsensor
  long range;
  range = ultrasonic.MeasureInMillimeters(); 
  
  if ((range < 100)) {
    E_distance_low = true;
    }

  // Togglebutton

  if ((digitalRead(PIN_EIN_AUS) == LOW) && (E_taste_tmp == HIGH)) {
    E_taste ^= true;
    delay(20); 
  }
  E_taste_tmp=digitalRead(PIN_EIN_AUS);


  //Farbsensor
  
  uint16_t clearcol, red, green, blue;
  float average, r, g, b;
  delay(60); // Farbmessung dauert ca. 50ms
  tcs.getRawData(&red, &green, &blue, &clearcol);

  average = (red + green + blue) / 3;

  r = red / average;
  g = green / average;
  b = blue / average;

  if ((r > 1.4) && (g < 0.9) && (b < 0.9)) {
    E_rot = true;
  }
  else {
    E_rot = false;
  }

}

void verarbeitung(){
     switch(zustand){
      case Z_stopp: stopp(); break;
      case Z_links: links_fahren(); break;
      case Z_rechts: rechts_fahren(); break;
      default: stopp();
     }
}

void setup(){
  
  zustand = Z_stopp;
  pinMode(PIN_EIN_AUS,INPUT_PULLUP);

  // Kommunikation Motorshield
  AFMS.begin();
  motor_rechts->setSpeed(100);
  motor_rechts->run(RELEASE);
  motor_links->setSpeed(100);
  motor_links->run(RELEASE);

  // Serielle Kommunikation
  Serial.begin(9600);
  Serial.println("");
  delay(500);
  Serial.println("Verbindungstest");

  //Initialisiere Farbsensor

  if (tcs.begin()){
    // Alles OK
    Serial.println("Sensor gefunden");
  } 
  else {
    // Kein Sensor gefunden. Programm wird gestoppt
    Serial.println("TCS34725 nicht gefunden, Programmstoppt!");
    while (1);
  }
}


void loop() {
  ereignisse();
  verarbeitung();
}

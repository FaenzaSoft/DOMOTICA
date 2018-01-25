
/* Generatore impulsi PWM con ciclopwm variabile:
 * Genera una uscita PWM con andamento a crescere e poi a decrescere:
 * Evidenzia il duty cycle sul display, insieme alla tensione di uscita:
 * usare Arduino UNO e display SERIALE 2x16:
 * Video YouTube: https://youtu.be/301RBk-4-e8
 */
#include <Arduino.h>                   //inclusione della libreria base:
#include <Wire.h>                      // libreria necessaria pere diplay seriale:
#include <LiquidCrystal_I2C.h>         //inclusione della libreria per display seriale:
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);    // impostazione display seriale NewLiquidCristal:
//
int caricoPin = 9;             // carico analogico PWM pin 9:
int ciclopwm = 0;
int aumento = 1; 
int conta = 0;
int velocita = 25;
int aumentovelocita = 1;
int volt_ingresso = 12;
void setup()
{
  pinMode(caricoPin, OUTPUT);  
  lcd.begin(16,2); // Inizializzo LCD 16 caratteri per 2 linee, SERIALE: 
  lcd.clear();
}   
void loop() 
{
  conta = conta + 1;
  ciclopwm = ciclopwm + aumento;
  //
  if (ciclopwm > 255)
  {
  aumento = -1;
  }
  //
  if (ciclopwm < 1)
  {
  aumento = +1;
  delay(1000);
  }
  //
  analogWrite (caricoPin, ciclopwm);
  //
  //
  if (conta > 30)
  {
    velocita = velocita + aumentovelocita;
    if (velocita > 50) aumentovelocita = -1;
    if (velocita < 2) aumentovelocita = +1; 
    conta = 0;   
  }
  delay (velocita);  
  //
  lcd.setCursor(0, 0);
  lcd.print("dutycycle:      ");    
  lcd.setCursor(11, 0);
  lcd.print(ciclopwm);  
  lcd.setCursor(0, 1); 
  lcd.print("volt.....:      ");           
  lcd.setCursor(11, 1);
  lcd.print(volt_ingresso*ciclopwm/253);   
//
}
//FINE by SoftPlus per blog Mauro Alfieri































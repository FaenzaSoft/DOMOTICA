
/* Ver. 0.0 del 29/01/2018    
   Il crepuscolare HYBRID consente di attivare lampade anche di giorno, IN AUTOMATICO, per supplire
   a cadute di luminosità dovuta a passaggi nuvolosi, giornata grigia, crepuscolo.
   Lo fa attivando le lampade al 100% della potenza o in soluzioni intermedie (da zero a 255), come un rubinetto per liquidi.
   Ovviamente di notte e con poca luminosità le lampade sono attivate a potenza piena.
   Le lampade a led da utilizzare, una o più in parallelo, debbono essere a 12 volt per corrente continua;
   Il software consente di valutare la luminosità dell'ambiente e conseguentemente di attivare il relè fai da te PWM:
   Il display (a regime) evidenzia il duty cycle e la tensione di uscita. Si può anche omettere nella installazione finale:
   In fase di collaudo del software il display evidenzia sulla seconda riga i segmenti percepiti (da 0 a 1024):
   Usare Arduino UNO/nano e display SERIALE 2x16:
   Video YouTube: https://youtu.be/301RBk-4-e8
   FOTORESISTENZA:
   Utilizzare una normale fotoresistenza da 20k collegata con un capo al positivo dei 5 volt:
   L'altro capo va all'ingresso A0 di arduino; tra A0 e GND va collocata una resistenza da 15k:
   I PROBLEMI
   che si pongono riguardano principalmente la collocazione della fotoresistenza:
   1) la fotoresistenza deve misurare la luminosità dell'ambiente:
   2) è bene che sia inserita in un tubetto che la rende molto direttiva e insensibile alla luminosità della lampada:
   3) non va "puntata" sulla lampada.
   RISULTATI:
   Il software proposto rappresenta una soluzione semplice quanto efficace. 
   Le impostazioni di luminosità possono essere variate con un semplice trimmer che sostituisce 
   la resistenza da 15K montata a fianco della fotoresistenza.
   P.S.: si può utilizzare un relè fai da te senza il 7812 e con dimensioni ridotte, senza display e con dissipatore mignon:
         da abbinare ad un arduino nano, ciò può consentire la collocazione del tutto in uno spazio molto:
         ridotto (tipicamente una "cassetta" da muro per interruttori).
*/
#include <Arduino.h>                   //inclusione della libreria base:
#include <Wire.h>                      // libreria necessaria pere diplay seriale:
#include <LiquidCrystal_I2C.h>         //inclusione della libreria per display seriale:
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);    // impostazione display seriale NewLiquidCristal:
// LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);    // indirizzo LCD Philips PCF8574AT:
//
int caricoPin = 9;             // carico analogico PWM pin 9:
int sensorePin = A0;           // sensore fotoresistenza;
int luminosita = 700;          // valore da modificare, regola la luminosità dell'ambiente che si vuole ottenere:
unsigned long timer = 43200000;   // (60000*60*12 = quarantatremilioniduentemila = 12 ore) opzionale: 
int ciclopwm = 0;
int volt_ingresso = 12;
int segmenti = 0;
int somma = 0;
void setup()
{
  pinMode(caricoPin, OUTPUT);
  pinMode(sensorePin, INPUT);
  lcd.begin(16, 2); // Inizializzo LCD 16 caratteri per 2 linee, SERIALE:
  lcd.clear();
}
void loop()
{
  somma = 0;
  for (int cicli = 0; cicli < 10; cicli++)
  {
    delay(2);
    segmenti = analogRead (sensorePin);
    somma = somma + segmenti;              // attenzione a non superare il limite di 32767;
  }
  segmenti = somma / 10; 
  if (segmenti < luminosita) ciclopwm = ciclopwm + 1;
  else ciclopwm = ciclopwm - 1;
  if (ciclopwm > 255) ciclopwm = 255;
  if (ciclopwm < 0) ciclopwm = 0;
  if (millis() > timer) ciclopwm = 0;    // timer opzionale: 
  analogWrite(caricoPin, ciclopwm);
  //
  lcd.setCursor(0, 0);
  lcd.print("dutycycle:      ");
  lcd.setCursor(11, 0);
  lcd.print(ciclopwm);
  lcd.setCursor(0, 1);
  lcd.print("segmenti.:      ");      // disattivare a regime:
  lcd.setCursor(11, 1);               // disattivare a regime:
  lcd.print(segmenti);                // disattivare a regime:
  //**** da utilizzare a regime **********************
  //  lcd.print("volt.....:     ");
  //  lcd.setCursor(11, 1);
  //  lcd.print(12*ciclopwm/254);
  //**************************************************
}
//FINE by SoftPlus per blog Mauro Alfieri































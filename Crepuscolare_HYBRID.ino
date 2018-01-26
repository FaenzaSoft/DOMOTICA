
/* Il crepuscolare HYBRID consente di attivare lampade anche di giorno, IN AUTOMATICO, per supplire
   a cadute di luminosità dovuta a passaggi nuvolosi, giornata grigia, crepuscolo.
   Lo fa attivando le lampade al 100% della potenza o in soluzioni intermedie, come un rubinetto.
   Ovviamente di notte le lampade sono attivate a potenza piena.
   Le lampade a led da utilizzare, una o più in parallelo, debbono essere a 12 volt per corrente continua;
   Il software consente di valutare la luminosità e conseguentemente di attivare il relè fai da te PWM:
   Il display (a regime) evidenzia il duty cycle e la tensione di uscita. Si può anche omettere:
   In fase di collaudo del software il display evidenzia sulla seconda riga i segmenti percepiti (da 0 a 1024):
   Usare Arduino UNO e display SERIALE 2x16:
   Video YouTube: https://youtu.be/301RBk-4-e8
   FOTORESISTENZA:
   Utilizzare una fotoresistenza da 20k collegata con un capo al positivo dei 5 volt:
   L'altro capo va all'ingresso A0; tra A0 e GND va collocata una resistenza da 15k:
   I PROBLEMI
   che si pongono riguardano principalmente la collocazione della fotoresistenza:
   1) la fotoresistenza va tenuta lontano dalla lampada che si vuole dimmare:
   2) va evitato l'effetto pendolo per cui quando si accende la lampada la fotoresistenza la spegne:
   3) evitare che una persona in movimento nella stanza condizioni la luminosità che arriva alla fotoresistenza:
   RISULTATI:
   Il software proposto rappresenta una soluzione di equilibrio: spegne la lampada con molta luminosità:
   e l'accende in presenza di scarsa luminosità, attivandola al massimo della luminosità, o con soluzioni intermedie stabili:
   E' previsto un lieve ritardo nell'intervento, e rilevazione dati plurime, che evitano lampeggi sconvenienti.
   P.S.: si può utilizzare un relè fai da te senza il 7812 e con dimensioni ridotte, senza display e con dissipatore mignon:
         da abbinare ad un arduino nano, ciò può consentire la collocazione del tutto in uno spazio molto ridotto.
*/
#include <Arduino.h>                   //inclusione della libreria base:
#include <Wire.h>                      // libreria necessaria pere diplay seriale:
#include <LiquidCrystal_I2C.h>         //inclusione della libreria per display seriale:
LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);    // impostazione display seriale NewLiquidCristal:
//
int caricoPin = 9;             // carico analogico PWM pin 9:
int sensorePin = A0;           // sensore fotoresistenza;
int ciclopwm1 = 0;
int ciclopwm2 = 0;
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
    delay(200);
    segmenti = analogRead (sensorePin);
    somma = somma + segmenti;              // attenzione a non superare il limite di 32767;
  }
  segmenti = somma / 10;
  ciclopwm1 = 255 - (segmenti  / 4);      // i segmenti possono arrivare a 1024:
  ciclopwm2 = ciclopwm1;
  if (ciclopwm1 < 50) ciclopwm2 = ciclopwm1 - (ciclopwm1 * 1.5);
  if (ciclopwm1 > 100) ciclopwm2 = ciclopwm1 + ((ciclopwm1 - 100) * 1.7);
  if (ciclopwm2 > 200) ciclopwm2 = 255;
  if (ciclopwm2 < 0) ciclopwm2 = 0;
  analogWrite(caricoPin, ciclopwm2);
  //
  lcd.setCursor(0, 0);
  lcd.print("dutycycle:      ");
  lcd.setCursor(11, 0);
  lcd.print(ciclopwm2);
  lcd.setCursor(0, 1);
  lcd.print("segmenti.:      ");      // disattivare a regime:
  lcd.setCursor(11, 1);               // disattivare a regime:
  lcd.print(segmenti);                // disattivare a regime:
  //**** da utilizzare a regime **********************
  //  lcd.print("volt.....:     ");
  //  lcd.setCursor(11, 1);
  //  lcd.print(12*ciclopwm2/254);
  //**************************************************
}
//FINE by SoftPlus per blog Mauro Alfieri































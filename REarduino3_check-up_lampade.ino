
/* REarduino3, check-up lampade a bassa tensione (alogene e LED), vedi video:
   Ver. 0.0 del 11/02/2018
   Video YouTube: https://youtu.be/IQgoPdrmbU4
   //
   Il software consente di testare l'assorbimento delle lampade LED ai diversi livelli di eccitazione PWM:
   Lo schema elettrico somiglia molto a quelli dei progetti Sibilla e DRIVE-IN.
   Il display videnzia il duty cycle, la tensione di uscita, i milliamper di assorbimento e i watt in unica videata.
   Vengono usati 3 sensori/divisori di tensione (2200 ohm verso massa e 3300 sul positivo da testare):
   Le resistenze di shunt sono 2 ceramiche a cubetto da 1 ohm 5 watt, in parallelo.
   Il Mosfet è il IRF3205. Optoisolatore 4N25.
   FOTORESISTENZA:
   Utilizzare una normale fotoresistenza da 20k collegata con un capo al positivo dei 5 volt:
   L'altro capo va all'ingresso A zero di arduino; tra A0 e GND va collocata una resistenza da 15k:
   SLIDER
   Per la prova delle lampade è più opportuno utilizzare uno slider da 10K con i capi collegati a massa uno e ai 5 volt positivi:
   di arduino l'altro. Il centrale va all'ingresso analogico A zero di arduino.
*/
#include <Arduino.h>                   //inclusione della libreria base:
#include <Wire.h>                      // libreria necessaria pere diplay seriale:
#include <LiquidCrystal_I2C.h>         //inclusione della libreria per display seriale:
// LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);    // impostazione display seriale NewLiquidCristal:
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);    // indirizzo LCD Philips PCF8574AT:
//
// int fondo_scala = 281;      // taratura fine scala volt, valido per 24 VOLT con divisore 1000 + 4700 ohm:
int fondo_scala = 125;         // taratura fine scala volt, valido per 12 VOLT con divisore 2200 + 3300 ohm:
//
int caricoPin = 9;             // carico analogico PWM pin 9:
int sensorePin = A0;           // sensore slider;
int luminosita = 700;          // valore da modificare, regola la luminosità dell'ambiente che si vuole ottenere:
unsigned long timer = 43200000;   // (60000*60*12 = quarantatremilioniduentemila = 12 ore) opzionale:
unsigned long volt_IN = A3;             // ingresso analogico A3 sensore volt FV:
unsigned long volt_SHUNT = A4;          // ingresso analogico A4 sensore dopo le 3 resistenze da 1 ohm:
unsigned long volt_DRAIN = A5;          // ingresso analogico A5 sensore volt batterie:
unsigned long volt_OUT = 0;
unsigned long somma_IN = 0;
unsigned long somma_SHUNT = 0;
unsigned long somma_DRAIN = 0;
long mAmper = 0;
int watt = 0;
int ciclopwm = 0;
int volt_ingresso = 12;
int segmenti = 0;
int somma_luce = 0;
int conta1 = 0;
int conta2 = 0;
int conta3 = 0;
int lento = 0;
int correzione = 0;
//
void setup()
{
  delay(2000);
  pinMode(caricoPin, OUTPUT);
  pinMode(A0, INPUT);     // ingresso sensore slider modifica duty cycle:
  pinMode(A1, INPUT);     // ingresso analogico A1 sensore volt IN:
  pinMode(A2, INPUT);     // ingresso analogico A2 sensore volt SHUNT:
  pinMode(A3, INPUT);     // ingresso analogico A3 sensore volt MOSFET:
  // i pin A4 e A5 sono dedicati al display IC2
  lcd.begin(16, 2); // Inizializzo LCD 16 caratteri per 2 linee, SERIALE:
  //  lcd.clear();
  lcd.begin(16, 2);
  lcd.print("    SoftPlus    ");
  lcd.setCursor(0, 1);
  lcd.print("  Consumo Zero  ");
  delay(2000);
  lcd.setCursor(0, 0);
  lcd.print("PWM      V      ");
  lcd.setCursor(0, 1);
  lcd.print("mA       W      ");
  //
}
void loop()
{
  // step 1 ************************ VERIFICA TENSIONI ****************************************
  //
  somma_IN = 0;
  somma_SHUNT = 0;
  somma_DRAIN = 0;
  somma_luce = 0;
  //
  for (int cicli = 0; cicli < 500; cicli++)    // analogRead() restituisce i segmenti (da 0 a 1023):
  {
    volt_IN = analogRead(A1);
    somma_IN = (somma_IN + volt_IN);
    //
    //  delay(1);
    volt_SHUNT = analogRead(A2);
    somma_SHUNT = (somma_SHUNT + volt_SHUNT);
    //
    //  delay(1);
    volt_DRAIN = analogRead(A3);
    somma_DRAIN = (somma_DRAIN + volt_DRAIN);
    //
    delay(2);
  }
  volt_IN = (somma_IN / 50);  // perchè è stata fatta la somma di 400 cicli:    // per avere 3 decimali
  volt_SHUNT = (somma_SHUNT / 50);  // perchè è stata fatta la somma di 400 cicli:   // per avere 3 decimali:
  volt_DRAIN = (somma_DRAIN / 50);  // perchè è stata fatta la somma di 400 cicli:   // per avere 3 decimali:
  volt_IN = (volt_IN * fondo_scala / 100);              // rettifica valori in relazione a divisore di tensione:
  volt_SHUNT = (volt_SHUNT * fondo_scala / 100);
  volt_DRAIN = (volt_DRAIN * fondo_scala / 100);
  volt_OUT = (volt_SHUNT - volt_DRAIN);
  //
  //  if (volt_OUT > 1200) correzione = correzione +1;     // limitatore a 12 volt:
  //  else correzione = 0;
  //
  // step 2 ************************ CALCOLO VOLT SU LAMPADA ***************************************
  if (volt_IN > volt_SHUNT)   // calcolo con volt con 3 decimali, cioè millivolt;
  {
    // utilizzare la versione 4/2 per l'hardware con resistenze da 0,5 ohm e quella 8/2 per resistenze da 0,25 ohm:
    // mAmper = (volt_IN - volt_SHUNT) * (2 / 2);     //perchè si utilizza una resistenze da 1 ohm:
    mAmper = (volt_IN - volt_SHUNT) * (4 / 2);     //perchè ci sono 2 resistenze da 1 ohm serie/parallelo, cioè 0,5 ohm di carico:
    // mAmper = (volt_IN - volt_SHUNT) * (8 / 2);     //perchè abbiamo 4 resistenze da 1 ohm in parallelo, cioè 0,25 ohm:
  }
  else
  {
    mAmper = 0;
  }
  volt_OUT = volt_OUT / 10;        // così ci sono solo 2 decimali;
  volt_SHUNT = volt_SHUNT / 10;    // così ci sono solo 2 decimali;
  //
  // step 3 ************************ GESTIONE PWM ***************************************************
  segmenti = analogRead (A0);        // verifica i segmenti della fotoresistenza o dello slider:
  // if (volt_OUT > 1200) ciclopwm = (segmenti / 4) - correzione;         // limitarore 12 volt:
  ciclopwm = (segmenti / 4);          // perchè i segmenti sono 1024 e il pwm 255:
  if (ciclopwm > 255) ciclopwm = 255;
  if (ciclopwm < 0) ciclopwm = 0;
  analogWrite(caricoPin, ciclopwm);        // pin D9:
  //
  // step 4 ************************++ VISUALIZZAZIONE DATI *****************************************
  lcd.setCursor(4, 0);
  lcd.print("    ");
  lcd.setCursor(4, 0);
  lcd.print(ciclopwm);
  lcd.setCursor(11, 0);
  lcd.print("      ");
  if (volt_OUT > 999)
  {
    lcd.setCursor(12, 0);
    lcd.print(volt_OUT);
    lcd.setCursor(13, 0);
    lcd.print(",");
    lcd.setCursor(11, 0);
    lcd.print(volt_OUT / 100);
  }
  if (volt_OUT < 1000)
  {
    if (volt_OUT < 100)
    {
      lcd.setCursor(13, 0);
      lcd.print(volt_OUT);
      lcd.setCursor(12, 0);
      lcd.print(",");
      lcd.setCursor(11, 0);
      lcd.print(volt_OUT / 100);
    }
    else
    {
      lcd.setCursor(12, 0);
      lcd.print(volt_OUT);
      lcd.setCursor(12, 0);
      lcd.print(",");
      lcd.setCursor(11, 0);
      lcd.print(volt_OUT / 100);
    }
  }
  lcd.setCursor(3, 1);
  lcd.print("     ");
  lcd.setCursor(3, 1);
  lcd.print(mAmper);
  // watt = ampermedia * voltshunt:
  watt = mAmper * volt_SHUNT / 1000;   // watt con 2 decimali:
  lcd.setCursor(11, 1);
  lcd.print("     ");
  if (watt < 1000)
  {
    if (watt < 100)
    {
      lcd.setCursor(13, 1);
      lcd.print(watt);
      lcd.setCursor(12, 1);
      lcd.print(",");
      lcd.setCursor(11, 1);
      lcd.print(watt / 100);
    }
    else
    {
      lcd.setCursor(12, 1);
      lcd.print(watt);
      lcd.setCursor(12, 1);
      lcd.print(",");
      lcd.setCursor(11, 1);
      lcd.print(watt / 100);
    }
  }
  if (watt > 999)
  {
    lcd.setCursor(12, 1);
    lcd.print(watt);
    lcd.setCursor(13, 1);
    lcd.print(",");
    lcd.setCursor(11, 1);
    lcd.print(watt / 100);
  }
  //
}
//FINE by SoftPlus Consumo Zero per blog Mauro Alfieri































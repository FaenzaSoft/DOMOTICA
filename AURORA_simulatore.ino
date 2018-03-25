/* PROGRAMMA AURORA simulatore per contatore digitale e RELE' di stop stacco energia (gestione carichi prioritari)
  Utilizzare centralina AURORA
  Versione per display 2x16 SERIALE (A4 e A5)
  Allaccio contatore digitali a A6 e A7
  Uscite relè: D9, D10 e D11
  Uscite PWM: D3, D5 e D6
  Alimentazione: 12 volt DC stabilizzati
  Autore SoftPlus Consumo Zero - email: luigi.marchi.faenza@gmail.com
  Versione software del 25/03/2018
  Video YouTube:   https://youtu.be/nXvUxrs9FpQ
  //
  Le notazioni che seguono NON valgono per il software di simulazione, che non prevede allacci di sorta.
  //
  ATTENZIONE, IMPORTANTE:
  Arduino funziona a 5 volt e non ha problemi di sorta, circa il rischio di scariche elettriche.
  Circa l'alimentatore da rete è OBBLIGATORIO utilizzare un caricabatteria da telefonino (a norma) con uscita 12 volt DC stabilizzati.
  Per operare sulle parti hardware di contorno occorre avere le competenze ed esperienze consolidate.
  Occorre essere ESPERTI e CON PATENTINO per gli interventi SUL QUADRO ELETTRICO e in apparecchiature con tensioni di rete (230 AC).
  E' pertanto necessario affidarsi a personale qualificato.
  Il produttore del software declina ogni responsabilità per danni elettrici.
  Quanto ai danni derivante da malfunzionamento del software, il produttore del software declina ogni responsabilità circa:
  1) danni derivanti da utilizzo di componentistica di contorno;
  2) danni derivanti da modifica del software originario;
  3) danni derivanti da scarsa esperienza e scarsa capacità professionale dell'installatore.
  //
  //
  IMPORTANTE: il contatore digitale va collocato sulla linea dei consumi domestici complessivi (consumi base + consumi sulle linee
  gestite da relè, cioè sui consumi prelevati da rete Enel).
  //
  IMPORTANTE: i sensori digitali producono degli impulsi la cui distanza, l'uno dall'altro, è inversamente proporzionale alla potenza:
  ciò comporta una certa lentezza nella valutazione dei piccoli carichi,la contropartita è una misurazione "fiscale" della massima
  precisione, come i contatori Enel; i sensori utilizzati (reperibili presso PARSIC ITALIA - Cervia) richiedono 3200 impulsi ora per un KWora.
  Nella gestione relè di carico la velocità degli impulsi è rilevante e si possono utilizzare anche contatori da 2000 e 1000 impulsi.
  //
  Basetta di interfaccia, tra contatore e Arduino: 1 resistenze da 39 Kohm lato massa (o maggiore, dipende) e 1 da 100/1000 ohm lato 5 volt.
  Relativamente al numero di impulsi, va utilizzato il software relativo, oppure modificare le istruzioni che fanno riferimento
  agli impulsi.  Vedi anche le altre versioni di software ARCHIMEDE.
  Questo software prevede anche un simulatore, cioè la simulazione di un carico base che varia da zero a 3 Kw per simulare la gestione
  ON e OFF dei relè. Serve inoltre per tarare i trimmer, in base ai carichi nominali da allacciare.
  Queste parti di software vanno annullate nella versione definitiva, collocata nel quadro elettrico. Cioè a regime.
  //
*/
// modificare le libreria LiquidCrystal se si usano integrati di diversa amrca:
#include <Arduino.h> //inclusione della libreria base:
// includere altre librerie:
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
// LiquidCrystal_I2C lcd(0x20, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);    // LCM1602 IIC A0 A1 A2:
// LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);    //POSITIVE=accensione retroilluminazione:
LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);    // indirizzo LCD Philips PCF8574AT:
//
//
int pin_consumi = A0;       // pin dove arrivano gli impulsi dei consumi:
int conta1 = 0;
int conta2 = 0;     // tempo per fare le regolazioni iniziali:
int conta3 = 0;      // serve per la simulazione:
//
//
//
// unsigned long val_cons = 0;
int val_cons = 0;
unsigned long potenza_watt_consumi = 0;      // non modificare unsigned long:
double vecchio_millis_consumi = 0;
double corrente_millis_consumi = 0;
unsigned long tempo_trascorso_consumi = 0;         // non modificare unsigned long anzichè int:
double tempo_adesso_consumi = 0;
unsigned long contawatt = 0;
int volt_partitore_consumi = 0;
double vecchio_millis_rele = 0;
double corrente_millis_rele = 0;
int impulsi_consumi = 0;
unsigned long impulsi_consumi_nuovo = 0;
unsigned long impulsi_consumi_vecchio = 0;
int linea1 = 0;
int linea2 = 0;
int linea3 = 0;
int potenza_linea1 = 0;
int potenza_linea2 = 0;
int potenza_linea3 = 0;
int potenza_linea4 = 0;
int surplus = 0;
int surplus_negativo = 0;
int compensazione = 2;             // moltiplicatore dei  segmenti trasformati in watt:
int potenza_watt_iniziali = 0;
int contarele = 0;
int variazione = 100;
//
// **********************************************************************************************************
int impulsi_kwh = 2000;                          // dato da modificare in base agli impulsi del contatore:
// **********************************************************************************************************
int watt_energia_contrattata = 3000;             // dato da modificare in base al contratto:
// int watt_energia_contrattata = 6600;
// **********************************************************************************************************
//
void setup()
{
  // pin analogici:
  // A0 inutilizzato:
  pinMode(A1, INPUT);             // trimmer 1:
  pinMode(A2, INPUT);             // trimmer 2:
  pinMode(A3, INPUT);             // trimmer 3:
  // A4 display I2C;
  // A5 display I2C;
  pinMode(A6, INPUT);             // pin sensore analogico consumi:
  pinMode(A7, INPUT);             // pin sensore analogico produzione FV:
  // pin digitali:
  pinMode(9, OUTPUT);            // uscita relè linea1;
  pinMode(10, OUTPUT);            // uscita relè linea2;
  pinMode(11, OUTPUT);           // uscita relè linea3;
  //
  lcd.begin(16, 2); // Inizializzo LCD 16 caratteri per 2 linee, SERIALE:
  lcd.print("    SoftPlus    ");
  lcd.setCursor(0, 1);
  lcd.print("  Consumo Zero  ");
  delay(3000);
  lcd.setCursor(0, 1);
  lcd.print("                ");
  //
  //  lcd.begin(16,2);
  //
  impulsi_kwh = impulsi_kwh / 100; //per semplificare i calcoli:
}
//
// Inizia ciclo
void loop() {
  //
  // se è premuto il tasto
  // digitalWrite(out_rst, LOW); //porta l'uscita a livello basso per resettare la scheda:
  //

  // step 1 lettura dei dati analogici:
  //
  tempo_adesso_consumi = millis();
  if (tempo_adesso_consumi > vecchio_millis_consumi + 100)   // bisogna che siano passati almeno 100 millisecondi dall'ultima lettura:
    // (mettere 100 per misure fino a 6 Kw e 200 per misure fino a 3 kw).
  {
    // legge lo stato e percepisce l'impulso sul pin deputato a rilevare i consumi:
    volt_partitore_consumi = analogRead(A6);
    if (volt_partitore_consumi > 200)       // la tensione deve essere maggiore di 1 volt:
    {
      corrente_millis_consumi = millis();
      tempo_trascorso_consumi = corrente_millis_consumi - vecchio_millis_consumi;
      vecchio_millis_consumi = corrente_millis_consumi;
      // calcolo produzione media, in watt, di tempo in tempo:
      // 3200 impulsi ora per 1 Kwh, cioè 1.000 wattora:
      // per ogni 1000 wattora serve 1 impulso ogni 3600/3200 secondi cioè ogni 1,125 secondi:
      // se gli impulsi arrivano ogni 11,25 secondi abbiamo 100 wattora e così via:
      // poi bisogna tradurre in millesimi, laddove 1,125 secondi sono 1.125 millesimi di secondo:
      // potenza_watt_consumi = 1000000 / tempo_trascorso_consumi * 36 / 64;        //valido per contatore da 6400 impulsi
      // potenza_watt_consumi = 1000000 / tempo_trascorso_consumi * 36 / 32;        //valido per contatore da 3200 impulsi
      // potenza_watt_consumi = 1000000 / tempo_trascorso_consumi * 36 / 20;        //valido per contatore da 2000 impulsi
      potenza_watt_consumi = 1000000 / tempo_trascorso_consumi * 36 / impulsi_kwh;     // valido per impostazione iniziale impulsi:
      val_cons = potenza_watt_consumi;
      impulsi_consumi = impulsi_consumi + 1;
    }
    //
  }
  // simulazione ********************************
  if (conta3 > 10000)
  {
    conta3 = 0;
    potenza_watt_iniziali = potenza_watt_iniziali + variazione;
    if (potenza_watt_iniziali < 0) variazione = 100;
    if (potenza_watt_iniziali > 3000) variazione = -100;
  }
  // potenza_watt_iniziali = 200;  // cioè 3100 di surplus:
  // potenza_watt_iniziali = 500;  // cioè 2800 di surplus:
  // potenza_watt_iniziali = 700;  // cioè 2600 di surplus:
  // potenza_watt_iniziali = 1000;  // cioè 2300 di surplus:
  // potenza_watt_iniziali = 1500;  // cioè 1800 di surplus:
  // potenza_watt_iniziali = 2000;  // cioè 1300 di surplus:
  // potenza_watt_iniziali = 2900;  // cioè 1300 di surplus:
  //
  val_cons = potenza_watt_iniziali;
  if (linea1 == 1) val_cons = val_cons + potenza_linea1;
  if (linea2 == 1) val_cons = val_cons + potenza_linea2;
  if (linea3 == 1) val_cons = val_cons + potenza_linea3;
  impulsi_consumi = impulsi_consumi + 1;
  // fine simulazione **************************
  //
  //
  // step 2 modifica uscite ON/OFF relè:
  // corrente_millis_rele = millis();
  // if (corrente_millis_rele > vecchio_millis_rele + 3000)
  //
  if (linea1 == 0 && linea2 == 0 && linea3 == 0 ) contarele = 0;
  if (linea1 == 1 && linea2 == 0 && linea3 == 0 ) contarele = 1;
  if (linea1 == 1 && linea2 == 1 && linea3 == 0 ) contarele = 2;
  if (linea1 == 1 && linea2 == 1 && linea3 == 1 ) contarele = 3;
  //
  if ( impulsi_consumi > 2)  // perchè bisogna che il contatore consumi abbia percepito la modifica nei consumi:
  {
    //
    if (watt_energia_contrattata > val_cons)
    {
      surplus = watt_energia_contrattata - val_cons;
      switch (contarele)
      {
        case 0:
          //linea1 = 0; linea2 = 0; linea3 = 0;
          if (surplus > potenza_linea1) linea1 = 1;
          break;
        case 1:
          // linea1 = 1; linea2 = 0; linea3 = 0;
          if (surplus > potenza_linea2) linea2 = 1;
          break;
        case 2:
          // linea1 = 1; linea2 = 1; linea3 = 0;
          if (surplus > potenza_linea3) linea3 = 1;
          break;
        case 3:
          // linea1 = 1; linea2 = 1; linea3 = 1:
          break;
      }
    }
    else   //surplus negativo, dobbiamo staccare qualcosa:
    {
      // surplus_negativo = potenza_watt_consumi - watt_energia_contrattata;
      switch (contarele)
      {
        case 3:
          // linea1 = 1; linea2 = 1; linea3 = 1:
          linea3 = 0; break;
        case 2:
          // linea1 = 1; linea2 = 1; linea3 = 0;
          linea2 = 0; break;
        case 1:
          // linea1 = 1; linea2 = 0; linea3 = 0;
          linea1 = 0; break;
        case 0:
          linea1 = 0; linea2 = 0; linea3 = 0;
          break;
      }
    }
    impulsi_consumi = 0;
    if (linea1 == 0 && linea2 == 0 && linea3 == 0 ) contarele = 0;
    if (linea1 == 1 && linea2 == 0 && linea3 == 0 ) contarele = 1;
    if (linea1 == 1 && linea2 == 1 && linea3 == 0 ) contarele = 2;
    if (linea1 == 1 && linea2 == 1 && linea3 == 1 ) contarele = 3;
    if (linea1 == 1) digitalWrite(9, HIGH);
    if (linea2 == 1) digitalWrite(10, HIGH);
    if (linea3 == 1) digitalWrite(11, HIGH);
    if (linea1 == 0) digitalWrite(9, LOW);
    if (linea2 == 0) digitalWrite(10, LOW);
    if (linea3 == 0) digitalWrite(11, LOW);
    //
  }
  vecchio_millis_rele = corrente_millis_rele;
  //
  //
  // step 3 visualizzazione dati:
  if (conta1 > 300)    // va bene:
    //if (conta1 > 30)
  {
    //    contawatt = impulsi_consumi*1000/3200;   // perchè abbiamo 3200 impulsi per Kwh:
    lcd.setCursor(0, 0);
    lcd.print("W_consumi:      ");
    lcd.setCursor(11, 0);
    lcd.print(val_cons);
    lcd.setCursor(0, 1);
    lcd.print("                ");
    lcd.setCursor(0, 1);
    if (linea1 == 1) lcd.print("1ON");
    else lcd.print("1OFF");
    lcd.setCursor(6, 1);
    if (linea2 == 1) lcd.print("2ON");
    else lcd.print("2OFF");
    lcd.setCursor(12, 1);
    if (linea3 == 1) lcd.print("3ON");
    else lcd.print("3OFF");
    conta1 = 0;
    if (conta2 > 300 ) conta2 = 300;
    //
  }
  //
  // step 4 lettura dei dati dei trimmer di regolazione potenza nominale:
  //
  if (conta2 < 300)
  {
    if (conta2 > 0 && conta2 < 100)
    {
      lcd.setCursor(0, 0);
      lcd.print("W_linea1:       ");
      lcd.setCursor(11, 0);
      potenza_linea1 = analogRead(A1) * compensazione;
      lcd.print(potenza_linea1);
      delay(200);
    }
    if (conta2 > 100 && conta2 < 200)
    {
      //delay(2000);
      lcd.setCursor(0, 0);
      lcd.print("W_linea2:       ");
      lcd.setCursor(11, 0);
      potenza_linea2 = analogRead(A2) * compensazione;
      lcd.print(potenza_linea2);
      delay(200);
    }
    if (conta2 > 200 && conta2 < 300)
    {
      lcd.setCursor(0, 0);
      lcd.print("W_linea3:       ");
      lcd.setCursor(11, 0);
      potenza_linea3 = analogRead(A3) * compensazione;
      lcd.print(potenza_linea3);
      delay(200);
    }
  }
  //}
  conta1 = conta1 + 1;
  conta2 = conta2 + 1;
  conta3 = conta3 + 1;
  //
}
// FINE listato by SoftPlus Consumo Zero - Faenza.
































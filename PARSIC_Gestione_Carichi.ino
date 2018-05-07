/* PROGRAMMA AURORA Salva Energia PARSIC ITALIA - NANO 1X16 per contatore digitale e RELE' di stop stacco energia
  Utilizzo centralina AURORA con ingresso A6 (consumi) e 1 contatore digitale
  N.B.: inserire una resistenza da 47k sul morsetto A zero per consentire la misura dell'energia di contratto:
  Ingresso contatore consumi SO+ e SO- connettore A6
  Uscite relè: D9, D10 e D11
  Uscite PWM: D3, D5 e D6
  Versione per display 1x16 SERIALE (A4 e A5)
  N 3 trimmer di regolazione potenza allaccio relè + 1 trimmer regolazione potenza contrattata
  Autore SoftPlus Consumo Zero - email: luigi.marchi.faenza@gmail.com
  Versione software del 28/04/2018
  Video YouTube: https://youtu.be/nXvUxrs9FpQ
  Implementazione carichi: vedi foglio Simulazione Carichi Excel in  https://github.com/FaenzaSoft/Domotica
  //
  //
  ATTENZIONE, IMPORTANTE:
  Arduino funziona a 5 volt e non ha problemi di sorta, circa il rischio di scariche elettriche.
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
  ciò comporta una certa lentezza nella valutazione dei piccoli carichi, la contropartita è una misurazione "fiscale" della massima
  precisione, come i contatori Enel; i sensori utilizzati (reperibili presso PARSIC ITALIA - Cervia) richiedono 3200 impulsi ora per un KWora.
  Nella gestione relè di carico la velocità degli impulsi è rilevante e si possono utilizzare anche contatori da 2000 e 1000 impulsi.
  //
  Relativamente al numero di impulsi, va utilizzato il software relativo, oppure modificare le istruzioni che fanno riferimento
  agli impulsi.  Vedi anche le altre versioni di software ARCHIMEDE.
  Questo software prevede anche un simulatore, cioè la simulazione di un carico base che varia da zero a 3 Kw per simulare la gestione
  ON e OFF dei relè. Serve inoltre per tarare i trimmer, in base ai carichi nominali da allacciare.
  Queste parti di software vanno annullate nella versione definitiva, collocata nel quadro elettrico. Cioè a regime.
  //
*/
// Modificare le librerie LiquidCrystal in relazione al display utilizzato e all'integrato I2C
//
#include <Arduino.h> //inclusione della libreria base:
// includere altre librerie:
#include <Wire.h>
//
// #include <LiquidCrystal_I2C.h>    // si può utilizzare in alternativa alla Parsic V40;
//
#include <LiquidCrystal_I2C_Parsic_V40.h>
//
LiquidCrystal_I2C lcd(0x20, 4, 5, 6, 0, 1, 2, 3, 7, POSITIVE);        // per V34:
//
// LiquidCrystal_I2C lcd(0x20, 16,2);    // LCM1602 IIC A0 A1 A2:
// LiquidCrystal_I2C lcd(0x20, 2, 1, 0, 4, 5, 6, 7);    // LCM1602 IIC A0 A1 A2:
// LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);    //POSITIVE=accensione retroilluminazione:
// LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);    // indirizzo LCD Philips PCF8574AT:
//
//
int pin_consumi = A6;       // pin dove arrivano gli impulsi dei consumi:
int conta1 = 0;
int conta2 = 0;     // tempo per fare le regolazioni iniziali:
int conta3 = 0;
int conta4 = 0;
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
// int potenza_watt_iniziali = 0;
int contarele_on = 0;
int variazione = 100;
int totale = 0;
//
// **********************************************************************************************************
int impulsi_kwh = 3200;                          // dato da modificare in base agli impulsi del contatore:
// **********************************************************************************************************
int watt_energia_contrattata = 0;        // va impostata con il trimmer accanto all'ingresso A zero, fino a 7 kw circa:
// N.B.: occorre cortocircuitare il connettore A zero con una resistenza da 47k o con allaccio spinotto:
// **********************************************************************************************************
//
void setup()
{
  // pin analogici:
  pinMode(A0, INPUT);             // trimmer zero:
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
  //
  lcd.begin(8, 2); // Inizializzo LCD 8 caratteri per 2 linee, SERIALE:

  lcd.setCursor(0, 0);
  lcd.print("V34 PARS");
  lcd.setCursor(0, 1);
  lcd.print("ICITALIA");
  // tone(2,1000,2000);
  delay(3000);
  lcd.setCursor(0, 0);
  lcd.print("Gestione");
  lcd.setCursor(0, 1);
  lcd.print(" CARICHI");
  // tone(2,1000,2000);
  delay(3000);
  lcd.setCursor(0, 0);
  lcd.print("    Soft");
  lcd.setCursor(0, 1);
  lcd.print("Plus    ");
  delay(3000);
  lcd.setCursor(0, 0);
  lcd.print("Impulsi ");
  lcd.setCursor(0, 1);
  lcd.print("Kwh     ");
  lcd.setCursor(4, 1);
  lcd.print(impulsi_kwh);
  delay(3000);
  //
  //
  impulsi_kwh = impulsi_kwh / 100; //per semplificare i calcoli:
}
//
// Inizia ciclo
void loop() {
  //
  // se è premuto il tasto reset
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
  //  // *************************************** SIMULAZIONE **********************************
  // if (conta4 > 10000)
  // {
  //  impulsi_consumi = 1;
  //  conta4 = 0;
  //  val_cons = val_cons + variazione;
  //  if (val_cons < 0) variazione = 100;
  //  if (val_cons > 3000) variazione = -100;
  // }
  //  // *************************************** FINE SIMULAZIONE *****************************
  if (linea1 == 0 && linea2 == 0 && linea3 == 0 ) contarele_on = 0;
  if (linea1 == 1 && linea2 == 0 && linea3 == 0 ) contarele_on = 1;
  if (linea1 == 1 && linea2 == 1 && linea3 == 0 ) contarele_on = 2;
  if (linea1 == 1 && linea2 == 1 && linea3 == 1 ) contarele_on = 3;
  //
  if ( impulsi_consumi > 1)  // perchè bisogna che il contatore consumi abbia percepito la modifica nei consumi:
  {
    //
    if (watt_energia_contrattata > val_cons)
    {
      surplus = watt_energia_contrattata - val_cons;
      switch (contarele_on)
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
      switch (contarele_on)
      {
        case 3:
          // linea1 = 1; linea2 = 1; linea3 = 1:
          linea3 = 0;
          tone (13, 1000, 2000);
          break;
        case 2:
          // linea1 = 1; linea2 = 1; linea3 = 0;
          linea2 = 0;
          tone (13, 1000, 2000);
          break;
        case 1:
          // linea1 = 1; linea2 = 0; linea3 = 0;
          linea1 = 0;
          tone (13, 1000, 2000);
          break;
        case 0:
          linea1 = 0; linea2 = 0; linea3 = 0;
          break;
      }
    }
    impulsi_consumi = 0;
    if (linea1 == 0 && linea2 == 0 && linea3 == 0 ) contarele_on = 0;
    if (linea1 == 1 && linea2 == 0 && linea3 == 0 ) contarele_on = 1;
    if (linea1 == 1 && linea2 == 1 && linea3 == 0 ) contarele_on = 2;
    if (linea1 == 1 && linea2 == 1 && linea3 == 1 ) contarele_on = 3;
    // ****************************************** INVERSIONE RELE' **************************************
    // if (linea1 == 1) digitalWrite(9, HIGH);
    // if (linea2 == 1) digitalWrite(10, HIGH);
    // if (linea3 == 1) digitalWrite(11, HIGH);
    // if (linea1 == 0) digitalWrite(9, LOW);
    // if (linea2 == 0) digitalWrite(10, LOW);
    // if (linea3 == 0) digitalWrite(11, LOW);
    // *********************** modofica ALLACCIARE C + NC ****************************************************
    if (linea1 == 1) digitalWrite(9, LOW);
    if (linea2 == 1) digitalWrite(10, LOW);
    if (linea3 == 1) digitalWrite(11, LOW);
    if (linea1 == 0) digitalWrite(9, HIGH );
    if (linea2 == 0) digitalWrite(10, HIGH);
    if (linea3 == 0) digitalWrite(11, HIGH);
    // **************************************** FINE MODIFICA *******************************************
    //
  }
  // vecchio_millis_rele = corrente_millis_rele;
  //
  //
  // step 3 visualizzazione dati:
  //
  // step 3 visualizzazione dati consumi e stato relè:
  if (conta1 > 400)    // va bene per gestione carichi prioritari:
  {
    if (conta3 > 10000)        // durata visualizzazione circa 2 secondi, alternata con quella della riga ON/OFF:
    {
      lcd.setCursor(0, 1);
      lcd.print("i:       ");
      lcd.setCursor(3, 1);
      lcd.print(val_cons);
      lcd.setCursor(0, 0);
      lcd.print("W_consum");
      conta3 = 0;
      //conta4 = conta4 + 10000;
    }
    if (conta4 > 20000)        // durata visualizzazione circa 2 secondi, alternata all'altra sopra:
    {
      lcd.setCursor(0, 0);
      if (linea1 == 1) lcd.print("1-SI   ");
      else lcd.print("1-NO   ");
      lcd.setCursor(6, 0);
      if (linea2 == 1) lcd.print("2-");
      else lcd.print("2-");
      lcd.setCursor(0, 1);
      if (linea2 == 1)lcd.print("SI  ");
      else lcd.print("NO   ");
      lcd.setCursor(4, 1);
      if (linea3 == 1) lcd.print("3-SI ");
      else lcd.print("3-NO ");
      conta4 = 0;
      conta3 = 0;
    }
    conta1 = 0;
    if (conta2 > 400 ) conta2 = 400;
    if (conta3 > 20000) conta3 = 0;     // quando conta3 supera quota 20000 si riparte da zero:
    //
  }
  //
  // step 4 lettura dei dati dei trimmer di regolazione potenza nominale:
  //
  if (conta2 < 400)
  {
    if (conta2 > 0 && conta2 < 100)
    {
      lcd.setCursor(0, 0);
      lcd.print("W_Enel:         ");
      lcd.setCursor(0, 1);
      lcd.print("         ");
      lcd.setCursor(1, 1);
      totale = 0;
      for (int cicli = 0; cicli < 10; cicli++)
      {
      totale = totale + analogRead(A0);       // deve misurare fino a circa 7 Kw:
      }
      watt_energia_contrattata = totale;
      lcd.print(watt_energia_contrattata);
      delay(200);
    }
    if (conta2 > 100 && conta2 < 200)           // durata 25 secondi circa:
    {
      lcd.setCursor(0, 0);
      lcd.print("W_rele1:        ");
      lcd.setCursor(0, 1);
      lcd.print("        ");
      lcd.setCursor(2, 1);
       totale = 0;
      for (int cicli = 0; cicli < 10; cicli++)
      {
      totale = totale + analogRead(A1);       
      }
      potenza_linea1 = totale /5;
      lcd.print(potenza_linea1);
      delay(200);
    }
    if (conta2 > 200 && conta2 < 300)
    {
      lcd.setCursor(0, 0);
      lcd.print("W_rele2:        ");
      lcd.setCursor(0, 1);
      lcd.print("        ");
      lcd.setCursor(2, 1);
      totale = 0;
      for (int cicli = 0; cicli < 10; cicli++)
      {
      totale = totale + analogRead(A2);       
      }
      potenza_linea2 = totale /5;
       lcd.print(potenza_linea2);
      delay(200);
    }
    if (conta2 > 300 && conta2 < 400)
    {
      lcd.setCursor(0, 0);
      lcd.print("W_rele3:        ");
      lcd.setCursor(0, 1);
      lcd.print("        ");
      lcd.setCursor(2, 1);
       totale = 0;
      for (int cicli = 0; cicli < 10; cicli++)
      {
      totale = totale + analogRead(A3);       
      }
      potenza_linea3 = totale /5;     
       lcd.print(potenza_linea3);
      delay(200);
    }
  }
  //}
  conta1 = conta1 + 1;
  conta2 = conta2 + 1;
  conta3 = conta3 + 1;
  conta4 = conta4 + 1;
  //
}
// FINE listato by SoftPlus Consumo Zero - Faenza.
































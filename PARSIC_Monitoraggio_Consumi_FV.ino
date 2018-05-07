/* PROGRAMMA AURORA Salva Energia PARSIC per contatore digitali Consumi e FV
  Versione per display 1x16 SERIALE (A4 e A5)
  Allaccio contatore digitalei a A6 e A7
  Alimentazione: 12 volt DC stabilizzati
  Autore SoftPlus Consumo Zero - email: luigi.marchi.faenza@gmail.com
  Versione software del 10/04/2018
  Video YouTube:   https://youtu.be/nXvUxrs9FpQ
  //
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
  Basetta di interfaccia, tra contatore e Arduino: 1 resistenze da 39 Kohm lato massa (o maggiore, dipende) e 1 da 100 ohm lato 5 volt.
  Relativamente al numero di impulsi, va utilizzato il software relativo, oppure modificare le istruzioni che fanno riferimento
  agli impulsi.  Vedi anche le altre versioni di software ARCHIMEDE.
  Questo software prevede anche un simulatore, cioè la simulazione di un carico base che varia da zero a 3 Kw per simulare la gestione
  ON e OFF dei relè. Serve inoltre per tarare i trimmer, in base ai carichi nominali da allacciare.
  Queste parti di software vanno annullate nella versione definitiva, collocata nel quadro elettrico. Cioè a regime.
  //
*/
#include <Arduino.h> //inclusione della libreria base:
// includere altre librerie:
#include <Wire.h>
//
//#include <LiquidCrystal_I2C.h>
//
#include <LiquidCrystal_I2C_Parsic_V40.h>      
//
LiquidCrystal_I2C lcd(0x20, 4, 5, 6, 0, 1, 2, 3, 7, POSITIVE);       // per V34:
//
// Viene utilizzata la libreria I2C NewliquidCrystal_1.3.4
// Se si usano altre librerie qualche riga di codice va modificata:
// se si usano integrati diversi si deve modificare l'indirizzo (0x3F, piuttosto che 0x27):
// LiquidCrystal_I2C lcd(0x20, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);
// LiquidCrystal_I2C lcd(0x27, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);    //POSITIVE=accensione retroilluminazione:
// LiquidCrystal_I2C lcd(0x3F, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);    // indirizzo LCD Philips PCF8574AT:
//
//
int pin_consumi = A0;       // pin dove arrivano gli impulsi dei consumi:
int conta1 = 0;
int conta2 = 0;     // tempo per fare le regolazioni iniziali:
int conta3 = 0;      // serve per la simulazione:
int conta4 = 0;
//
//
//
// unsigned long val_cons = 0;
int val_cons = 0;
int val_prod = 0;
unsigned long potenza_watt_consumi = 0;      // non modificare unsigned long:
unsigned long potenza_watt_produzione = 0;      // non modificare unsigned long:
double vecchio_millis_consumi = 0;
double corrente_millis_consumi = 0;
double vecchio_millis_produzione = 0;
double corrente_millis_produzione = 0;
double tempo_ultima_lettura_consumi = 0;
double tempo_adesso_consumi = 0;
double tempo_ultima_lettura_produzione = 0;
double tempo_adesso_produzione = 0;
unsigned long tempo_trascorso_consumi = 0;         // non modificare unsigned long anzichè int:
unsigned long tempo_trascorso_produzione = 0;         // non modificare unsigned long anzichè int:
unsigned long contawatt = 0;
int volt_partitore_consumi = 0;
int volt_partitore_produzione = 0;
double vecchio_millis_rele = 0;
double corrente_millis_rele = 0;
int impulsi_consumi = 0;
int impulsi_produzione = 0;
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
int contarele_on = 0;
// int variazione = 100;
//
// **********************************************************************************************************
int impulsi_kwh = 3200;                          // dato da modificare in base agli impulsi del contatore:
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
  lcd.begin(8, 2); // Inizializzo LCD 8 caratteri per 2 linee, SERIALE:
  lcd.setCursor(0, 0);
  lcd.print("V34 PARS");
  lcd.setCursor(0, 1);
  lcd.print("ICITALIA");
  // tone(2,1000,2000);
  delay(3000);
  lcd.setCursor(0, 0);
  lcd.print("Monitora");
  lcd.setCursor(0, 1);
  lcd.print(" Cons+FV");
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
      tempo_ultima_lettura_consumi = tempo_adesso_consumi;
      corrente_millis_consumi = tempo_adesso_consumi;
      tempo_trascorso_consumi = (corrente_millis_consumi - vecchio_millis_consumi);
      vecchio_millis_consumi = corrente_millis_consumi;
      potenza_watt_consumi = 1000000 / tempo_trascorso_consumi * 36 / impulsi_kwh;
      val_cons = potenza_watt_consumi;
      impulsi_consumi = impulsi_consumi + 1;
    }
  }
  //
  //
  tempo_adesso_produzione = millis();
  if (tempo_adesso_produzione > vecchio_millis_produzione + 100)   // bisogna che siano passati almeno 100 millisecondi dall'ultima lettura:
    // (mettere 100 per misure fino a 6 Kw e 200 per misure fino a 3 kw).
  {
    // legge lo stato e percepisce l'impulso sul pin deputato a rilevare i consumi:
    volt_partitore_produzione = analogRead(A7);
    if (volt_partitore_produzione > 200)       // la tensione deve essere maggiore di 1 volt:
    {
      tempo_ultima_lettura_produzione = tempo_adesso_produzione;
      corrente_millis_produzione = tempo_adesso_produzione;
      tempo_trascorso_produzione = (corrente_millis_produzione - vecchio_millis_produzione);
      vecchio_millis_produzione = corrente_millis_produzione;
      potenza_watt_produzione = 1000000 / tempo_trascorso_produzione * 36 / impulsi_kwh;
      val_prod = potenza_watt_produzione;
      //    impulsi_produzione = impulsi_produzione + 1;
    }
  }
  //
  // step 3 visualizzazione dati consumi e stato relè:
  //  if (conta2 > 400)    // va bene per gestione carichi prioritari:
  //  {
  if (conta3 > 10000 && conta3 < 10002)        // durata visualizzazione circa 2 secondi, alternata con quella della riga ON/OFF:
  {
    lcd.setCursor(0, 1);
    lcd.print("i:       ");
    lcd.setCursor(3, 1);
    lcd.print(val_cons);
    lcd.setCursor(0, 0);
    lcd.print("W_consum");
  }
  if (conta3 > 20000 && conta3 < 20002)        // durata visualizzazione circa 2 secondi, alternata con quella della riga ON/OFF:
  {
    lcd.setCursor(0, 1);
    lcd.print("i:       ");
    lcd.setCursor(3, 1);
    lcd.print(val_prod);
    lcd.setCursor(0, 0);
    lcd.print("W_produz");
  }
  if (conta3 > 30000 && conta3 < 30002)        // durata visualizzazione circa 2 secondi, alternata con quella della riga ON/OFF:
  {
    if (val_prod > val_cons)
    {
      lcd.setCursor(0, 1);
      lcd.print("s:       ");
      lcd.setCursor(3, 1);
      lcd.print(val_prod - val_cons);
      lcd.setCursor(0, 0);
      lcd.print("W_surplu");
    }
    else
    {
      lcd.setCursor(0, 1);
      lcd.print("vi:      ");
      lcd.setCursor(3, 1);
      lcd.print(val_cons - val_prod);
      lcd.setCursor(0, 0);
      lcd.print("W_prelie");
    }
  }
  conta1 = 0;
  if (conta2 > 400 ) conta2 = 400;
  if (conta3 > 30000) conta3 = 0;     // quando conta3 supera quota 20000 si riparte da zero:
  //
  //  }
  //
  //
  //}
  conta1 = conta1 + 1;
  conta2 = conta2 + 1;
  conta3 = conta3 + 1;
  conta4 = conta4 + 1;
  //
}
// FINE listato by SoftPlus Consumo Zero - Faenza.






























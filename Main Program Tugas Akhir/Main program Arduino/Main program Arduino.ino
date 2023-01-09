#include <EEPROM.h>                   // Library untuk ROM mikrokontroler
#include <Wire.h>                     // Library untuk komunikasi dengan I2C
#include <OneWire.h>                  // Library untuk 1-wire 
#include "WiFiEsp.h"
#include "secrets.h"
#include "ThingSpeak.h"               // library untuk Thingspeak
#define ONE_WIRE_BUS A1               // menetapkan pin A1 sebagai ONE_WIRE_BUS
OneWire oneWire(ONE_WIRE_BUS);        // inisialisasi variabel 'oneWire'
#include <DallasTemperature.h>        // Library untuk sensor suhu
DallasTemperature sensors(&oneWire);  // inisialisasi sensor suhu dan meneruskan nilai 'oneWire' sebagai pin-nya
#include "DFRobot_PH.h"               // Library untuk sensor pH
#define PH_PIN A0                     // menetapkan pin A0 sebagai PH_PIN
DFRobot_PH ph;                        // deklarasi variabel ph
#include "GravityTDS.h"               // Library untuk sensor TDS
#define TdsSensorPin A2               // menetapkan pin A2 sebagai TdsSensorPin
GravityTDS gravityTDS;                // deklarasi variabel gravityTDS
#include <SoftwareSerial.h>
// SoftwareSerial SIM900A(3,2);
#include <Keypad.h>                   // Library untuk Keypad
// thinkspeak Library


// Daftar Menu Sitem
String menuItems[] = {"Mulai Pengujian", "Ganti Nomor HP"};
// Variabel Tombol Navigasi
short readKey, key;
// Variabel Kontrol Menu
short menuPage = 0, cursorPosition = 0, result = 0;
short maxMenuPages = round(((sizeof(menuItems) / sizeof(String)) / 2) + .5);
const byte ROWS = 4;      // Empat Baris Tombol
const byte COLS = 4;      // Empat Kolom Tombol
char keys[ROWS][COLS] = {
    {'1','2','3','A'},
    {'4','5','6','B'},
    {'7','8','9','C'},
    {'*','0','#','D'}
};
byte rowPins[ROWS] = {4, 5,  6,  7}; // Menghubungkan pin baris keypad
byte colPins[COLS] = {8, 9, 10, 11}; // Menghubungkan pin kolom keypad
Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );
#include <LiquidCrystal_I2C.h>        // Library untuk LCD
LiquidCrystal_I2C lcd(0x27, 20, 4);   // deklarasi dan inisialisasi LCD
byte ledPin = 13; 
boolean blink = false;
boolean ledPin_state;
// Membuat 4 karakter (simbol) untuk tampilan Menu
byte downArrow[8] = {
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b10101, // * * *
    0b01110, //  ***
    0b00100  //   *
};
byte upArrow[8] = {
    0b00100, //   *
    0b01110, //  ***
    0b10101, // * * *
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b00100, //   *
    0b00100  //   *
};
byte menuCursor[8] = {
    B01000, //  *
    B00100, //   *
    B00010, //    *
    B00001, //     *
    B00010, //    *
    B00100, //   *
    B01000, //  *
    B00000  //
};

byte backArrow[8] = {
    B00001, //     *
    B00010, //    *
    B00100, //   *
    B01000, //  *
    B00100, //   *
    B00010, //    *
    B00001, //     *
    B00000  //
};
struct data
{
    float ph_val, temp_val, tds_val;
};
float ph_akhir = 0, temp_akhir = 0, fahr_akhir = 0, tds_akhir = 0;
float classkategori;  
String recomendationAction = "";
String recomendationActiontds = "";

String ph_recc, temp_recc, tds_recc;
String phSMS, tempSMS, tdsSMS, pesan_SMS;
// Inputan PengguLakukan untuk meningkatkan pH air tambakna
short jumlahTitikDipilih;
short counter = 0;
String nomorTelepon = "+6282273958244";
void setup(){
    pinMode(ledPin, OUTPUT);              // Sets the digital pin as output.
    digitalWrite(ledPin, HIGH);           // Turn the LED on.
    ledPin_state = digitalRead(ledPin);   // Store initial LED state. HIGH when LED is on.
    lcd.createChar(0, menuCursor);
    lcd.createChar(1, upArrow);
    lcd.createChar(2, downArrow);
    lcd.createChar(3, backArrow);
// 
    // SIM900A.begin(9600);                  // memulai serial GSM pada baud rate 9600

    Serial.begin(9600);                   // memulai serial moniter pada baud rate 9600
    keypad.addEventListener(keypadEvent); // memulai 'event listener' keypad
    ph.begin();                           // memulai sensor pH
    sensors.begin();                      // memulai sensor suhu
    lcd.init();                           // memulai LCD
    
    gravityTDS.setPin(TdsSensorPin);      // menentukan pin untuk sensor TDS
    gravityTDS.setAref(5.0);              // menentukan nilai tegangan untuk sensor TDS
    gravityTDS.setAdcRange(1023);         // menentukan rentang Analog-to-Digital Converter (Arduino Uno = 10 bit)
    gravityTDS.begin();                   // memulai sensor TDS
    
    // Menampilkan tulisan "pondomo" ketika LCD menyala
    lcd.backlight();
    lcd.setCursor(6,1);
    lcd.print("pondomo");
    delay(2000);
    lcd.clear();
}

void loop(){
    ph_akhir = 0, temp_akhir = 0, fahr_akhir = 0, tds_akhir = 0;
    mainMenuDraw();
    drawCursor();
    operateMainMenu();
}
// Taking care of some special events.
void keypadEvent(KeypadEvent key){
    switch (keypad.getState()){
        int readKey;     
        case PRESSED:
        break;
    }
}
// Menampilkan menu utama sistem
void mainMenuDraw() {
    lcd.clear();
    lcd.setCursor(2, 1);
    lcd.print(menuItems[menuPage]);
    lcd.setCursor(2, 2);
    lcd.print(menuItems[menuPage + 1]);
}
// Ketika dipanggil, fungsi ini akan menghapus 'cursor' saat ini dan menampilkannya ulang berdasarkan variabel 'cursorPosition' dan 'menuPage'
void drawCursor() {
    
    // menghapus 'cursor' saat ini
    lcd.setCursor(0, 1);
    lcd.print(" ");
    lcd.setCursor(0, 2);
    lcd.print(" ");

    // The menu is set up to be progressive (menuPage 0 = Item 1 & Item 2, menuPage 1 = Item 2 & Item 3, menuPage 2 = Item 3 & Item 4), so
    // in order to determine where the cursor should be you need to see if you are at an odd or even menu page and an odd or even cursor position.
    if (menuPage % 2 == 0) {
        if (cursorPosition % 2 == 0) {  // If the menu page is even and the cursor position is even that means the cursor should be on line 1
            lcd.setCursor(0, 1);
            lcd.write(byte(0));
        }
        if (cursorPosition % 2 != 0) {  // If the menu page is even and the cursor position is odd that means the cursor should be on line 2
            lcd.setCursor(0, 2);
            lcd.write(byte(0));
        }
    }
    if (menuPage % 2 != 0) {
        if (cursorPosition % 2 == 0) {  // If the menu page is odd and the cursor position is even that means the cursor should be on line 2
            lcd.setCursor(0, 2);
            lcd.write(byte(0));
        }
        if (cursorPosition % 2 != 0) {  // If the menu page is odd and the cursor position is odd that means the cursor should be on line 1
            lcd.setCursor(0, 1);
            lcd.write(byte(0));
        }
    }
}

void operateMainMenu() {
    short activeButton = 0;
    while (activeButton == 0) {
        short button = evaluateButton(readKey);
        
        switch (button) {
            case 1: // Tidak melakukan aksi apapun
                break;
            case 3:
                button = 0;
                cursorPosition = cursorPosition - 1;
                cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
                mainMenuDraw();
                drawCursor();
                activeButton = 1;
                break;  
            case 4:  // Dieksekusi ketika suatu pilihan dipilih
                button = 0;
                switch (cursorPosition) {
                    // Melakukan navigasi ke menu yang dipilih
                    case 0: mulaiPengujian();   break;
                    case 1: ubahNomorHP();      break;
                }
                activeButton = 1;
                mainMenuDraw();
                drawCursor();
                break;
            case 5:
                button = 0;
                cursorPosition = cursorPosition + 1;
                cursorPosition = constrain(cursorPosition, 0, ((sizeof(menuItems) / sizeof(String)) - 1));
                mainMenuDraw();
                drawCursor();
                activeButton = 1;
                break;
        }
    }
}

// 1 = 49     2 = 50      3 = 51      A = 65
// 4 = 52     5 = 53      6 = 54      B = 66
// 7 = 55     8 = 56      9 = 57      C = 67
// * = 42     0 = 48      # = 35      D = 68

short evaluateButton(short x) {
    short result = 0;   
    short key = keypad.getKey();
    if (key < 57) {
        delay(100);
    }
    if(key!=0) Serial.println(key);
    x=key;

    if      (x == 52)               result = 2; 
    else if (x == 50)               result = 3; 
    else if (x == 53 || x == 54)    result = 4; 
    else if (x == 56)               result = 5; 
    else                            result = 1;

    return result;
}

void mulaiPengujian() {
    short activeButton = 0;

    lcd.clear();

    // Tekan * untuk kembali
    lcd.setCursor(0, 3);
    lcd.write(byte(3));
    lcd.setCursor(1, 3);
    lcd.print("(*)");
    
    // Tekan # untuk lanjut
    lcd.setCursor(15, 3);
    lcd.print("(#)OK");

    // Pesan untuk memasukkan jumlah titik yang akan diuji
    lcd.setCursor(1, 1);
    lcd.print("Jumlah titik : ");

    while (activeButton == 0) {

        short button = keypad.getKey();
        if(button!=0) Serial.println(button);
        if(button<49) delay(100);
        
        switch (button) {
            case 49 : jumlahTitik(1); break;
            case 50 : jumlahTitik(2); break;
            case 51 : jumlahTitik(3); break;
            case 52 : jumlahTitik(4); break;
            case 53 : jumlahTitik(5); break;
            case 54 : jumlahTitik(6); break;
            case 55 : jumlahTitik(7); break;
            case 56 : jumlahTitik(8); break;
            case 57 : jumlahTitik(9); break;
            case 68 : hapusJumlahTitik(); break;
            case 42 :
                button = 0;
                activeButton = 1;
                break;
            case 35 : 
                if(jumlahTitikDipilih > 0){
                    button = 0;
                    activeButton = 1;
                    prosesPengujian(jumlahTitikDipilih);
                    lihatDataAkhir(ph_akhir, temp_akhir, fahr_akhir, tds_akhir);
                    tampilUlasan();
                    break;
                }
            default : break;
        }
    }
}

void jumlahTitik(short value){
    lcd.setCursor(16, 1);
    jumlahTitikDipilih = value;
    lcd.print(jumlahTitikDipilih);
}

void hapusJumlahTitik(){
    lcd.setCursor(16, 1);
    lcd.print(" ");
}

void ubahNomorHP(){
    int activeButton = 0;
    lcd.clear();

    // Tekan * untuk kembali
    lcd.setCursor(0, 3);
    lcd.write(byte(3));
    lcd.setCursor(1, 3);
    lcd.print("(*)");
    
    // Tekan # untuk lanjut
    lcd.setCursor(15, 3);
    lcd.print("(#)OK");

    lcd.setCursor(0, 1);
    lcd.print("Masukkan nomor HP :");
    lcd.setCursor(0, 2);
    lcd.print("082273958244");
    
    delay(10000);

    lcd.clear();
    lcd.setCursor(0, 1);
    lcd.print(" Nomor HP Disimpan  ");
    lcd.setCursor(0, 2);
    lcd.print("       (#) OK       ");

    delay(10000);
}

void prosesPengujian(short n){
    data pengujian[n];
    // deklarasi dan inisialisasi variabel

    short i = 0;
    while (i<n){
        sensors.requestTemperatures(); // memulai

        lcd.clear();
        lcd.setCursor(1,0);
        lcd.print("PENGAMBILAN DATA ");
        lcd.print(i+1);
        lcd.setCursor(2,1);
        lcd.print("mohon tunggu ...");
        lcd.setCursor(4,2);
        lcd.print("[");
        lcd.setCursor(15,2);
        lcd.print("]");

        for(short j=1; j<=10; j++){
            delay(3500);
            short col = j+4;
            short percent = j*10;
            lcd.setCursor(col,2); lcd.print("=");
            lcd.setCursor( 8, 3); lcd.print("   ");
            lcd.setCursor( 8, 3); lcd.print(percent);
            lcd.setCursor(11, 3); lcd.print("%");
            delay(1500);
        }

        lcd.clear();
        pengujian[i].temp_val   = sensors.getTempCByIndex(0); // membaca nilai dari sensor suhu
        temp_akhir+=pengujian[i].temp_val;
        float far               = DallasTemperature::toFahrenheit(pengujian[i].temp_val); // konversi celcius ke fahrenheit
        fahr_akhir+=far;
        float voltage           = analogRead(PH_PIN) / 1023.0 * 5000;// membaca nilai tegangan yang diterima pin 'PH_PIN'
        pengujian[i].ph_val     = ph.readPH(voltage, pengujian[i].temp_val);// menghitung nilai pH dengan nilai tegangan dan kompensasi suhu
        ph_akhir += pengujian[i].ph_val;
        gravityTDS.setTemperature(pengujian[i].temp_val);// menetapkan nilai suhu dan menghitung nilai kompensasi suhu
        gravityTDS.update();// memperbarui data
        pengujian[i].tds_val    = gravityTDS.getTdsValue();// mendapatkan nilai TDS (ppm)
        tds_akhir+=pengujian[i].tds_val;

        int currentPrev = 1;
        bool cekPrevData =
         tampilDataTitik(i+1, pengujian[i].ph_val, pengujian[i].temp_val, far, pengujian[i].tds_val, n,currentPrev);
        
        while (cekPrevData) {
          if(currentPrev == 0) {
            cekPrevData = false;
          }

          if(tampilDataTitik(i+1, 
          pengujian[i-currentPrev].ph_val, 
          pengujian[i-currentPrev].temp_val, 
          far, 
          pengujian[i-currentPrev].tds_val, n, currentPrev)) 
          
          {
            currentPrev++; 
          }else if (currentPrev != 0) {
            currentPrev--;
          }

        }
        i++;
        counter = i;
    }
}

bool tampilDataTitik(int idx, float P, float C, float F, float T, int n, int m){
    String index = String(idx), ph = String(P,2), cels = String(C,1), fahr = String(F,1), tds = String(T,0);
    
    // String item1 = String("    DATA TITIK " + index + "    ");
    String item2 = String("ph   : " + ph);
    String item3 = String("Suhu : " + cels + (char)223 + "C|" + fahr + (char)223 + "F");
    String item4 = String("TDS  : " + tds + " ppm");
    lcd.clear();
    lcd.setCursor(0,0); lcd.print(item2);
    lcd.setCursor(0,1); lcd.print(item3);
    lcd.setCursor(0,2); lcd.print(item4);

    // lcd.setCursor(0,3); lcd.print(idx);
    if(idx != 1) {
      lcd.setCursor(0,3); lcd.print("(*)PREV");
    } 
    if (idx != n) {
      lcd.setCursor(12,3); lcd.print("(#)NEXT");
    }

    bool lanjutLihatData = true;
    while(lanjutLihatData == true){
        short button = keypad.getKey();
        // Next Button
        if (button==35) { 
            lanjutLihatData = false;
            lcd.clear();
            return 0;
            // break;
        } 
        // Previous Button
        else if (button == 42 && idx != 1 && m != idx-1) {
            lanjutLihatData = false;
            lcd.clear();
            return 1;
        }
    }
}
void lihatDataAkhir(float P, float C, float F, float T){
    // pesan_SMS = "";

    P/=jumlahTitikDipilih;
    // if(P<7.0){
    //     ph_recc     = "pH terlalu rendah";
    //     phSMS       = "pH terlalu rendah, lakukan pengkapuran untuk meningkatkannya\n";
    // }else if(P>8.5){
    //     ph_recc     = "pH terlalu tinggi";
    //     phSMS       = "pH terlalu tinggi, lakukan penambahan air untuk menurunkannya\n";
    // }else{
    //     ph_recc     = "pH sudah optimal";
    //     phSMS       = "pH air sudah optimal\n";
    // }

    C/=jumlahTitikDipilih;
    // if(C<30){
    //     temp_recc   = "Suhu terlalu dingin";
    //     tempSMS     = "Suhu air tambak terlalu dingin\n";
    // }else if(C>32){
    //     temp_recc   = "Suhu terlalu panas";
    //     tempSMS     = "Suhu air tambak terlalu panas\n";
    // }else{
    //     temp_recc   = "Suhu sudah optimal";
    //     tempSMS     = "Suhu air tambak sudah optimal\n";
    // }

    F/=jumlahTitikDipilih;
    T/=jumlahTitikDipilih;
    // if(T>600){
    //     tds_recc    = "TDS terlalu tinggi";
    //     tdsSMS      = "TDS air terlalu tinggi, lakukan filtrasi pada air\n";
    // }else if(T<300){
    //     tds_recc    = "TDS terlalu rendah";
    //     tdsSMS      = "TDS air terlalu rendah, lakukan pengkapuran untuk meningkatkannya\n";
    // }else{
    //     tds_recc    = "TDS sudah optimal";
    //     tdsSMS      = "TDS air sudah optimal\n";
    // }

    // pesan_SMS = String(phSMS + tempSMS + tdsSMS);
    

    String ph = String(P,2), cels = String(C,1), fahr = String(F,1), tds = String(T,0);
    
    String item1 = String("     DATA AKHIR     ");
    String item2 = String("ph   : " + ph);
    String item3 = String("Suhu : " + cels + (char)223 + "C|" + fahr + (char)223 + "F");
    String item4 = String("TDS  : " + tds + " ppm");
    lcd.clear();
    lcd.setCursor(0,0); lcd.print(item1);
    lcd.setCursor(0,1); lcd.print(item2);
    lcd.setCursor(0,2); lcd.print(item3);
    lcd.setCursor(0,3); lcd.print(item4);
    waterClassification(P,C,T);
    // delay(30000);
    // lcd.clear();
    // lcd.setCursor(0,1); lcd.print("    LIHAT ULASAN    ");
    // lcd.setCursor(0,2); lcd.print("       (#) OK       ");
    bool lanjutLihatData = true;
    while(lanjutLihatData == true){
        short button = keypad.getKey();
        if (button==35) { 
            lanjutLihatData = false;
            break;
        }
    }
}
void tampilUlasan(){
    String outputClassification =  stringClassification();
    lcd.clear();
    // lcd.setCursor(0,0); lcd.print(" KONDISI AIR TAMBAK " + outputClassification);
    lcd.setCursor(5,0);
    lcd.print("C4.5 Rule");
    lcd.setCursor(5,2); lcd.print(outputClassification);
    nextMove();
    lcd.clear();
    lcd.setCursor(2,0); lcd.print("Rekomendasi Aksi");
    lcd.setCursor(0,2); lcd.print(recomendationAction);
    lcd.setCursor(0,3); lcd.print(recomendationActiontds); 
    nextMove();
    lcd.clear();
    startThinkSpeak();
    nextMove();
}

void nextMove() {
  bool lanjutLihatData = true;
  while(lanjutLihatData == true){
        short button = keypad.getKey();
        if (button==35) { 
            lanjutLihatData = false;
            break;
        }
    }
}

//Algoritma C4.5 rule 

void waterClassification (float pH, float suhu, float TDS){
  //Klasifikasi Nilai Ph
   if (pH <= 6.75) {
        if (TDS <= 655.47) {
            if (pH <= 6.36) {
                if (TDS <= 320.91) {
                    if (TDS <= 313.94) {
                        if (TDS <= 306.97) {
                            if (suhu <= 29.76) {
                                classkategori = 0;
                                recomendationAction = "beri kapur (banyak)";
                                   recomendationActiontds ="TDS sudah optimal";
                            }
                            else {
                              classkategori = 2;
                              recomendationAction = "beri kapur (banyak)";
                                   recomendationActiontds ="TDS sudah optimal";      
                        }
                        }
                        else {
                          classkategori = 0;
                             recomendationAction = "beri kapur (banyak)";
                                   recomendationActiontds ="TDS sudah optimal";
                        }
                    }
                    else {
                        classkategori = 2;
                              recomendationAction = "beri kapur (banyak)";
                                   recomendationActiontds ="TDS sudah optimal";
                    }
                }
                else {
                   classkategori = 0;
                                recomendationAction = "beri kapur (banyak)";
                                   recomendationActiontds ="TDS sudah optimal";
                }
            }
            else {
               if (suhu <= 30.52) {
                    // rule ini terpenuhi
                    classkategori = 2;
                     recomendationAction = "beri kapur (sedikit)";
                                   recomendationActiontds ="TDS sudah optimal";

                } else {
                    // rule ini terpenuhi
      
                    classkategori = 0;
                           recomendationAction = "beri kapur (sedikit)";
                                   recomendationActiontds ="berikan balok es";
                }
             }
           } //batas pH banyak 
        
            else {
              if (pH <=6.24){
                if(suhu<=30){
                  classkategori=0;
                  recomendationAction = "Lakukan penggantian";
                                   recomendationActiontds ="air tambak";
                }
                else {
                  classkategori= 3;
                    recomendationAction = "Lakukan penggantian";
                                   recomendationActiontds ="air tambak";
                     }
                }
           else {
          if(suhu<=29.48){
            classkategori =2;
              recomendationAction = "beri kapur(sedikit)";
                                  
          }
          else {
            classkategori = 0;
            recomendationAction = "beri kapur(sedikit)";
            recomendationActiontds ="beri balok es";
          }
        }
    }
   }
   

   else if (pH > 6.75) {
    if (pH <= 8.46) {
      if (TDS <= 634.56) {
        if (TDS <= 425.46) {
          if (TDS <= 376.57) {
            if (pH <= 8.25) {
              classkategori= 1;
                recomendationAction = "air tambak sudah optimal";
        
            }
            else {
              classkategori =2;
                recomendationAction = "berikan asam ke tambak";
                 recomendationActiontds ="(sedang)";
            }
          }
          else {
            if(suhu<=30.52){
              if(pH<=7.89){
                classkategori=1;
                  recomendationAction = "air tambak sudah optimal";
              }
              else {
                classkategori = 2;
                   recomendationAction = "berikan asam ke tambak";
                 recomendationActiontds ="(sedikit)";

              }
            }
            else {
              if(TDS<=404.55){
                classkategori= 2;
                 recomendationAction = "berikan balok es";
                      recomendationActiontds ="(sedikit)";
              }
              else {
                classkategori =2 ;
                recomendationAction = "berikan balok es";
                      recomendationActiontds ="(sedikit)";
              }
            }
          }
        }
        else {
          if (pH<=6.9){
            classkategori = 2;
             recomendationAction = "air kurang baik";
          }
          else{
            if(pH<=8.1){
              if(TDS<=613.65){
                classkategori =1;
                  recomendationAction = "air tambak sudah optimal";
              }
              else{
                classkategori = 1;
                  recomendationAction = "air tambak sudah optimal";
              }
            }
            else{
              if(TDS<=474.25){
                classkategori =1;
                  recomendationAction = "air tambak sudah optimal";
              }
              else{
                classkategori=2;
                recomendationAction = "berikan kapur";
                 recomendationActiontds ="(sedikit)";

              }
            }
          }
        }

      }
      else {
        if(suhu<=30.52){
          if(pH <=8.04){
          if(pH<=6.94){
            if(suhu<=29.48){
              classkategori =2 ;
               recomendationAction = "air kurang baik";
            }
            else {
              classkategori = 2 ;
               recomendationAction = "air kurang baik";
            }
          }
          else {
            if(pH<=7.29){
              classkategori = 1;
                recomendationAction = "air tambak sudah optimal";
            }
            else {
              classkategori =1;
                recomendationAction = "air tambak sudah optimal";
            }
          }
          }

           else {
             classkategori = 2;
             recomendationAction = "berikan asam ke tambak";
                 recomendationActiontds ="(sedikit)";
           }
        }
        else {
          if(pH <= 7.74){
            classkategori =2;
             recomendationAction = "berikan balok es";
                      recomendationActiontds ="(sedikit)";
          }
          else {
            if(pH<=7.89){
              classkategori =1;
                recomendationAction = "air tambak sudah optimal";
            }
            else {
              if(suhu<=31.76){
                classkategori =2;
                recomendationAction = "berikan asam ke tambak";
                 recomendationActiontds ="(sedikit)";
              }
              else {
                classkategori=2;
                recomendationAction = "berikan asam ke tambak";
                 recomendationActiontds ="(sedikit)";
              }
            }
          }
        }
      }

    }
    else {
      if(pH<=8.94){
        if(pH <=8.55){
          if(TDS<=697.29){
            classkategori =2;
              recomendationAction = "berikan asam ke tambak";
                 recomendationActiontds ="(sedang)";
          }
          else {
            if(TDS<=794.87){
              classkategori =0;
              recomendationAction = "berikan asam ke tambak";
                 recomendationActiontds ="lakukan filter air";

            }
            else {
              classkategori =2;
              recomendationAction = "lakukan filter air";
               
            }
          }
        }
        else{
          if(suhu<=29.48){
            classkategori =2;
          }
          else {
            if(suhu<=30.52){
              if(TDS<=606.68){
                classkategori =2;
              }
              else {
                classkategori =0;
              }
            }
            else {
              classkategori = 0 ;
            }
          }
        }
      }
      else {
        if(suhu<=30.52){
          classkategori=0;
        }
        else {
          classkategori = 3;
          recomendationAction = "pergantian air";
        }
      }
    }
   }

}


String stringClassification(){
  //Klasifikasi Akhir 
  float Clasification = classkategori ;
  if(Clasification == 3){
    return "Sangat Buruk";
  }else if(Clasification == 0){
    return "Buruk";
  }else if(Clasification ==2){
    return "Kurang Baik";
  }else{
    return "Baik";
  }
}

// void kirimPesan(){
//     SIM900A.println("AT+CMGF=1");                         // Sets the GSM Module in Text Mode
//     delay(1000);

//     String set_phone_number = String("AT+CMGS=\"" + nomorTelepon +"\"\r");
//     SIM900A.println(set_phone_number);                    // Nomor tujuan
//     delay(1000);
    
//     SIM900A.print(pesan_SMS);                             // Konten SMS
//     delay(100);
    
//     SIM900A.println((char)26);                            // Kode ASCII 'CTRL+Z'
//     delay(1000);
    
// }

void startThinkSpeak(){
  lcd.clear();
  lcd.setCursor(0,1); lcd.print("Menghubungkan ...");  

 char ssid[] = "Xperia";   // your network SSID (name) 
  char pass[] = "testdongkakak";   // your network password
  int keyIndex = 0;            // your network key Index number (needed only for WEP)
  WiFiEspClient  client;

  // Emulate Serial1 on pins 6/7 if not present
  #ifndef HAVE_HWSERIAL1
  // #include "SoftwareSerial.h"
  SoftwareSerial Serial1(18,19); // RX, TX
  #define ESP_BAUDRATE  19200
  #else
  #define ESP_BAUDRATE  115200
  #endif

  unsigned long myChannelNumber ="1979856";
  const char * myWriteAPIKey ="JMR8D2GZG1J0AKPC";

  String myStatus = "";
  //Initialize serial and wait for port to open
  Serial.begin(115200);  // Initialize serial
  while(!Serial){
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }
  
  // initialize serial for ESP module  
  setEspBaudRate(ESP_BAUDRATE);
  
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo native USB port only
  }

  Serial.print("Searching for ESP8266..."); 
  // initialize ESP module
  WiFi.init(&Serial1);

  // check for the presence of the shield
  if (WiFi.status() == WL_NO_SHIELD) {
    Serial.println("WiFi shield not present");
    // don't continue
    while (true);
  }
  Serial.println("found it!");
   
  ThingSpeak.begin(client);  // Initialize ThingSpeak

  // Connect or reconnect to WiFi
  if(WiFi.status() != WL_CONNECTED){
     
    while(WiFi.status() != WL_CONNECTED){
      WiFi.begin(ssid, pass);  // Connect to WPA/WPA2 network. Change this line if using open or WEP network
    } 
    lcd.clear();
    lcd.setCursor(0,1); lcd.print("Terhubung SSID:");
    lcd.setCursor(0,2); lcd.print(ssid);
  }
    delay(5000);     

    lcd.clear();
    lcd.setCursor(0,1); lcd.print("Mengirim Data ...");
    ThingSpeak.setField(1, ph_akhir/=jumlahTitikDipilih);
    ThingSpeak.setField(2, temp_akhir/=jumlahTitikDipilih);
    ThingSpeak.setField(3, tds_akhir/=jumlahTitikDipilih);
    ThingSpeak.setField(4, classificationThinkSpeak(stringClassification()));

    // set the status
    ThingSpeak.setStatus(myStatus);
    
    // write to the ThingSpeak channel
    int x = ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);

    delay(5000);     
    lcd.clear();

    if(x == 200){
      lcd.setCursor(0,1); lcd.print("Data Terkirim");
      lcd.setCursor(12,3); lcd.print("(#)NEXT");
    }
    else{
      lcd.setCursor(0,1); lcd.print("Gagal Mengirim Data");
      lcd.setCursor(0,2); lcd.print("error (" + String(x) + ")");
    }
}
void setEspBaudRate(unsigned long baudrate){
  long rates[6] = {115200,74880,57600,38400,19200,9600};

  Serial.print("Setting ESP8266 baudrate to ");
  Serial.print(baudrate);
  Serial.println("...");

  for(int i = 0; i < 6; i++){
    Serial1.begin(rates[i]);
    delay(100);
    Serial1.print("AT+UART_DEF=");
    Serial1.print(baudrate);
    Serial1.print(",8,1,0,0\r\n");
    delay(100);  
  }
    
  Serial1.begin(baudrate);
}

int classificationThinkSpeak (String classification){
  if(classification == "Sangat Buruk") {
    return 3;
  }else if(classification == "Buruk") {
    return 0;
  }else if(classification == "Kurang Baik") {
    return 2;
  }else if(classification == "Baik") {
    return 1;
  }
}
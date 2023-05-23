#include <WiFi.h>
#include <HTTPClient.h>
#include <ArduinoJson.h>
#include <MFRC522.h>
#include <SPI.h>

#define SS_PIN 5
#define RST_PIN 0 

#include <LiquidCrystal_I2C.h>

int ledR=2;
int ledV=4;

byte nuidPICC[4];

// Informations de connexion WiFi
const char *ssid = "DEFARSCI";
const char *password = "defarsci2022";

// Adresse IP du script PHP
const char *server = "https://pointage-app.000webhostapp.com/api/userPointer";
const char *server2 = "https://pointage-app.000webhostapp.com/api/userPointers/";

LiquidCrystal_I2C lcd(0x27,20,4);

void setup()
{
  // Connexion WiFi
  Serial.begin(115200);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED)
  {
    Serial.println(".");
    delay(1000);
  }
  Wire.begin();
  lcd.init();
  lcd.backlight();
  lcd.clear();

  pinMode(ledR, OUTPUT);
  pinMode(ledV, OUTPUT);
  
  Serial.println("Connecté à WiFi");
  lcd.setCursor(1,0);
  lcd.print("Connection réussie");
  delay(1000);
  SPI.begin();
  Serial.println("Scannez votre badge");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Scannez votre badge");
}
void loop() {
  // Lecture des données RFID
  String rfidData;
    do {
    digitalWrite(ledR,HIGH);
    delay(1000);
    digitalWrite(ledR,LOW);
    // Lecture des données RFID
    rfidData = readRFID();
    delay(500);
  } while (rfidData.isEmpty());
  
  Serial.println("Données RFID lues : " + rfidData);
    digitalWrite(ledV,HIGH);
    delay(1000);
    digitalWrite(ledV,LOW);
  // Création de l'objet JSON pour l'envoi des données
  StaticJsonDocument<200> doc;
  doc["carte_id"] = rfidData;

  // Envoi des données en utilisant une requête POST
  HTTPClient http;
  http.begin(server);
  http.addHeader("Content-Type", "application/json");
  int httpCode = http.POST(doc.as<String>());
  http.end();

  if (httpCode > 0) {
    Serial.println("Données envoyées avec succès");
    digitalWrite(ledV,HIGH);
    delay(1000);
    digitalWrite(ledV,LOW);
  } else {
    Serial.println("Erreur lors de l'envoi des données");
    digitalWrite(ledR,HIGH);
    delay(1000);
    digitalWrite(ledR,LOW);
  }

  // Récupération des données stockées en utilisant une requête GET
  HTTPClient http2;
  http2.begin(server2 + rfidData);
  http2.addHeader("Content-Type", "application/json");
  int httpCode2 = http2.GET();
  String payload = http2.getString();
  http2.end();

  if (httpCode2 > 0) {
    // Désérialisation des données JSON
    StaticJsonDocument<1204> doc;
    DeserializationError error = deserializeJson(doc, payload);
  if (error) {
    Serial.println("Erreur de désérialisation : " + String(error.c_str()));
    Serial.println("Carte enregistrée");
    digitalWrite(ledV,HIGH);
    delay(1000);
    digitalWrite(ledV,LOW);
    return;
  }
    //Valeurs reçus
    String prenom=doc["prenom"];
    String nom=doc["nom"];
    String dateDar=doc["heurDarriver"];
    String dateDep=doc["heurDepart"];
    // Traitement des données reçues
    Serial.println("Données récupérées : " +payload);
    Serial.println(prenom.length());
    Serial.println(nom);
    Serial.println(dateDar);
    Serial.println(dateDep);
    if(dateDep=="null"){
        Serial.println("Bienvenu " +prenom+" "+nom);
        digitalWrite(ledV,HIGH);
        lcd.clear();
        lcd.setCursor(6,0);
        lcd.print("Bienvenu ");
        lcd.setCursor(2,1); 
        lcd.print(prenom);
        lcd.setCursor(2,2);
        lcd.print(nom);
        digitalWrite(ledV,LOW);
        delay(2000);
          }
        else{
          Serial.println("Au revoir " +prenom+" "+nom);
          digitalWrite(ledV,HIGH);
          lcd.clear();
          lcd.setCursor(6,0);
          lcd.print("Au revoir");
          lcd.setCursor(2,1); 
          lcd.print(prenom);
          lcd.setCursor(2,2);
          lcd.print(nom);
          digitalWrite(ledV,LOW);
          delay(2000);
      }
    } 
  else {
    Serial.println("Erreur lors de la récupération des données");
  }

  delay(500);
  Serial.println("Scannez votre badge");
  lcd.clear();
  lcd.setCursor(0,0);
  lcd.print("Scannez votre badge");
}

 String readRFID() {
   // Initialisation du lecteur RFID
   MFRC522 mfrc522(SS_PIN, RST_PIN); // SS_PIN et RST_PIN sont les broches utilisées pour communiquer avec le lecteur RFID
   mfrc522.PCD_Init();

   // Lecture des données RFID
   if ( ! mfrc522.PICC_IsNewCardPresent()) {
     return "";
   }

   if ( ! mfrc522.PICC_ReadCardSerial()) {
     return "";
   }

   // Récupération des données de la carte
   MFRC522::PICC_Type piccType = mfrc522.PICC_GetType(mfrc522.uid.sak);
   String rfidData = "";
   for (byte i = 0; i < mfrc522.uid.size; i++) {
     rfidData += (mfrc522.uid.uidByte[i] < 0x10 ? "0" : "");
     rfidData += String(mfrc522.uid.uidByte[i]);
   }

   // Arrêt de la communication avec le lecteur RFID
   mfrc522.PICC_HaltA();
   mfrc522.PCD_StopCrypto1();

   return rfidData;
}

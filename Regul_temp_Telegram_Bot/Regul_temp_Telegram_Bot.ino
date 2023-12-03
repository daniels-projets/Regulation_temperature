//*********************************************************************************************
//       Projet  Regulation de température avec Thermostat émetteur ESP32
//                                                    et  récepteur ESP8266
//                                                 Telégram Bot sur ESP8266 couplé au recepteur
//                              ou avec modif pin (3 RX, 1 TX) pour ESP01
//
//       selon le protocole Esp-Now, Télégram Bot et SoftwareSérial
//
//       sources d'inspiration:  https://randomnerdtutorials.com/
//                         et :  https://github.com/olikraus/u8g2/
//*********************************************************************************************             


#ifdef ESP32
       #include <WiFi.h>
#else
       #include <ESP8266WiFi.h>
#endif

#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h> 
#include <ArduinoJson.h>

// Replace with your network credentials
const char* ssid = "Bbox-Lux";
const char* password = "2427242711";

// Initialize Telegram BOT
#define BOTtoken "6849681295:AAHlnx_lgC6lWufIOA9LIkaPEYvQLnXCn80"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "1406207106"

#ifdef ESP8266
       X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

#include <SoftwareSerial.h>
          SoftwareSerial softSerial(14, 12);// RX, TX
typedef struct struct_message {
    int tempConsigne;//          Zone d'échange pour affichage et gestion consigne
    int telegramConsigne;//      Retour de la consigne modifiée par Telegram
    float Temperature;//         Pour change température entre récépteur et Télégram Bot
    char Action;//               Z = sans action,  F = marche froid,  C = marche chaud
    char valideAction;//        90 = sans action, 70 = marche froid, 67 = marche chaud 
    bool Connection;//           Non utilisé, réservé pour évolution script
} struct_message;
  struct_message espnowDatas;

//______________________________________________
//                         Variables de travail
bool modification = false;
bool setConsigne = false;
int thermostatConsigne;
int telegramNewConsigne;
int valNum;
float Temperature;

//______________________________________________
//                           suite Télégram Bot
WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);
// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

//__________________________________________________
// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {

     Serial.println("handleNewMessages");
     Serial.println(String(numNewMessages));

     for (int i=0; i<numNewMessages; i++) {
          // Chat id of the requester
          String chat_id = String(bot.messages[i].chat_id);
          if (chat_id != CHAT_ID) {
              bot.sendMessage(chat_id, "Unauthorized user", "");
              continue;
             }
    
          // Print the received message
          String text = bot.messages[i].text;
          Serial.println(text);
          String from_name = bot.messages[i].from_name;
//        Message ouverture Télégram Bot
          if (text == "/start") {
              String welcome = "Bonjour, " + from_name + ".\n";
              welcome += "Thermostat\n";
              welcome += "Télégram Bot\n";
              welcome += "------------\n";
              String keyboardJson = "[[\"/Température\", \"/Réglage\"]]";
              bot.sendMessageWithReplyKeyboard(CHAT_ID, "Daniel", "", keyboardJson, true);
              bot.sendMessage(CHAT_ID, welcome, "");
             }
//        Si requète Température, retour Température thermostat et consigne actuelle
          if (text == "/Température") {
              bot.sendMessage(chat_id, "Relevé température  " + String(Temperature));
              bot.sendMessage(chat_id, "Thermostat consigne " + String(thermostatConsigne));
              Serial.println("Relevé température");
             }
//        Mise en forme d'une saisie clavier Télégram pour nouvelle consigne    
          char charBuf[5];
          text.toCharArray(charBuf, 5) ;
          valNum = int(atof(charBuf));    
//        Si requète Réglage message invite pour saisie nouvelle consigne
          if (text == "/Réglage") {
              bot.sendMessage(chat_id, "Réglage consigne", "");
              Serial.println("Réglage consigne ?");
//            Flag pour lancer la récupération saisie clavier
              setConsigne = true;
             }
//        C'est ici que l'on récupère la saisie clavier 
          if ((text != "/Réglage") && (setConsigne == true)) {           
              telegramNewConsigne = valNum; valNum = 0;
//            Flag de sortie récupération saisie
              setConsigne = false;
//            Flag pour lancer toute les taches liées à la modification consigne
              modification = true;
              bot.sendMessage(chat_id, "Nouvelle consigne " + String(telegramNewConsigne));
              Serial.print("Télégram consigne: "); Serial.println(telegramNewConsigne);
             }   

         }

}


void setup() {

     Serial.begin(115200); Serial.println();
     softSerial.begin(9600);
     Serial.println("Initialisation SetUp");

     #ifdef ESP8266
            configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
            client.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
     #endif
  
     // Connect to Wi-Fi
     WiFi.mode(WIFI_STA);
     WiFi.begin(ssid, password);
     #ifdef ESP32
            client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
     #endif
     while (WiFi.status() != WL_CONNECTED) {
            delay(1000);
            Serial.println("Connecting to WiFi..");
           }
     // Print Local IP Address
     Serial.println(WiFi.localIP());

//   Ouvre Télégram Bot et affichage clavier
     String welcome = "Bonjour, ESP32 .......... prêt. \n";
            welcome += "clavier  personnalise  Json\n\n";
     String keyboardJson = "[[\"/Température\", \"/Réglage\"]]";
     bot.sendMessageWithReplyKeyboard(CHAT_ID, "Daniel", "", keyboardJson, true);
     bot.sendMessage(CHAT_ID, welcome, "");

}


void loop() {

//   Pour resset du système en cas de perte Réseau
     if ( WiFi.status() != WL_CONNECTED ) {
        ESP.restart();
        }
     delay(50);

//   Si récéption de récepteur température et consigne actuelles    
     if (softSerial.available()) {
         softSerial.readBytes((byte *)&espnowDatas, sizeof espnowDatas);
         softSerial.flush();
         Temperature = espnowDatas.Temperature;
         thermostatConsigne = espnowDatas.tempConsigne;
        }

//   Consultation Cyclique Télégram Bot
     if (millis() > lastTimeBotRan + botRequestDelay)  {
         int numNewMessages = bot.getUpdates(bot.last_message_received + 1);
         while (numNewMessages) {
                Serial.println("got response");
                handleNewMessages(numNewMessages);
                numNewMessages = bot.getUpdates(bot.last_message_received + 1);
               }
         lastTimeBotRan = millis();
        }

//   Si modification de la consigne, mise à jour structure et envoi sérial vers récépteur
     if (modification == true) {
         espnowDatas.telegramConsigne = telegramNewConsigne;
         softSerial.write((byte *)&espnowDatas, sizeof espnowDatas);
         Serial.print("Nouvelle consigne Télégram "); Serial.println(telegramNewConsigne);
         softSerial.flush();
         Serial.println("Envoi Datas sur Sofware Sérial");
//       Flag de sortie
         modification = false;
        }

}
//*********************************************************************************************
//       Projet  Regulation de température avec Thermostat émetteur ESP32
//                                                    et  récepteur ESP8266
//                                                 Telégram Bot sur ESP8266 couplé au recepteur
//                                                               ou ESP01 avec modif pin TX RX
//       selon le protocole Esp-Now, Télégram Bot et SoftwareSérial
//
//       sources d'inspiration:  https://randomnerdtutorials.com/
//                         et :  https://github.com/olikraus/u8g2/
//*********************************************************************************************                     


#include <U8g2lib.h>
#include <Wire.h>
         U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R0, /* reset=*/ U8X8_PIN_NONE);

#include <ESP8266WiFi.h>
#include <espnow.h>
//                            ESP32 Dev Kit  Régulation émetteur
uint8_t broadcastAddress[] = {0xC8, 0xC9, 0xA3, 0xC6, 0x19, 0x2C};
//______________________________________________________________________________
//                               Structure pour Envoi et Réception Datas Esp-Now                        
typedef struct struct_message {
    int tempConsigne;//          Zone d'échange pour affichage et gestion consigne
    int telegramConsigne;//      Retour de la consigne modifiée par Telegram
    float Temperature;//         Pour change température entre récépteur et Télégram Bot
    char Action;//               Z = sans action,  F = marche froid,  C = marche chaud
    char valideAction;//        90 = sans action, 70 = marche froid, 67 = marche chaud 
    bool Connection;//           Non utilisé, réservé pour évolution script
} struct_message;
  struct_message espnowDatas;

String success;

#include <SoftwareSerial.h>
          SoftwareSerial softSerial(14, 12);// RX, TX

//______________________________________________
//                          Variables de travail
int consigne;
float temp;
char choixAction;//         Action demandée par l'émetteur
bool Changement;//          Flag changement consigne température, pour nouvelle action a définir

//__________________________________________________________________________
//                         Variables gestion pin de sortie pour exploitation
#define actionFroid 13//   Led bleue pour lancer système de rafraichissement
#define actionChaud 2//    Led rouge pour lancer système de chauffage
#define actionZero 15//    Led verte pour tout est OK pas d'action

//_______________________________
// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {

     memcpy(&espnowDatas, incomingData, sizeof(espnowDatas));
     Serial.print("Bytes received: ");
     Serial.println(len) ;delay(10);
     consigne = espnowDatas.tempConsigne;
     temp = espnowDatas.Temperature;
     choixAction = espnowDatas.Action; Serial.println(choixAction); delay(10);
//   Positionnement Flag pour lancer l'action à faire par le système
     Changement = true; delay(10);

}

//___________________________
// Callback when data is sent
void OnDataSent(uint8_t *mac_addr, uint8_t sendStatus) {

     Serial.print("Last Packet Send Status: ");
     if (sendStatus == 0) {
         Serial.println("Delivery success");
        } else {
         Serial.println("Delivery fail");
        }

}

//____________
void setup() {

     Serial.begin(115200); Serial.println();
     Serial.println("Initialisation SetUp");
     softSerial.begin(9600); delay(10);
     Serial.println("Soft Serial OK!");     
//   Pour faire joli init. OLED
     u8g2.begin();
     u8g2.clearBuffer();
     u8g2.enableUTF8Print();
     u8g2.setFont(u8g2_font_emoticons21_tr);
     u8g2.drawGlyph(50, 25, 0x0021);
     u8g2.setFont(u8g2_font_helvR10_tf);
     u8g2.setCursor(25, 55);  
     u8g2.print("OLED OK !");  
     u8g2.sendBuffer();
     delay(1000);
     u8g2.clearBuffer(); u8g2.sendBuffer(); 

//   Partie initialisation Esp-Now  
     // Set device as a Wi-Fi Station
     WiFi.mode(WIFI_STA);
     WiFi.disconnect();
     // Init ESP-NOW
     if (esp_now_init() != 0) {
         Serial.println("Error initializing ESP-NOW");
         return;
        }
     // Set ESP-NOW Role
     esp_now_set_self_role(ESP_NOW_ROLE_COMBO);
     // Once ESPNow is successfully Init, we will register for Send CB to
     // get the status of Trasnmitted packet
     esp_now_register_send_cb(OnDataSent);
     // Register peer
     esp_now_add_peer(broadcastAddress, ESP_NOW_ROLE_COMBO, 1, NULL, 0); 
     // Register for a callback function that will be called when data is received
     esp_now_register_recv_cb(OnDataRecv);

//   Déclaration pin's pour pilotage systèmes
     pinMode(actionFroid, OUTPUT);
     pinMode(actionChaud, OUTPUT);
     pinMode(actionZero, OUTPUT);

}

//___________
void loop() {

//   Si réception de Telegram Bot extraction de la nouvelle consigne
     if (softSerial.available()) {
         Serial.print("Réception new consigne Telegram ");
         softSerial.readBytes((byte *)&espnowDatas, sizeof espnowDatas);
         softSerial.flush();
         consigne = espnowDatas.telegramConsigne;
         Serial.println(consigne);
//       Pour remise à jour affichage sur récepteur
         Changement = true;
         Serial.print("Flag changement "); Serial.println(Changement);
//       Renvoi Datas au thermostat pour  afficher la nouvelle consigne
         espnowDatas.tempConsigne = consigne;
         esp_now_send(0, (uint8_t *) &espnowDatas, sizeof(espnowDatas));         
        }


//   Si Flag changement vrai venant de thermostat ou Télégram Bot
     if (Changement == true) {
//       Envoi à Télégram Bot température et consigne de thermostat via structure
         softSerial.write((byte *)&espnowDatas, sizeof espnowDatas);
         Serial.println("Envoi Datas sur Sofware Sérial");
//      Debug des variables de travail  et préparation affichage
         Serial.println(consigne);
         Serial.println(temp);
         Serial.println(choixAction); 
         u8g2.clearBuffer(); u8g2.sendBuffer(); 

//   Affichage température de BME280 avec une décimale
     u8g2.setFont(u8g2_font_bubble_tn);     
     u8g2.setCursor(25, 23);
     u8g2.print((String(temp).substring(0, 4)));
     u8g2.setFont(u8g2_font_courR14_tr);
     u8g2.setCursor(100, 12);
     u8g2.print("o");

//   Affichage icone thermomètre et température consigne
     u8g2.setFont(u8g2_font_streamline_all_t);     
     u8g2.drawGlyph(30, 54, 0x02bc);
     u8g2.setFont(u8g2_font_fub20_tr);      
     u8g2.setCursor(60, 54);
     u8g2.print(String(consigne));
     u8g2.setFont(u8g2_font_courR14_tr);
     u8g2.setFont(u8g2_font_courR14_tr);
     u8g2.setCursor(100, 40);
     u8g2.print("o");

//   Trace cadre et envoi l'affichage   
     u8g2.drawRFrame(0, 0, 127, 27, 4);
     u8g2.drawRFrame(0, 30, 127, 27, 4);
     u8g2.sendBuffer();     

//   Lancement des actions système      
     if (choixAction == 'Z') { 
         digitalWrite(actionZero, HIGH); digitalWrite(actionFroid, LOW); digitalWrite(actionChaud, LOW);  
         Serial.println("OK !"); 
        }
     if (choixAction == 'C') { 
         digitalWrite(actionFroid, HIGH); digitalWrite(actionChaud, LOW); digitalWrite(actionZero, LOW);  
         Serial.println("Trop froid, mise en route chauffage"); 
        }
     if (choixAction == 'F') { 
         digitalWrite(actionChaud, HIGH); digitalWrite(actionFroid, LOW); digitalWrite(actionZero, LOW);  
         Serial.println("Trop chaud, mise en route ventilation");  
        }

//    Renvoi valideAction pour vérification
      espnowDatas.valideAction = choixAction;
      esp_now_send(0, (uint8_t *) &espnowDatas, sizeof(espnowDatas));
//    Positionnement Flag pour ne pas boucler sur les actions si pas de changement
      Changement = false;

     }  

}
//***********************************************************************************
//       Projet  Regulation de température avec Thermostat récepteur ESP38266
//                                                      et  émetteur ESP32
//       selon le protocole Esp-Now
//       sources d'inspiration:  https://randomnerdtutorials.com/
//                         et :  https://github.com/olikraus/u8g2/
//***********************************************************************************                         


#include <ESP8266WiFi.h>
#include <espnow.h>
//                            ESP32 Dev Kit  Régulation émetteur
uint8_t broadcastAddress[] = {0xC8, 0xC9, 0xA3, 0xC6, 0x19, 0x2C};
//______________________________________________________________________________
//                               Structure pour Envoi et Réception Datas Esp-Now                        
typedef struct struct_message {
    int tempConsigne;//          Non utilisé, réservé pour évolution script
    float Temperature;//         Non utilisé, réservé pour évolution script
    char Action;//               Z = sans action,  F = marche froid,  C = marche chaud
    char valideAction;//        90 = sans action, 70 = marche froid, 67 = marche chaud 
    bool Connection;//           Non utilisé, réservé pour évolution script
} struct_message;
  struct_message espnowDatas;

String success;

//______________________________________________
//                          Variables de travail
char choixAction;//         Action demandée par l'émetteur
bool Changement;//          Flag changement consigne température, pour nouvelle action a définir

//__________________________________________________________________________
//                         Variables gestion pin de sortie pour exploitation
#define actionFroid 13//   Led bleue pour lancer système de rafraichissement
#define actionChaud 14//   Led rouche pour lancer système de chauffage
#define actionZero 12//    Led verte pour tout est OK pas d'action

//__________________________________________________________________________
//                         Variables gestion clignotement LED_BUILTIN
int delai = (1)*1000;
unsigned long lastdelai;
int ledState = false;

//_______________________________
// Callback when data is received
void OnDataRecv(uint8_t * mac, uint8_t *incomingData, uint8_t len) {

     memcpy(&espnowDatas, incomingData, sizeof(espnowDatas));
     Serial.print("Bytes received: ");
     Serial.println(len) ;delay(10);
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
//   Juste pour le fun !
     pinMode(LED_BUILTIN, OUTPUT);

}

//___________
void loop() {

//   Si Flag changement vrai lancement de l'action du système et renvoi de la validation pour confirmation
     if (Changement == true) {

//       Lancement des actions système      
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

//____________________________________________
//   Et pour le fun un clignotant non bloquant
     if ((millis() - lastdelai) > delai) {
         lastdelai = millis();
         ledState = (ledState == false) ? true : false;
         digitalWrite(LED_BUILTIN, ledState);
      }

}

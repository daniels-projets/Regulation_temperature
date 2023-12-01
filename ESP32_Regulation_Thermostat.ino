//***********************************************************************************
//       Projet  Regulation de température avec Thermostat émetteur ESP32
//                                                    et  récepteur ESP8266
//       selon le protocole Esp-Now
//       sources d'inspiration:  https://randomnerdtutorials.com/
//                         et :  https://github.com/olikraus/u8g2/
//***********************************************************************************                         


#include <U8g2lib.h>
#include <Wire.h>
         U8G2_SH1106_128X64_NONAME_F_HW_I2C u8g2(U8G2_R2, /* reset=*/ U8X8_PIN_NONE);

#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
          Adafruit_BME280 bme;
        float temp;//         Température pièce

#include <WiFi.h>
#include <esp_now.h>
//                           Node MCU ESP8266  Régulation récepteur
uint8_t broadcastAddress[] = {0xA8, 0x48, 0xFA, 0xDC, 0x87, 0x8D};
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

esp_now_peer_info_t peerInfo;
String success;

//______________________________________________
//                         Variables de travail
char choixAction;//        Action demandée au récépteur
char postAction;//         Pour déterminer si nouvelle action demandée
int retourAction;//        Stockage valideAction rentournée par le récépteur pour vérification
int consigne = 19;//       Consigne de température thermostat, réglage par défaut
int postConsigne;//        Pour déterminer si nouvelle consigne est demandée
bool Changement;//         Flag changement consigne température, pour nouvelle action a définir
bool statusReport;//       Flag pour renvoi Datas, si erreur délivery ou test vérification faux

//__________________________________________________________
//                         Variables gestion Rotary Encodeur
#define pinSW  5//         SW
#define pinDT  18//        DT
#define pinCLK 19//        CLK
volatile boolean mouvement;
volatile boolean up; 
volatile boolean presse;
int lastBoutonMillis = 0;
bool valide = true;//      Flag pour validation de la nouvelle consigne

//___________________________
// Callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {

     Serial.print("\r\nLast Packet Send Status:\t");
     Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
     if (status == 0) {
         success = "Delivery Success :)";
        } else {
         success = "Delivery Fail :(";
        }
//   Flag statusReport pour renvoi Datas si erreur
     if (status == 0) { statusReport = true; } else { statusReport = false; }

}

//_______________________________
// Callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {

     memcpy(&espnowDatas, incomingData, sizeof(espnowDatas));
     Serial.print("Bytes received: ");
     Serial.println(len);
     retourAction = espnowDatas.valideAction; Serial.println(retourAction);
//   Flag statusReport pour renvoi Datas si erreur retour retourAction # choixAction
     if (retourAction == choixAction) { 
         Serial.print("Action: "); Serial.print(choixAction);
         Serial.print(" > Retour action: "); Serial.print(retourAction); 
         Serial.print("  C'est OK! ");
         statusReport = true;
        } else {
         Serial.println("Erreur retour action!");
         statusReport = false;
        }

}

//_________________________
void IRAM_ATTR click() {//        Click pour valider nouvelle consigne

     if (millis() - lastBoutonMillis > 10) { // Software debouncing buton
         presse = true;
        }
     lastBoutonMillis = millis();     
//   Flag valide pour nouvelle consigne et nouvelle action a définir 
     valide = true;

}

//_________________________      
void IRAM_ATTR rotation() {//        Rotation pour choix nouvelle consigne

//   Flag valide pour réglage nouvelle consigne
     valide = false;
     if (millis() - lastBoutonMillis > 50) {  // Software debouncing buton      
        if (digitalRead(pinDT)) {
           up = digitalRead(pinCLK);
           } else {
           up = !digitalRead(pinCLK);
           mouvement = true;
           }
        lastBoutonMillis = millis();
        }
       
}

//____________
void setup() {

     Serial.begin(115200); Serial.println();
     Serial.println("Initialisation SetUp");
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
//   Pour faire joli init. BME 280  
     bme.begin(0x76);
     u8g2.setFont(u8g2_font_streamline_weather_t);
     u8g2.drawGlyph(50, 25, 0x0036);
     u8g2.setFont(u8g2_font_helvR10_tf);
     u8g2.setCursor(30, 55);  
     u8g2.print("BME OK !");  
     u8g2.sendBuffer();
     delay(1000);
     u8g2.clearBuffer(); u8g2.sendBuffer(); 

//   Déclaration pin's Rotary Encodeur     
     pinMode(pinSW,INPUT_PULLUP);
     pinMode(pinDT,INPUT_PULLUP);
     pinMode(pinCLK,INPUT_PULLUP);  

//   Déclaration routines d'interruption
     attachInterrupt(pinSW, click, RISING);
     attachInterrupt(pinDT, rotation, FALLING);

//   Partie initialisation Esp-Now
     // Set device as a Wi-Fi Station
     WiFi.mode(WIFI_STA);
     // Init ESP-NOW
     if (esp_now_init() != ESP_OK) {
         Serial.println("Error initializing ESP-NOW");
         return;
        }
     // Once ESPNow is successfully Init, we will register for Send CB to
     // get the status of Trasnmitted packet
     esp_now_register_send_cb(OnDataSent);  
     // Register peer
     memcpy(peerInfo.peer_addr, broadcastAddress, 6);
     peerInfo.channel = 0;  
     peerInfo.encrypt = false; 
     // Add peer        
     if (esp_now_add_peer(&peerInfo) != ESP_OK){
         Serial.println("Failed to add peer");
         return;
        }
     // Register for a callback function that will be called when data is received
     esp_now_register_recv_cb(OnDataRecv);

//   Pour faire joli init. Esp-Now
     u8g2.setFont(u8g2_font_streamline_interface_essential_wifi_t);
     u8g2.drawGlyph(50, 25, 0x0030);
     u8g2.setFont(u8g2_font_helvR10_tf);
     u8g2.setCursor(15, 55);  
     u8g2.print("ESP-NOW OK !");  
     u8g2.sendBuffer();
     delay(1000);
     u8g2.clearBuffer(); u8g2.sendBuffer(); 
     
}

//___________
void loop() {

//   Gestion Rotary Encodeur
     if (mouvement) {//      Détection rotation pour nouvelle consigne  
        if (up) {
           consigne++;
           } else {
           consigne--;        
           }
        mouvement = false; 
        }
     if (presse) { //       Détection click pour valider nouvelle consigne
        presse = false;
        }

//   Lecture température BME280
     temp = bme.readTemperature(); espnowDatas.Temperature = temp;

//   Préparation infos sur OLED
     u8g2.clearBuffer();

//   Comparaison température et consigne validée
//   Choix de l'action a envoyer au récépteur
//   Affichage de l'émoticone humeur selon température, froid, à l'aise ou trop chaud
//   Test si changement de seuil température
//   Et positionnement Flag Changement pour nouvelle action à destination du récepteur      
     u8g2.setFont(u8g2_font_unifont_t_emoticons);
     if (int(temp) == consigne && valide == true) {
         choixAction = 'Z';
         u8g2.drawGlyph(11, 20, 0x002a);// OK
         if (postAction != choixAction) { Changement = true; postAction = choixAction; Serial.println(choixAction); }
        } else if (int(temp) < consigne && valide == true) {
         choixAction = 'C';
         u8g2.drawGlyph(11, 20, 0x0065);// Trop froid
         if (postAction != choixAction) { Changement = true; postAction = choixAction; Serial.println(choixAction); }
        } else if (int(temp) > consigne && valide == true){
         choixAction = 'F';
         u8g2.drawGlyph(11, 20, 0x009f);// Trop chaud
         if (postAction != choixAction) { Changement = true; postAction = choixAction; Serial.println(choixAction); }
        }

//   Affichage température de BME280 avec une décimale
     u8g2.setFont(u8g2_font_bubble_tn);     
     u8g2.setCursor(40, 23);
     u8g2.print((String(temp).substring(0, 4)));
     u8g2.setFont(u8g2_font_courR14_tr);
     u8g2.setCursor(114, 12);
     u8g2.print("o");
    // u8g2.drawRFrame(0, 0, 127, 27, 4);

//   Affichage icone thermomètre et température consigne
     u8g2.setFont(u8g2_font_streamline_all_t);     
     u8g2.drawGlyph(13, 54, 0x02bc);
     u8g2.setFont(u8g2_font_fub20_tr);      
     u8g2.setCursor(43, 54);
     u8g2.print(String(consigne));
     u8g2.setFont(u8g2_font_courR14_tr);

//   Affichage symbole degrés en mode normal si Flag Valide vrai ou # durant changement de la consigne
     if (valide == true) {
         u8g2.setCursor(78, 42);
         u8g2.print("o");
         if (postConsigne != consigne) { Changement = true; postConsigne = consigne; Serial.println(consigne); }
        } else {
         u8g2.setCursor(78, 46);
         u8g2.print("#");
        }

//   Affiche icone montée ou descente selon retourAction recu du récépteur
     if (retourAction == 70) {
         u8g2.setFont(u8g2_font_unifont_t_86);
         u8g2.drawGlyph(97, 50, 0x2b02);
        } 
     if (retourAction == 67) {
         u8g2.setFont(u8g2_font_unifont_t_86);
         u8g2.drawGlyph(97, 50, 0x2b00);
        } 

//   Trace cadre et envoi l'affichage   
     u8g2.drawRFrame(0, 0, 127, 27, 4);
     u8g2.drawRFrame(0, 30, 127, 27, 4);
     u8g2.sendBuffer();     

//   Fin du cycle d'affichage des informations 
//________________________________________________________________
//
//   Envoi de la structure espnowDatas si Flag Changement est vrai 
     if (Changement == true) { 
         Serial.println("Envoi espnowDatas");
         espnowDatas.Action = choixAction;
         // Send message via ESP-NOW
         esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &espnowDatas, sizeof(espnowDatas)); delay(10);   

//       Affichage icone wifi si envoi ok ou triangle exclamation si erreur
         if (result == ESP_OK) {
             Serial.println("Sent with success");
             u8g2.setFont(u8g2_font_streamline_interface_essential_wifi_t);
             u8g2.drawGlyph(97, 54, 0x0030);
             u8g2.sendBuffer(); 
             delay(1000);
            } else {
             Serial.println("Error sending the data");
             u8g2.setFont(u8g2_font_streamline_interface_essential_circle_triangle_t);
             u8g2.drawGlyph(97, 54, 0x0032);
             u8g2.sendBuffer(); 
             delay(5000);
            }
//       Positionnement Flag faux pour sortir du mode envoi Datas
         Changement = false; 

        } 

//   Si Flag statusReport est faux courte pause et renvoi structure espnowDatas en boucle
//   En général si le récepteur est déconnecté, 
//   Le programme reprendra automatiquement sont cycle dès le récepteur connecté
//   Et OnDataRecv revalide Flag statusReport à vrai
     if (statusReport == false) { 
         Serial.println("C'est paaaaaas bon du tout!");
         u8g2.setFont(u8g2_font_streamline_interface_essential_circle_triangle_t);
         u8g2.drawGlyph(97, 54, 0x0032);
         u8g2.sendBuffer();
         delay(1000);
         esp_now_send(broadcastAddress, (uint8_t *) &espnowDatas, sizeof(espnowDatas)); delay(10);           
        }

}


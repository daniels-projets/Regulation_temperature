# Regulation_temperature
![image](https://github.com/daniels-projets/Regulation_temperature/assets/126287326/78cef19c-0ddd-4897-9b21-395d7aba1606)
-
![Regul_Telegram_Bot](https://github.com/daniels-projets/Regulation_temperature/assets/126287326/f805d470-e1c7-4489-9e84-790261f37d66)
-
Projet de régulation pour la température avec Emetteur ESP32 vers Récepteur ESP8266 en protocole Esp-Now
Emetteur composé d'un ESP32, OLED SH1106, BME280 et KY040
Recepteur composé d'un ESP8266 et ST010 pour visualiser les actions
-
Démo utilisation
https://github.com/daniels-projets/Regulation_temperature/assets/126287326/be40a93b-feab-4139-a571-7c51fae67b7e
-
Démo passage mode rafraîchissement
https://github.com/daniels-projets/Regulation_temperature/assets/126287326/75a4b709-9193-445d-8d5a-3cd06b6e87ea
-
Démo passage mode chauffage
https://github.com/daniels-projets/Regulation_temperature/assets/126287326/2a68c6f8-c46a-4190-a2f0-2992ad7cf03e
-
![image](https://github.com/daniels-projets/Regulation_temperature/assets/126287326/25fabb4c-9a1d-4576-bc6e-684d00444dbd)
-
Au démarage la temperature est fixée à 19°, pour la modifier tourner l'encodeur et valider par une pression.
Le mode réglage change le symbole ° de la consigne en #, le symbole ° sera remis après la validation de la nouvelle consigne.
Un logo signal wifi, s'affiche durant l'envoi de la consigne vers le récepteur.
En cas de coupure signal ou erreur de transfert (car le recepteur renvoi le code de l'action recu pour verification) un logo ! s'affiche,
et l'émetteur fera de nouvelles tentatives de transfert.
Les émoticons signalent l'état de la température ressenti par rapport à la consigne.
Un symbole flèche vers haut s'affiche pour le déclanchement mode chauffage du récepteur, ou symbole flèche vers le bas pour le mode rafraîchissement déclanché.
-
Démo utilisation Télégram Bot
https://github.com/daniels-projets/Regulation_temperature/assets/126287326/d8f4b0d3-781e-4b49-8e03-068c784f2dd1
-
Deuxième version avec Telegram Bot pour Lecture température et modification consigne à distance.
Aussi affichage température, consigne sur module récepteur et remontée de la nouvelle consigne vers module thermostat.
La liaison module bot et recepteur par Software Serial.






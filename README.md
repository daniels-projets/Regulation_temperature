# Regulation_temperature

<img width="539" alt="Regulation" src="https://github.com/daniels-projets/Regulation_temperature/assets/126287326/5cd42d6d-2e66-4835-b04c-bf3f9de4c228">

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
Au démarage la temperature est fixée à 19°, pour la modifier tourner l'encodeur et valider par une pression.
Le mode réglage change le symbole ° de la consigne en #, le symbole ° sera remis après la validation de la nouvelle consigne.
Un logo signal wifi, s'affiche durant l'envoi de la consigne vers le récepteur.
En cas de coupure signal ou erreur de transfert (car le recepteur renvoi le code de l'action recu pour verification) un logo ! s'affiche,
et l'émetteur fera de nouvelles tentatives de transfert.
Les émoticons signalent l'état de la température ressenti par rapport à la consigne.
Un symbole flèche vers haut s'affiche pour le déclanchement mode chauffage du récepteur, ou symbole flèche vers le bas pour le mode rafraîchissement déclanché.





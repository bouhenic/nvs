# SecureStorage pour ESP32

`SecureStorage` est un module Arduino pour ESP32 permettant de stocker de manière sécurisée des secrets (Wi-Fi, MQTT, identifiants, mots de passe, etc.) dans la mémoire NVS en utilisant le chiffrement AES-GCM 256 bits.

---

## 🧱 Partitionnement et justification du projet

### 📋 Exemple de partitionnement par défaut (ESP32)

Voici un aperçu des partitions typiquement utilisées dans une configuration OTA :

| Label     | Type   | Sous-type     | Offset    | Taille     | Description              |
|-----------|--------|----------------|-----------|------------|--------------------------|
| nvs       | DATA   | WIFI (2)       | 0x9000    | 0x5000     | Données Wi-Fi / config   |
| otadata   | DATA   | OTA (0)        | 0xE000    | 0x2000     | Info OTA (bootloader)    |
| app0      | APP    | OTA_0 (16)     | 0x10000   | 0x140000   | Première image firmware  |
| app1      | APP    | OTA_1 (17)     | 0x150000  | 0x140000   | Deuxième image firmware  |
| spiffs    | DATA   | SPIFFS (130)   | 0x290000  | 0x160000   | Système de fichiers      |
| coredump  | DATA   | UNKNOWN (3)    | 0x3F0000  | 0x10000    | Dump en cas de crash     |

Par défaut, l'ESP32 stocke les identifiants Wi-Fi dans la mémoire NVS sans chiffrement, ce qui peut poser un problème de sécurité en cas d'accès physique à la mémoire flash.

Ce module propose une solution de chiffrement personnalisée qui vient compléter cette gestion implicite du Wi-Fi :

- Il permet de stocker **tout type de secret**, et pas seulement les identifiants Wi-Fi.
- Il chiffre ces données avec une clé dérivée de l'adresse MAC de l'appareil.
- Il donne à l'utilisateur un **contrôle explicite** sur le processus de chiffrement/déchiffrement, contrairement au stockage automatique du Wi-Fi.

**Recommandation :** Utilisez `SecureStorage` pour gérer manuellement les secrets sensibles, en évitant de les exposer dans le firmware ou via le stockage NVS implicite d’ESP-IDF.

---

## 🚀 Fonctionnalités

- Stockage sécurisé des secrets dans la mémoire NVS.
- Chiffrement AES-GCM avec clé dérivée à partir de l'adresse MAC.
- Utilisation simplifiée grâce à une interface claire et intuitive.

---

## 📂 Installation

Placez simplement le fichier `SecureStorage.h` dans le dossier de votre projet Arduino :

```
VotreProjet/
├── VotreProjet.ino
└── SecureStorage.h
```

## ⚡️ Exemple d'utilisation
```
#include <WiFi.h>
#include "SecureStorage.h"

SecureStorage storage;

void setup() {
  Serial.begin(115200);
  WiFi.begin("SSID", "PASSWORD");
  delay(2000);

  // Stocker un secret
  if(storage.storeSecret("wifi_pass", "SuperSecretWiFi")){
    Serial.println("Mot de passe stocké avec succès!");
  }

  // Récupérer un secret
  char secret[64];
  if(storage.retrieveSecret("wifi_pass", secret, sizeof(secret))){
    Serial.print("Mot de passe récupéré : ");
    Serial.println(secret);
  }
}

void loop() {}
```

---

## 🔐 Sécurité

- Clé dérivée automatiquement de l'adresse MAC de l'ESP32.
- Chiffrement AES-GCM avec authentification (tag de vérification).
- Effacement sécurisé des données sensibles en mémoire après utilisation.

---

### 🚩 Comment effacer complètement les anciens secrets Wi-Fi non chiffrés ?

Nous devons effacer le namespace Wi-Fi comme ceci :

```
#include <nvs_flash.h>

void setup() {
  Serial.begin(115200);
  delay(1000);
  
  Serial.println("Effacement complet des informations Wi-Fi en NVS...");
  
  // Effacement complet des infos Wi-Fi
  nvs_flash_init();
  nvs_flash_erase();  // Efface complètement la partition NVS
  nvs_flash_init();   // Re-initialise la partition NVS après effacement
  
  Serial.println("Toutes les informations Wi-Fi en NVS ont été effacées.");
}

void loop() {
  // Rien à faire ici
}
```
### ⚠️ Important :
- La commande nvs_flash_erase() efface toute la partition NVS. Cela supprime donc absolument tout, y compris les paramètres Wi-Fi, tes données perso, et toutes les configurations.
- Après avoir exécuté ce code, toutes les données anciennes disparaîtront complètement.






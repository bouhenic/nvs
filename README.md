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





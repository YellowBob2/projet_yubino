## Projet Yubino – Authenticator embarqué + Client Python

Ce dépôt contient :
- **`authenticator/`** : le firmware AVR jouant le rôle de clé d’authentification (type U2F simplifié) ;
- **`yubino-client/`** : un client Python permettant de piloter l’authenticator (tests unitaires et CLI).

### 1. Firmware `authenticator/`

- Cible : **ATmega328P** (type Arduino Uno).
- Périphériques utilisés :
  - **UART** pour la communication avec le PC (protocole binaire simple) ;
  - **EEPROM** pour stocker les credentials ;
  - **ADC** comme source d’aléa (générateur pseudo‑TRNG) ;
  - **GPIO** pour un **bouton de consentement** et une **LED**.
- Crypto : bibliothèque **micro-ecc** (`micro-ecc/uECC.c`) sur la courbe `secp160r1`.

#### Commandes supportées (côté device)

Les codes sont définis dans `authenticator/consts.h` et correspondent à ceux utilisés par le client Python :

- **`CMD_LIST_CREDENTIALS` (0x00)** : renvoie la liste des credentials stockés ;
- **`CMD_MAKE_CREDENTIAL` (0x01)** : crée une nouvelle paire de clés pour un `app_id` donné ;
- **`CMD_GET_ASSERTION` (0x02)** : signe un challenge pour un `app_id` existant ;
- **`CMD_RESET` (0x03)** : efface tous les credentials de l’EEPROM.

#### Architecture du firmware

- `main.c` :  
  - initialise `uart`, `ui`, `rng` ;  
  - met le CPU en **sleep (SLEEP_MODE_IDLE)** et se réveille sur **interruption UART RX** ;  
  - délègue le traitement à `command_handle(...)` du module de commandes.
- `commands.c` / `commands.h` :  
  - implémentent la logique de haut niveau des commandes (`MAKE_CREDENTIAL`, `GET_ASSERTION`, etc.) ;
  - orchestrent UART, RNG, stockage EEPROM et UI.
- `uart.c` / `uart.h` : initialisation UART (115200 8N1), envoi/réception d’octets/buffers, interruption RX activée.
- `ui.c` / `ui.h` : gestion de la LED et du bouton ; le consentement utilisateur est demandé avant les opérations sensibles, avec **clignotement toutes les 500 ms** et **timeout 10 s** (pour que ce consentement soit donné automatiquement en mode debug, il faut décommenter la ligne 8 de ui.c)
- `rng.c` / `rng.h` : génération d’aléa à partir du bruit de l’ADC.
- `storage.c` / `storage.h` : stockage des credentials en EEPROM (structure `CredentialEntry`, itération, reset, find/save).

#### Schéma matériel / câblage

- **Microcontrôleur** : ATmega328P (Arduino Uno R3).
- **LED de consentement** : connectée au **pin D5 (PD5)** via une résistance, puis au GND, configuré en sortie dans `ui.c`.
- **Bouton poussoir** : connecté au **pin A0 (PC0)**, configuré en entrée avec **pull‑up interne** ; le bouton relie A0 au GND lorsqu’il est pressé.

#### Compilation et flash

Depuis le dossier `authenticator/` :

```bash
make          # compile le firmware (yubino.hex)
make flash    # flash sur /dev/ttyACM0 avec avrdude
```


### 2. Client Python `yubino-client/`

Le client Python se trouve dans le dossier `yubino-client/`.  
Il fournit :
- un module `yubino.device` qui encapsule le protocole binaire vers l’authenticator ;
- une **suite de tests unitaires** (`tests/device.py`) qui vérifie le bon comportement du firmware.

#### Installation pour le développement

Depuis `yubino-client/` :

```bash
python3 -m venv .env
source .env/bin/activate
pip install .
```

#### Lancement des tests

Dans `yubino-client/` :

```bash
python -m unittest tests.device -v
```

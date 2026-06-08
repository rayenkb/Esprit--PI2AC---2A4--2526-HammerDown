# HammerDown — Système ERP d'Atelier & Usine de Menuiserie

## Description
HammerDown est une solution logicielle ERP de bureau premium conçue spécifiquement pour la gestion moderne des ateliers de menuiserie et des usines de fabrication (mobilier, découpe de bois). Développée en C++ avec le framework Qt, elle permet de gérer de bout en bout la production, depuis l'approvisionnement en matières premières jusqu'à la livraison finale des commandes clients. 

Dotée d'une interface graphique interactive haut de gamme (thème sombre boisé avec animations soignées), d'un assistant vocal hors ligne intelligent (Vosk API) et d'un module d'analyse financière et de maintenance prédictive (TCO, Nexus Widget), cette application modernise la gestion opérationnelle tout en simplifiant le quotidien des artisans et administrateurs.

---

## Technologies utilisées
*   **Interface Utilisateur (Frontend)** : C++17, Qt Widgets, QPainter, Animations et transitions fluides (système d'ouverture de fiches d'identité et menus radiaux).
*   **Logique & Intégrations (Backend)** : Vosk Speech-to-Text Engine (Assistant vocal hors ligne local), SMTP Email Dispatcher (envoi de factures et notifications), QNetworkAccessManager pour la météo et l'IA.
*   **Base de données** : Oracle Database (connexions via le pilote natif `QOCI` ou alternativement `QODBC`).

---

## Fichiers de configuration requis
Pour exécuter le projet, les fichiers suivants doivent être présents à la racine du dépôt :
*   `.env` : Fichier de variables d'environnement configuré (créé à partir du fichier exemple [.env.example](file:///h:/7aw/.env.example)).
*   `libvosk.dll` : Bibliothèque d'analyse acoustique vocale pour l'assistant intelligent.
*   `hammerdown_chat.json` : Base de stockage locale pour le chat interne de l'atelier.

---

## Installation et Lancement du Projet

### 1. Prérequis Système
1.  **Système d'exploitation** : Windows (requis pour charger la bibliothèque `libvosk.dll`).
2.  **Compilateur C++** : MSVC (2019/2022) ou MinGW compatible C++17.
3.  **Qt SDK** : Qt 5.15.x ou Qt 6.x.
4.  **Base de données** : Oracle Database configurée et accessible (Port 1521).

### 2. Initialisation de la Base de Données
Avant de lancer l'application, vous devez initialiser le schéma de base de données Oracle.
1.  Connectez-vous à votre instance de base de données Oracle.
2.  Exécutez l'intégralité du script SQL [schema.sql](file:///h:/7aw/docs/schema.sql) situé dans le répertoire `docs/`. Ce script va créer les tables (`CLIENTS`, `EMPLOYEES`, `SUPPLIERS`, `EQUIPMENT`, `ORDERS`, `EQUIPMENT_HISTORY`), configurer les index, séquences et déclencheurs d'audit automatiques, puis charger les données de test.

### 3. Configuration de l'environnement
1.  Dupliquez le fichier de template [.env.example](file:///h:/7aw/.env.example) et renommez la copie en `.env`.
2.  Renseignez vos coordonnées de connexion Oracle ainsi que vos clés d'API (OpenWeatherMap pour les données météo de Tunis et Groq/Giphy pour les assistants interactifs) :
    ```ini
    DB_HOST=localhost
    DB_PORT=1521
    DB_NAME=source_2a4
    DB_USER=SYSTEM
    DB_PASS=esprit1

    AI_API_KEY=votre_cle_groq
    GIPHY_API_KEY=votre_cle_giphy
    OPENWEATHER_API_KEY=votre_cle_openweathermap
    GROQ_API_KEY=votre_cle_groq
    ```

### 4. Compilation et Exécution (via Qt Creator)
1.  Lancez **Qt Creator**.
2.  Ouvrez le projet en sélectionnant le fichier `HammerDown.pro` (ou `CMakeLists.txt`).
3.  Configurez le projet avec votre Kit de compilation (ex: `Desktop Qt 5.15.2 MSVC2019 64bit`).
4.  Cliquez sur le bouton **Exécuter** (icône verte ou `Ctrl + R`).

### 5. Compilation en Ligne de Commande (QMake)
```bash
# Nettoyer et générer les Makefiles
qmake -makefile HammerDown.pro

# Compiler le projet en mode Release
mingw32-make.exe -f Makefile.Release

# Exécuter l'application
release\HammerDown.exe
```

### 6. Compilation en Ligne de Commande (CMake)
```bash
# Configurer le projet
cmake -B build -S .

# Compiler le projet
cmake --build build --config Release

# Exécuter l'application
.\build\Release\HammerDown.exe
```

---

## Fonctionnalités Clés
*   **Gestion Complète (CRUD & Filtres)** : Modules dédiés à la gestion des Clients, Employés, Fournisseurs, Équipements et Commandes.
*   **Triggers d'Audit Oracle** : Suivi en temps réel des actions sur l'inventaire matériel dans une table d'audit dédiée, synchronisée par déclencheur de base de données.
*   **Assistant Vocal Hors Ligne** : Permet aux artisans d'utiliser des commandes vocales simples pour interroger la base ou naviguer dans l'application les mains libres.
*   **Module Nexus & Coûts** : Outils de modélisation prédictive de pannes de machines et calculs financiers de TCO (Coût Total de Possession) avec graphiques Qt interactifs.
*   **Intelligence Météo & Conseils** : Assistant connecté récupérant les conditions en temps réel pour suggérer des recommandations adaptées au traitement du bois (humidité, séchage, OpenWeather + Llama 3).
*   **Service Mail (SMTP) & QR Codes** : Génération de fiches d'identité matérielles, impression de rapports PDF stylisés, envoi de mails automatiques et scans par QR Codes.
*   **Internationalisation dynamique** : Permet de basculer instantanément l'ensemble de l'interface du Français vers l'Anglais.
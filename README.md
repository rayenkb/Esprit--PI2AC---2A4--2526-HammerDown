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

## Prérequis

### Système & Environnement C++
*   **OS cible** : Windows 10/11 (requis pour le chargement dynamique de `libvosk.dll`)
*   **Compilateur** : MSVC (2019/2022) ou MinGW (compatible C++17)
*   **Standard C++** : C++17
*   **Version Qt** : Qt 5.15.x ou Qt 6.x
*   **Base de données** : Oracle Database (Port 1521)
*   **Dépendances système** :
    *   Pilotes de base de données Oracle (`QOCI` ou `QODBC`)
    *   Réseau requis pour les APIs externes (Groq, OpenWeatherMap, Giphy)

---

## Installation

### 1. Fichiers requis à la racine
Assurez-vous que les fichiers suivants sont présents dans le répertoire racine :
*   `libvosk.dll` (Bibliothèque acoustique Vosk pour l'assistant vocal)
*   `hammerdown_chat.json` (Base locale pour la messagerie interne de l'atelier)

### 2. Initialisation de la Base de Données Oracle
Avant de lancer l'application, vous devez initialiser le schéma :
1.  Connectez-vous à votre instance locale/distante Oracle Database.
2.  Exécutez l'intégralité du script SQL [`docs/schema.sql`](docs/schema.sql) pour créer automatiquement les tables (`CLIENTS`, `EMPLOYEES`, `SUPPLIERS`, `EQUIPMENT`, `ORDERS`, `EQUIPMENT_HISTORY`), index, séquences, déclencheurs d'audit automatiques, et insérer le jeu de données fictives.

### 3. Variables d'environnement
1.  Copiez le fichier de template [`.env.example`](.env.example) et renommez-le en `.env`.
2.  Renseignez vos identifiants Oracle et vos clés API (Groq, OpenWeatherMap, Giphy).

---

## Lancement

### Option A. Lancement via Qt Creator (Recommandé)
1.  Ouvrez **Qt Creator**.
2.  Importez le projet en sélectionnant le fichier `HammerDown.pro`.
3.  Configurez le projet avec votre Kit de compilation (ex: `Desktop Qt 6.7.3 MinGW 64-bit`).
4.  Cliquez sur **Exécuter** (ou `Ctrl + R`).

### Option B. Compilation et Lancement en Ligne de Commande (QMake)
```bash
# Générer le Makefile
qmake -makefile HammerDown.pro

# Compiler le projet
mingw32-make.exe -f Makefile.Release

# Lancer l'exécutable
release\HammerDown.exe
```

### Option C. Lancement automatisé via Script
Vous pouvez également exécuter le script de build fourni à la racine :
```cmd
build.bat
```

---

## Variables d'environnement

Voir [`.env.example`](.env.example) pour le modèle complet des clés de configuration.

---

## Démo

*   **Vidéo de démonstration** : https://www.youtube.com/watch?v=wF4bGg_VJuU&t=13s
*   **Déploiement / Release** : https://github.com/SkrrtTn/projectc-/releases

> Captures d'écran et animations disponibles dans le dossier [`demo/`](demo/).

---

## Auteurs

| Nom | Classe | Année | Tuteur |
|-----|--------|-------|--------|
| Hammer Down | PI — 2A4 | 2025–2026 | Soumaya Agroubi |
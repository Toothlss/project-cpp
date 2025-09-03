# LMS - Système de gestion logistique

## Description

Application desktop en C++/Qt pour la gestion logistique d'une entreprise de livraison.

### Modules inclus

- Gestion des Commandes (CRUD, recherche, tri, statistiques, génération PDF, etc.)
- Gestion des Clients (CRUD, recherche, tri, statistiques, génération PDF, etc.)

### Fonctionnalités avancées prévues

- Recherche et tri multicritères (3 critères minimum)
- Statistiques dynamiques avec graphiques
- Génération de documents PDF
- Deux autres métiers utiles par module

### Structure du projet

- `main.cpp`, `mainwindow.*`, `mainwindow.ui` : point d'entrée et fenêtre principale
- `commande.*`, `commande.ui` : gestion des commandes
- `client.*`, `client.ui` : gestion des clients

### Compilation

Ouvrir `LMS.pro` avec Qt Creator, puis compiler et exécuter.

### Base de données

Utilisation de SQLite intégrée via Qt SQL.

---

Projet inspiré de "First working version".

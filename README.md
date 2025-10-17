# LaunchBar - Application Qt/C++

Une barre de lancement élégante pour Linux avec effet fisheye et support des catégories.

## Fonctionnalités

- ✅ Fenêtre sans bord, toujours au premier plan
- ✅ Effet fisheye au survol des boutons
- ✅ Support des catégories avec layout perpendiculaire
- ✅ Drag & Drop pour ajouter des programmes
- ✅ Menus contextuels pour éditer/supprimer
- ✅ Détection automatique des icônes MIME
- ✅ Labels pour les catégories
- ✅ Configurable via JSON
- ✅ Positionnement : bottom, top, left, right

## Compilation

```bash
qmake LaunchBar.pro
make
```

## Utilisation

```bash
./LaunchBar
```

## Configuration

Modifiez le fichier `config.json` pour personnaliser vos applications.

Un script python en CLI a été fait pour gérer le fichier de configuration. Ce script sera capable de rechercher automatiquement les icones.

Il peut aussi enregistrer l'environnement de tel maniere que lorsque vous lancerez l'application , cela sera avec l'environnement 
lorsque vous l'auriez enregistré avec le script python.

```
usage: managetoolbarconfig.py [-h] [--config CONFIG] {init,list,add,add-category,remove,validate} ...

Gestionnaire de configuration pour launchbar

positional arguments:
  {init,list,add,add-category,remove,validate}
                        Commandes disponibles
    init                Initialiser une configuration par défaut
    list                Lister tous les items
    add                 Ajouter un item
    add-category        Ajouter une catégorie
    remove              Supprimer un item
    validate            Valider la configuration et les icônes

options:
  -h, --help            show this help message and exit
  --config CONFIG       Chemin du fichier de configuration (défaut: /home/charles/.launchbar.json)

```


### Structure JSON

```json
{
  "position": "bottom",
  "items": [
    {
      "icon": "/path/to/icon.png",
      "program": "command",
      "label": "Name"
    },
    {
      "icon": "/path/to/folder.png",
      "label": "Category Name",
      "category": [
        {
          "icon": "/path/to/icon.png",
          "program": "command",
          "label": "Subitem"
        }
      ]
    }
  ]
}
```

## Menu contextuel

- **Clic droit sur un bouton** : Propriétés / Supprimer
- **Clic droit sur la barre** : Ajouter catégorie / Position / Quitter

## Drag & Drop

Glissez-déposez des fichiers .desktop ou des exécutables pour les ajouter à la barre.

Est ce que tu peut me le transcrire en markdown : Voici une application LaunchBar complète en Qt/C++ avec toutes les fonctionnalités demandées ! 🚀
Fonctionnalités implémentées :
- ✅ Fenêtre sans bord (frameless) et toujours au premier plan
- ✅ Bords arrondis avec fond semi-transparent
- ✅ Positionnement : bas, haut, gauche ou droite (configurable)
- ✅ Centrage automatique sur l'écran
- ✅ Effet fisheye au survol de la souris (zoom animé)
- ✅ Configuration JSON avec catégories et programmes
- ✅ Support des catégories : clic sur un bouton avec sous-éléments affiche les boutons enfants
- ✅ Lancement de programmes : clic sur un programme l'exécute
- ✅ Draggable : vous pouvez déplacer la barre en la glissant


Ajoute du 17 octobre 2025
---

# ✨ Nouvelles Fonctionnalités : Horloge et Calendrier Intégrés

La LaunchBar a été enrichie de nouvelles fonctionnalités d'horloge et de calendrier, offrant des informations utiles directement depuis la barre.

## ✅ Horloge Numérique

*   **Affichage en temps réel** : Affiche l'heure au format `HH:mm:ss`.
*   **Mise à jour automatique** : Rafraîchissement chaque seconde.
*   **Positionnement** : Située à l'extrémité droite de la barre (si la barre est horizontale).
*   **Style** : Texte blanc sur un fond transparent.

## ✅ Bouton Calendrier

*   **Affichage synthétique** :
    *   Affiche le jour du mois en grand.
    *   Affiche le mois en abréviation en haut.
*   **Mise à jour automatique** : Se met à jour à minuit pour refléter le jour courant.
*   **Design** : Intégration élégante avec un fond semi-transparent.

## ✅ Dropdown Calendrier

*   **Interaction** : Un clic sur le bouton calendrier affiche un calendrier complet du mois.
*   **Grille de jours** : Présente les jours du mois sous forme de grille, avec les jours de la semaine (L, M, M, J, V, S, D).
*   **Indicateur de jour courant** : Le jour actuel est souligné et mis en évidence en bleu (`#4080ff`).
*   **Style des autres jours** : Les autres jours sont affichés en gris, avec un effet de survol.
*   **Toggle** : Un second clic sur le bouton ferme le calendrier.

## ✅ Configuration dans les Préférences

Ces nouvelles fonctionnalités peuvent être activées ou désactivées via le dialogue des préférences de la LaunchBar :

*   **"Afficher l'horloge"** : Une case à cocher pour activer ou désactiver l'horloge numérique.
*   **"Afficher le calendrier"** : Une case à cocher pour activer ou désactiver le bouton calendrier et son dropdown.
*   **Persistance** : L'état de ces options est sauvegardé dans le fichier `config.json` de la barre.

## ✅ Disposition Intelligente

*   **Mode Horizontal** : L'horloge et le calendrier sont visibles et pleinement fonctionnels lorsque la barre est positionnée en `bottom` ou `top`.
*   **Mode Vertical** : En mode `left` ou `right`, l'horloge et le calendrier sont automatiquement cachés pour optimiser l'espace et l'esthétique de la barre verticale.

---

# Screenshot 
  ![screenshot1](https://github.com/zebulon75018/launchbarVC/blob/main/assets/launchbar1.png?raw=true)

  ![screenshot multi level](https://github.com/zebulon75018/launchbarVC/blob/main/assets/launchbar2.png?raw=true)

  ![On bottom](https://github.com/zebulon75018/launchbarVC/blob/main/assets/launchbar3.png?raw=true)

![On bottom](https://github.com/zebulon75018/launchbarVC/blob/main/assets/launchbar4.png?raw=true)

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


# Screenshot 
  ![screenshot1](https://github.com/zebulon75018/launchbarVC/blob/main/assets/launchbar1.png?raw=true)

  ![screenshot multi level](https://github.com/zebulon75018/launchbarVC/blob/main/assets/launchbar2.png?raw=true)

  ![On bottom](https://github.com/zebulon75018/launchbarVC/blob/main/assets/launchbar3.png?raw=true)

![On bottom](https://github.com/zebulon75018/launchbarVC/blob/main/assets/launchbar4.png?raw=true)

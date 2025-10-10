#!/usr/bin/env python3
"""
Gestionnaire de configuration pour toolbar
"""

import json
import os
import argparse
import sys
from pathlib import Path
from typing import List, Dict, Any, Optional

# Configuration par défaut
DEFAULT_CONFIG_PATH = Path.home() / ".toolbarconf.json"
DEFAULT_ITEM = {
    "icon": "/usr/share/icons/hicolor/48x48/apps/vlc.png",
    "label": "VLC",
    "program": "vlc"
}

# Chemins de recherche d'icônes
ICON_SEARCH_PATHS = [
    "/usr/share/icons/hicolor/48x48/apps/",
    "/usr/share/icons/hicolor/64x64/apps/",
    "/usr/share/icons/hicolor/128x128/apps/",
    "/usr/share/pixmaps/",
    "/snap/gtk-common-themes/1535/share/icons/Yaru/48x48/apps/",
]


class ToolbarConfigManager:
    def __init__(self, config_path: Path = DEFAULT_CONFIG_PATH):
        self.config_path = config_path
        self.config = None

    def load_config(self) -> Dict[str, Any]:
        """Charge la configuration depuis le fichier JSON"""
        if not self.config_path.exists():
            return None
        
        try:
            with open(self.config_path, 'r', encoding='utf-8') as f:
                self.config = json.load(f)
            return self.config
        except json.JSONDecodeError as e:
            print(f"❌ Erreur de lecture du fichier JSON: {e}")
            sys.exit(1)

    def save_config(self):
        """Sauvegarde la configuration dans le fichier JSON"""
        with open(self.config_path, 'w', encoding='utf-8') as f:
            json.dump(self.config, f, indent=4, ensure_ascii=False)
        print(f"✅ Configuration sauvegardée dans {self.config_path}")

    def create_default_config(self):
        """Crée une configuration par défaut"""
        self.config = {
            "backgroundColor": {
                "r": 40,
                "g": 40,
                "b": 40
            },
            "iconPaths": ICON_SEARCH_PATHS,
            "iconSize": 64,
            "items": [DEFAULT_ITEM],
            "opacity": 200,
            "position": "bottom"
        }
        self.save_config()
        print(f"✅ Configuration par défaut créée avec un item VLC")

    def find_icon(self, program_name: str) -> Optional[str]:
        """Recherche automatiquement une icône pour un programme"""
        # Nettoyer le nom du programme
        base_name = os.path.basename(program_name).lower()
        
        # Extensions possibles
        extensions = ['.png', '.svg', '.xpm']
        
        # Rechercher dans tous les chemins
        for search_path in ICON_SEARCH_PATHS:
            if not os.path.exists(search_path):
                continue
            
            for ext in extensions:
                # Essayer le nom exact
                icon_path = os.path.join(search_path, f"{base_name}{ext}")
                if os.path.exists(icon_path):
                    return icon_path
                
                # Essayer avec des variantes courantes
                for variant in [base_name, base_name.replace('-', ''), base_name.replace('_', '')]:
                    icon_path = os.path.join(search_path, f"{variant}{ext}")
                    if os.path.exists(icon_path):
                        return icon_path
        
        # Icône par défaut si rien trouvé
        default_icon = "/usr/share/icons/hicolor/48x48/apps/application-default-icon.png"
        print(f"⚠️  Icône non trouvée pour '{program_name}', utilisation d'une icône par défaut")
        return default_icon

    def display_hierarchy(self, items: List[Dict], prefix: str = "", index_prefix: str = ""):
        """Affiche la hiérarchie des items avec des numéros"""
        for i, item in enumerate(items):
            current_index = f"{index_prefix}{i+1}" if index_prefix else str(i+1)
            
            if "category" in item:
                print(f"{prefix}[{current_index}] 📁 {item.get('label', 'Sans nom')}")
                self.display_hierarchy(item["category"], prefix + "  ", current_index + ".")
            else:
                print(f"{prefix}[{current_index}] 📄 {item.get('label', 'Sans nom')}")

    def list_items(self):
        """Liste tous les items de la configuration"""
        if self.config is None:
            print("❌ Aucune configuration chargée")
            return
        
        print("\n📋 Hiérarchie de la configuration:\n")
        self.display_hierarchy(self.config.get("items", []))
        print()

    def find_item_by_path(self, path: str) -> tuple:
        """Trouve un item par son chemin (ex: '3.2')"""
        parts = path.split('.')
        current = self.config["items"]
        parent = None
        
        for part in parts:
            try:
                idx = int(part) - 1
                if idx < 0 or idx >= len(current):
                    return None, None, None
                
                parent = current
                if idx < len(current) - 1:
                    next_item = current[idx + 1]
                else:
                    next_item = None
                
                item = current[idx]
                
                if part == parts[-1]:  # Dernier élément du chemin
                    return item, parent, idx
                
                if "category" not in item:
                    return None, None, None
                
                current = item["category"]
            except (ValueError, IndexError):
                return None, None, None
        
        return None, None, None

    def remove_item(self, index: str):
        """Supprime un item par son index"""
        item, parent, idx = self.find_item_by_path(index)
        
        if item is None:
            print(f"❌ Item '{index}' non trouvé")
            return
        
        label = item.get('label', 'Sans nom')
        confirm = input(f"⚠️  Confirmer la suppression de '{label}' ? (o/n): ")
        
        if confirm.lower() == 'o':
            parent.pop(idx)
            self.save_config()
            print(f"✅ Item '{label}' supprimé")
        else:
            print("❌ Suppression annulée")

    def add_item(self, label: str, program: str, category: Optional[str] = None, save_env: bool = False):
        """Ajoute un nouvel item"""
        icon = self.find_icon(program)
        
        new_item = {
            "icon": icon,
            "label": label,
            "program": program
        }
        
        if save_env:
            new_item["env"] = dict(os.environ)
            print("💾 Environnement du shell enregistré")
        
        if category:
            # Ajouter à une catégorie spécifique
            cat_item, parent, idx = self.find_item_by_path(category)
            
            if cat_item is None:
                print(f"❌ Catégorie '{category}' non trouvée")
                return
            
            if "category" not in cat_item:
                print(f"❌ L'item '{category}' n'est pas une catégorie")
                return
            
            cat_item["category"].append(new_item)
        else:
            # Ajouter à la racine
            self.config["items"].append(new_item)
        
        self.save_config()
        print(f"✅ Item '{label}' ajouté avec succès")
        print(f"   Programme: {program}")
        print(f"   Icône: {icon}")

    def add_category(self, label: str, parent_category: Optional[str] = None):
        """Ajoute une nouvelle catégorie"""
        icon = "/snap/gtk-common-themes/1535/share/icons/Adwaita/16x16/places/folder-documents.png"
        
        new_category = {
            "icon": icon,
            "label": label,
            "program": "",
            "category": []
        }
        
        if parent_category:
            # Ajouter à une catégorie parente
            cat_item, parent, idx = self.find_item_by_path(parent_category)
            
            if cat_item is None:
                print(f"❌ Catégorie parente '{parent_category}' non trouvée")
                return
            
            if "category" not in cat_item:
                print(f"❌ L'item '{parent_category}' n'est pas une catégorie")
                return
            
            cat_item["category"].append(new_category)
        else:
            # Ajouter à la racine
            self.config["items"].append(new_category)
        
        self.save_config()
        print(f"✅ Catégorie '{label}' ajoutée avec succès")

    def validate_config(self):
        """Valide la configuration et vérifie les icônes"""
        if self.config is None:
            print("❌ Aucune configuration chargée")
            return
        
        print("🔍 Validation de la configuration...\n")
        
        # Vérifier la structure JSON
        required_keys = ["items", "iconPaths", "backgroundColor"]
        missing_keys = [key for key in required_keys if key not in self.config]
        
        if missing_keys:
            print(f"⚠️  Clés manquantes: {', '.join(missing_keys)}")
        else:
            print("✅ Structure JSON valide")
        
        # Vérifier les icônes
        print("\n🔍 Vérification des icônes:\n")
        self.validate_icons(self.config.get("items", []))

    def validate_icons(self, items: List[Dict], prefix: str = ""):
        """Vérifie récursivement les icônes"""
        for item in items:
            label = item.get('label', 'Sans nom')
            icon_path = item.get('icon', '')
            
            if icon_path and os.path.exists(icon_path):
                print(f"{prefix}✅ {label}: {icon_path}")
            else:
                print(f"{prefix}❌ {label}: {icon_path} (introuvable)")
            
            if "category" in item:
                self.validate_icons(item["category"], prefix + "  ")


def main():
    parser = argparse.ArgumentParser(
        description="Gestionnaire de configuration pour toolbar",
        formatter_class=argparse.RawDescriptionHelpFormatter
    )
    
    parser.add_argument(
        '--config',
        type=Path,
        default=DEFAULT_CONFIG_PATH,
        help=f"Chemin du fichier de configuration (défaut: {DEFAULT_CONFIG_PATH})"
    )
    
    subparsers = parser.add_subparsers(dest='command', help='Commandes disponibles')
    
    # Commande: init
    subparsers.add_parser('init', help='Initialiser une configuration par défaut')
    
    # Commande: list
    subparsers.add_parser('list', help='Lister tous les items')
    
    # Commande: add
    add_parser = subparsers.add_parser('add', help='Ajouter un item')
    add_parser.add_argument('label', help='Label de l\'item')
    add_parser.add_argument('program', help='Programme à exécuter')
    add_parser.add_argument('--category', help='Index de la catégorie (ex: 3.2)')
    add_parser.add_argument('--env', action='store_true', help='Enregistrer l\'environnement du shell')
    
    # Commande: add-category
    cat_parser = subparsers.add_parser('add-category', help='Ajouter une catégorie')
    cat_parser.add_argument('label', help='Label de la catégorie')
    cat_parser.add_argument('--parent', help='Index de la catégorie parente (ex: 3)')
    
    # Commande: remove
    remove_parser = subparsers.add_parser('remove', help='Supprimer un item')
    remove_parser.add_argument('index', help='Index de l\'item (ex: 3 ou 3.2)')
    
    # Commande: validate
    subparsers.add_parser('validate', help='Valider la configuration et les icônes')
    
    args = parser.parse_args()
    
    # Créer le gestionnaire
    manager = ToolbarConfigManager(args.config)
    
    # Gérer les commandes
    if args.command == 'init':
        if manager.config_path.exists():
            confirm = input(f"⚠️  Le fichier {manager.config_path} existe déjà. Écraser ? (o/n): ")
            if confirm.lower() != 'o':
                print("❌ Initialisation annulée")
                sys.exit(0)
        manager.create_default_config()
    
    elif args.command == 'list':
        if manager.load_config() is None:
            print(f"❌ Fichier {manager.config_path} introuvable")
            print("💡 Utilisez 'init' pour créer une configuration par défaut")
            sys.exit(1)
        manager.list_items()
    
    elif args.command == 'add':
        if manager.load_config() is None:
            print(f"❌ Fichier {manager.config_path} introuvable")
            sys.exit(1)
        manager.add_item(args.label, args.program, args.category, args.env)
    
    elif args.command == 'add-category':
        if manager.load_config() is None:
            print(f"❌ Fichier {manager.config_path} introuvable")
            sys.exit(1)
        manager.add_category(args.label, args.parent)
    
    elif args.command == 'remove':
        if manager.load_config() is None:
            print(f"❌ Fichier {manager.config_path} introuvable")
            sys.exit(1)
        manager.remove_item(args.index)
    
    elif args.command == 'validate':
        if manager.load_config() is None:
            print(f"❌ Fichier {manager.config_path} introuvable")
            sys.exit(1)
        manager.validate_config()
    
    else:
        parser.print_help()


if __name__ == "__main__":
    main()

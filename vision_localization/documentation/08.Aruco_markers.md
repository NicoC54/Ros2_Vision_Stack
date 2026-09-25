# Les Marqueurs ArUco en Robotique

## 1. Introduction : Pourquoi a-t-on besoin de marqueurs ?
En robotique, un robot "voit" le monde à travers des grilles de pixels (les images de sa caméra). Pour un ordinateur, une image n'est qu'une matrice de nombres. Il est très complexe pour un algorithme de regarder une image et de dire : "Ceci est le coin de la table où je dois poser mon colis". 

C'est ici qu'interviennent les **repères fiduciels** (ou *fiducial markers* en anglais). 
**Définition :** Un repère fiduciel est un objet artificiel placé dans le champ de vision d'une caméra, conçu spécifiquement pour être détecté de manière ultra-robuste et rapide par un algorithme.

Un **marqueur ArUco** est un type spécifique de repère fiduciel. Il ressemble à un petit QR code simplifié : un carré noir avec un motif de blocs blancs et noirs à l'intérieur. 

## 2. Les Concepts Clés : Étape par Étape

### A. L'image en Niveaux de Gris (`MONO8`)
* **Définition :** `MONO8` (ou Monochrome 8-bits) est un format d'image où la couleur n'existe pas. Chaque pixel est codé sur 8 bits, ce qui donne une valeur allant de 0 (noir absolu) à 255 (blanc pur).
* **Pourquoi l'utiliser ?** La détection d'ArUco repose sur le contraste entre le noir et le blanc. Les couleurs (Rouge, Vert, Bleu) n'apportent aucune information utile pour trouver ces marqueurs, mais elles multiplient par trois la quantité de données à traiter. Convertir l'image en niveaux de gris permet d'économiser de la puissance de calcul (crucial pour un drone ou un robot en temps réel) et de faciliter la séparation entre ce qui est sombre et ce qui est clair (ce qu'on appelle le *seuillage*).

### B. Le Dictionnaire ArUco
Un robot ne cherche pas des motifs au hasard. On lui donne un "catalogue" des marqueurs qui existent dans son environnement. 
* **Définition :** Un **Dictionnaire** est une collection prédéfinie de marqueurs. Il est défini par deux paramètres :
    1. **La taille de la grille interne :** Par exemple, 4x4, 5x5, 6x6. Un dictionnaire 4x4 aura une grille interne de 16 petits carrés (bits) noirs ou blancs.
    2. **Le nombre de marqueurs uniques :** Par exemple, 50, 100, 250.
* **Comment ça marche ?** Un dictionnaire `DICT_4X4_50` signifie que les marqueurs ont une grille interne de 4x4 et qu'il en existe 50 différents (avec des IDs de 0 à 49).
* **Le compromis :** Plus la grille (ex: 6x6) et le nombre de marqueurs (ex: 1000) sont grands, plus il y a d'IDs possibles. Mais plus la grille est dense, plus la caméra doit être proche ou de haute résolution pour distinguer les petits carrés noirs et blancs.

### C. Le Processus de Détection (`detectMarkers`)
L'algorithme de détection (souvent via la bibliothèque OpenCV) se déroule en plusieurs étapes logiques :
1. **Seuillage (Binarisation) :** L'image en niveaux de gris est transformée en une image purement noire et blanche (0 ou 1) pour isoler les formes sombres sur des fonds clairs.
2. **Extraction des contours :** L'algorithme cherche toutes les formes géométriques qui ressemblent à des carrés.
3. **Extraction de la grille :** Pour chaque carré trouvé, l'algorithme "redresse" la perspective (si le robot regarde le marqueur de biais) et analyse la grille interne.
4. **Correspondance et ID :** Il lit le motif binaire (les cases noires et blanches) et le compare au **Dictionnaire**. S'il trouve une correspondance parfaite (en tenant compte de la correction d'erreurs), le marqueur est validé.

### D. Le Résultat : Coins et ID
Quand la fonction `detectMarkers()` réussit, elle te renvoie deux choses essentielles pour ton robot :
1. **L'ID unique :** Le numéro du marqueur (ex: "J'ai trouvé le marqueur 42"). Cela permet au robot de savoir *ce qu'il regarde* (ex: ID 42 = station de recharge, ID 15 = boîte à saisir).
2. **Les 4 coins en pixels :** Les coordonnées $(x, y)$ des quatre coins du carré dans l'image. Cela permet de savoir *où est* l'objet et, par la suite, de calculer sa distance et son orientation en 3D (ce qu'on appelle l'estimation de pose ou *Pose Estimation*).

---

## 3. Exemple de Code C++ (OpenCV)

Voici un exemple minimaliste de détection. En robotique, OpenCV est le standard de l'industrie pour la vision.

```cpp
#include <opencv2/opencv.hpp>
#include <opencv2/aruco.hpp>
#include <iostream>
#include <vector>

int main() {
    // 1. Charger une image en niveaux de gris (MONO8)
    // IMREAD_GRAYSCALE force la conversion en 1 canal (0-255)
    cv::Mat image = cv::imread("robot_view.jpg", cv::IMREAD_GRAYSCALE);
    if(image.empty()) {
        std::cerr << "Erreur: Image non trouvée." << std::endl;
        return -1;
    }

    // 2. Définir le dictionnaire utilisé dans l'environnement du robot
    // Ici, un dictionnaire avec des grilles de 4x4, contenant 50 marqueurs possibles
    cv::Ptr<cv::aruco::Dictionary> dictionary = cv::aruco::getPredefinedDictionary(cv::aruco::DICT_4X4_50);

    // 3. Préparer les structures pour récupérer les résultats
    // 'corners' contiendra une liste de listes de 4 points (x,y) pour chaque marqueur détecté
    std::vector<std::vector<cv::Point2f>> corners; 
    // 'ids' contiendra les numéros uniques des marqueurs détectés
    std::vector<int> ids; 

    // 4. Lancer la détection
    cv::aruco::detectMarkers(image, dictionary, corners, ids);

    // 5. Exploiter les résultats
    if (ids.size() > 0) {
        std::cout << "Génial ! " << ids.size() << " marqueur(s) détecté(s)." << std::endl;
        
        for (size_t i = 0; i < ids.size(); i++) {
            std::cout << "Marqueur ID: " << ids[i] << " trouvé aux coordonnées :" << std::endl;
            std::cout << " - Coin Haut-Gauche : (" << corners[i][0].x << ", " << corners[i][0].y << ")" << std::endl;
            std::cout << " - Coin Haut-Droite : (" << corners[i][1].x << ", " << corners[i][1].y << ")" << std::endl;
            std::cout << " - Coin Bas-Droite  : (" << corners[i][2].x << ", " << corners[i][2].y << ")" << std::endl;
            std::cout << " - Coin Bas-Gauche  : (" << corners[i][3].x << ", " << corners[i][3].y << ")" << std::endl;
        }

        // Bonus : Dessiner les contours et l'ID sur l'image pour visualiser (pour le debug)
        cv::Mat image_color;
        cv::cvtColor(image, image_color, cv::COLOR_GRAY2BGR); // Repasser en couleur juste pour l'affichage visuel
        cv::aruco::drawDetectedMarkers(image_color, corners, ids);
        cv::imshow("Vision du Robot", image_color);
        cv::waitKey(0);
    } else {
        std::cout << "Aucun marqueur détecté dans le champ de vision." << std::endl;
    }

    return 0;
}
```
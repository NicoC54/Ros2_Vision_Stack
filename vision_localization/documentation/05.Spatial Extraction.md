Jusqu'à présent, nous avons fait de la manipulation de pixels. Notre masque propre est une matrice où le fond est rempli de 0 (noir) et notre balle rouge de 255 (blanc).

Pour toi, humain, il est évident qu'il y a un rond blanc au milieu de l'écran. Mais pour le robot, cela reste une simple grille de nombres. Si on veut qu'il suive la balle, on ne peut pas lui envoyer une grille entière ; on doit lui fournir une **coordonnée spatiale (X, Y)** qui indique précisément où se trouve le centre de la cible.

C'est l'objectif de l'extraction spatiale : passer du monde des pixels (l'image) au monde géométrique (les coordonnées).

Voici comment les idées s'enchaînent pour y parvenir.

### Étape 1 : Détecter la géométrie avec les Contours

La première étape consiste à tracer une frontière numérique autour de notre forme blanche.
Un **contour**, en vision par ordinateur, est une liste de points (des coordonnées X, Y) qui décrivent le périmètre continu d'un objet.

On utilise la fonction OpenCV `cv::findContours`. Elle va scanner notre masque binaire de haut en bas et de gauche à droite. Dès qu'elle détecte une transition entre un pixel noir (fond) et un pixel blanc (objet), elle longe cette bordure pour l'enregistrer. L'algorithme nous renvoie alors un vecteur de contours (une liste de listes de points).

### Étape 2 : Isoler la cible avec l'Aire

Même après un bon nettoyage morphologique, il arrive qu'une petite tache blanche isolée (par exemple, le reflet d'un autre objet rouge) survive dans un coin de l'image. `cv::findContours` va donc trouver *plusieurs* formes géométriques. Comment dire au robot laquelle est la vraie balle ?

On utilise le concept d'**Aire** (Area). L'aire d'un contour est simplement le nombre de pixels contenus à l'intérieur de cette frontière.
Grâce à la fonction `cv::contourArea`, on calcule la surface de chaque forme détectée. La règle métier est alors très simple : la balle que l'on cherche à suivre est forcément l'objet rouge le plus gros à l'écran. On trie nos contours et on ne conserve *que* celui qui a la plus grande aire, ignorant instantanément tout le bruit résiduel.

### Étape 3 : Trouver le centre de masse avec les Moments

Maintenant que nous avons la frontière de notre balle (notre plus grand contour), nous devons trouver son point central pour diriger le robot.

On utilise un outil mathématique appelé **Moment d'image** (`cv::moments`).
Imagine que tu découpes la forme blanche de ton masque dans un morceau de carton. Le "moment" sert à calculer le **centre de gravité** (ou centre de masse) de ce bout de carton. C'est le point d'équilibre exact où tu pourrais faire tenir la forme sur la pointe d'une aiguille.

En mathématiques, les moments d'une forme calculent sa répartition spatiale. OpenCV nous renvoie plusieurs valeurs, dont :

* $M_{00}$ : L'aire exacte de l'objet (son "poids").
* $M_{10}$ et $M_{01}$ : La somme des positions de tous les pixels sur les axes X et Y, pondérée par leur intensité.

Pour obtenir le centre de gravité (les coordonnées finales X et Y de notre cible), la connexion mathématique est une simple division :

* $X = \frac{M_{10}}{M_{00}}$
* $Y = \frac{M_{01}}{M_{00}}$

Nous avons maintenant un point (X, Y) unique et parfait. C'est cette donnée ultra-légère (deux nombres entiers) que tu vas publier sur un topic ROS 2 pour ordonner aux moteurs du robot de s'aligner.

---

## Fiche de cours : Extraction Spatiale (Contours et Moments)

### 1. Concepts Fondamentaux

L'extraction spatiale est l'étape charnière entre la *Perception* (comprendre l'image) et le *Contrôle* (agir sur le monde). Son but est de convertir un objet délimité par des pixels (masque binaire) en un point géométrique cartésien (X, Y) utilisable par la cinématique du robot.

### 2. Algorithmique : Le Pipeline d'Extraction

1. **Détourage (`cv::findContours`) :**
* L'algorithme analyse le masque binaire et extrait les frontières topologiques des objets.
* *Configuration standard :* On utilise souvent le mode `cv::RETR_EXTERNAL` pour ne récupérer que les contours extérieurs (ignorer les trous à l'intérieur des objets si la fermeture morphologique n'a pas été parfaite).


2. **Filtrage spatial (`cv::contourArea`) :**
* Un algorithme robuste ne suppose jamais qu'il n'y a qu'un seul objet détecté.
* L'itération (boucle `for`) sur tous les contours permet d'évaluer leur aire.
* Le contour possédant la valeur d'aire maximale est défini comme la cible (Target), le reste est écarté comme bruit (Noise).


3. **Calcul du centre de gravité (`cv::moments`) :**
* Application des formules physiques de barycentre à une matrice 2D.
* Calcul des coordonnées cartésiennes : $Centre_{X} = M_{10} / M_{00}$ et $Centre_{Y} = M_{01} / M_{00}$.
* *Sécurité logicielle :* Il faut toujours s'assurer que $M_{00}$ (l'aire) est strictement supérieur à 0 avant de diviser, pour éviter un crash fatal du programme (Division par zéro).



### 3. Implémentation C++

```cpp
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

int main() {
    // Simulation : Chargement direct d'un masque propre (remplace ton code précédent)
    // Assure-toi d'avoir un masque binaire valide pour tester
    cv::Mat masque_propre = cv::imread("masque_propre.png", cv::IMREAD_GRAYSCALE);
    cv::Mat image_rendu;
    cv::cvtColor(masque_propre, image_rendu, cv::COLOR_GRAY2BGR); // Pour dessiner en couleur

    if (masque_propre.empty()) return -1;

    // 1. Détection des contours
    // On crée une liste (std::vector) qui contiendra des listes de points (std::vector<cv::Point>)
    std::vector<std::vector<cv::Point>> contours;
    
    // RETR_EXTERNAL = Prendre uniquement les bords externes
    // CHAIN_APPROX_SIMPLE = Compresser les segments droits pour économiser de la mémoire
    cv::findContours(masque_propre, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    // 2. Filtrage : Trouver le plus grand contour
    double aire_max = 0;
    int index_plus_grand_contour = -1;

    for (size_t i = 0; i < contours.size(); i++) {
        double aire_actuelle = cv::contourArea(contours[i]);
        if (aire_actuelle > aire_max) {
            aire_max = aire_actuelle;
            index_plus_grand_contour = i;
        }
    }

    // 3. Calcul du centre (si au moins un contour a été trouvé)
    if (index_plus_grand_contour != -1) {
        
        // Extraction des moments mathématiques du plus grand contour
        cv::Moments m = cv::moments(contours[index_plus_grand_contour]);

        // Sécurité : Éviter la division par zéro si l'objet est infiniment petit
        if (m.m00 > 0) {
            // Formule du centre de masse
            int centre_x = m.m10 / m.m00;
            int centre_y = m.m01 / m.m00;

            std::cout << "Cible trouvee aux coordonnees : X=" << centre_x << ", Y=" << centre_y << std::endl;

            // Dessin visuel sur l'image pour vérifier
            // Dessiner le contour géométrique en bleu
            cv::drawContours(image_rendu, contours, index_plus_grand_contour, cv::Scalar(255, 0, 0), 2);
            
            // Dessiner un cercle vert de rayon 5 au centre de masse
            cv::circle(image_rendu, cv::Point(centre_x, centre_y), 5, cv::Scalar(0, 255, 0), -1);
        }
    } else {
        std::cout << "Aucune cible détectée à l'écran." << std::endl;
    }

    cv::imshow("Resultat du tracking", image_rendu);
    cv::waitKey(0);

    return 0;
}

```

---

Nous avons franchi la frontière entre le traitement d'image et la vraie robotique.

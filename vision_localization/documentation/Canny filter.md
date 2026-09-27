# Sujet 14. Filtrage spatial et Détection de contours purs

**Prérequis :** Le principe mathématique de la convolution.
**Objectif :** Maîtriser le lissage pour la réduction du bruit, l'extraction de gradients avec Sobel, et la création de contours nets avec l'algorithme de Canny.

## 1. La Convolution (Le Moteur)
La majorité des traitements spatiaux en vision par ordinateur repose sur la **convolution**. 
Un **noyau (Kernel)**, qui est une petite matrice (ex: 3x3, 5x5), glisse sur l'image originale. La valeur de chaque nouveau pixel est calculée en faisant la somme des multiplications entre les pixels locaux et les coefficients du noyau.

## 2. Le Lissage : Flou Gaussien (`cv::GaussianBlur`)
*   **Objectif :** Éliminer le bruit du capteur (le grain) et lisser les micro-textures (bruit haute fréquence).
*   **Pourquoi est-ce crucial ?** Les détecteurs de contours sont basés sur les variations d'intensité (les dérivées). Sans lissage préalable, le moindre grain de poussière génère une variation et crée un "faux contour" qui pollue l'image.
*   **Fonctionnement :** Le noyau gaussien attribue un poids fort au pixel central, et des poids décroissants aux pixels environnants selon une courbe de Gauss.

## 3. Les Gradients : Filtre de Sobel (`cv::Sobel`)
Un contour est une transition brutale d'intensité (ex: passage du noir de l'ombre au blanc d'un objet). Sobel calcule cette dérivée spatiale (le gradient).

*   **Sobel X (Dérivée par rapport à X) :** Détecte les variations horizontales, donc il met en surbrillance les **contours verticaux**.
*   **Sobel Y (Dérivée par rapport à Y) :** Détecte les variations verticales, donc il met en surbrillance les **contours horizontaux**.

Pour obtenir la magnitude totale du contour (la force du bord), on combine généralement X et Y : $G = \sqrt{G_x^2 + G_y^2}$.

## 4. L'algorithme de Canny (`cv::Canny`)
C'est le détecteur de contours le plus populaire et le plus performant. Il enchaîne 4 étapes pour produire des contours propres, continus et fins.

1.  **Flou Gaussien :** Nettoyage automatique du bruit.
2.  **Calcul des gradients :** Utilisation de Sobel pour trouver l'intensité et la direction des contours.
3.  **Suppression des non-maxima (Affinage) :** Parcours des crêtes de gradients pour ne conserver que la valeur maximale locale. Le contour passe d'une "tache" floue à une ligne parfaite de **1 pixel d'épaisseur**.
4.  **Seuillage par hystérésis :** Utilisation de deux seuils (bas et haut).
    *   Pixels > Seuil Haut = Conservés (Contours forts).
    *   Pixels < Seuil Bas = Supprimés.
    *   Entre les deux = Conservés **uniquement** s'ils sont physiquement connectés à un contour fort (permet de combler les trous).

---
## 💻 Exemples de Code C++ (OpenCV)

```cpp
#include <opencv2/opencv.hpp>
#include <iostream>

void processImage(const cv::Mat& image) {
    // Il est toujours recommandé de convertir l'image en niveaux de gris
    // avant de chercher des contours (on cherche une géométrie, pas des couleurs)
    cv::Mat gray_image;
    cv::cvtColor(image, gray_image, cv::COLOR_BGR2GRAY);

    // ---------------------------------------------------------
    // 1. Lissage (Flou Gaussien)
    // ---------------------------------------------------------
    cv::Mat blurred;
    // (5,5) est la taille du noyau (doit être impair). 
    // 0 est l'écart-type (calculé automatiquement par OpenCV)
    cv::GaussianBlur(gray_image, blurred, cv::Size(5, 5), 0);

    // ---------------------------------------------------------
    // 2. Gradients de Sobel
    // ---------------------------------------------------------
    cv::Mat grad_x, grad_y;
    cv::Mat abs_grad_x, abs_grad_y, sobel_combined;

    // Calcul de la dérivée en X (Contours Verticaux)
    // CV_16S : On utilise du 16 bits signé car une dérivée peut être négative !
    cv::Sobel(blurred, grad_x, CV_16S, 1, 0, 3); // dx=1, dy=0, noyau=3x3
    cv::convertScaleAbs(grad_x, abs_grad_x);     // Reconversion en 8 bits absolu

    // Calcul de la dérivée en Y (Contours Horizontaux)
    cv::Sobel(blurred, grad_y, CV_16S, 0, 1, 3); // dx=0, dy=1, noyau=3x3
    cv::convertScaleAbs(grad_y, abs_grad_y);

    // Combinaison des deux (Approximation mathématique rapide de la magnitude)
    cv::addWeighted(abs_grad_x, 0.5, abs_grad_y, 0.5, 0, sobel_combined);

    // ---------------------------------------------------------
    // 3. Algorithme de Canny
    // ---------------------------------------------------------
    cv::Mat canny_edges;
    double threshold_low = 50.0;
    double threshold_high = 150.0;
    
    // Canny fait le Sobel, l'affinage et l'hystérésis tout seul !
    // Note : Il est recommandé de passer l'image DÉJÀ floutée à Canny
    cv::Canny(blurred, canny_edges, threshold_low, threshold_high);

    // Les résultats peuvent être affichés avec cv::imshow()
}
```
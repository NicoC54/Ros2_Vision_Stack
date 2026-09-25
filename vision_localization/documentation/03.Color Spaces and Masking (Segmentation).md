# Le Pipeline de Tracking 2D : Espaces Colorimétriques et Masquage

Pour qu'un robot puisse interagir avec son environnement (par exemple, attraper une balle rouge), il doit d'abord pouvoir "voir" et "isoler" cet objet dans l'image que lui envoie sa caméra. C'est ce qu'on appelle la **Segmentation**.

## 1. Comment un robot voit-il une image ? (Le concept de Pixel)

Avant de parler de couleurs, il faut comprendre le support de base. 

*   **Terme technique - Pixel :** Un pixel (Picture Element) est le plus petit point lumineux qui compose une image numérique. 
*   **Terme technique - Image numérique :** Pour un ordinateur (et donc un robot), une image n'est rien d'autre qu'une immense grille (une matrice) remplie de pixels.

Chaque pixel possède une valeur numérique qui indique sa couleur. 

## 2. Le standard par défaut : L'espace RGB (Rouge, Vert, Bleu)

Par défaut, les caméras et les écrans fonctionnent avec un système appelé RGB (ou BGR dans OpenCV, ce qui est exactement la même chose mais dans l'ordre inverse : Bleu, Vert, Rouge).

*   **Terme technique - Espace colorimétrique :** C'est une méthode mathématique pour représenter les couleurs sous forme de nombres. 
*   **Le fonctionnement du RGB :** Dans l'espace RGB, chaque pixel est défini par un mélange de trois couleurs primaires de lumière : le Rouge, le Vert et le Bleu. L'intensité de chaque couleur primaire varie généralement de 0 à 255. 
    *   Exemple : Un rouge pur sera (R:255, G:0, B:0).
    *   Exemple : Du blanc sera (R:255, G:255, B:255) car c'est le mélange de toutes les lumières.

### Le problème du RGB en robotique

Imagine que tu veuilles programmer ton robot pour qu'il suive un objet "Rouge pur". Tu lui dis de chercher les pixels dont la valeur est environ (R:255, G:0, B:0).
Tout fonctionne parfaitement dans ton laboratoire bien éclairé.

Soudain, un nuage passe devant le soleil, ou le robot entre dans une zone d'ombre. Ton objet rouge pur devient beaucoup plus sombre. La caméra capte moins de lumière. La valeur du pixel tombe par exemple à (R:100, G:0, B:0). 

**La connexion des idées :** En RGB, si la luminosité change, les valeurs des trois canaux (Rouge, Vert, Bleu) changent en même temps. La couleur et la lumière sont complètement mélangées. Pour un robot, il devient mathématiquement très difficile de comprendre que le (R:255) du soleil et le (R:100) de l'ombre représentent *le même objet rouge*.

## 3. La solution : L'espace colorimétrique HSV

Pour résoudre ce problème de lumière, les ingénieurs en vision par ordinateur utilisent une astuce mathématique : ils convertissent l'image de l'espace RGB vers un nouvel espace appelé **HSV**.

HSV signifie : **Hue, Saturation, Value** (Teinte, Saturation, Valeur). 
Définissons ces trois termes cruciaux :

1.  **Hue (Teinte) :** C'est la couleur pure en elle-même, la position sur l'arc-en-ciel. Est-ce que c'est du rouge, du bleu, du jaune ? La teinte se moque totalement de savoir s'il fait sombre ou clair.
2.  **Saturation :** C'est l'intensité ou la "pureté" de la couleur. Est-ce que c'est un rouge vif (saturation haute) ou un rouge délavé, presque grisâtre (saturation basse) ?
3.  **Value (Valeur) :** C'est la luminosité pure. Est-ce que le pixel est éclairé (valeur haute) ou plongé dans le noir (valeur basse) ?

### Pourquoi le HSV est-il magique pour les robots ?

**La connexion des idées :** Le grand pouvoir du HSV, c'est qu'il **sépare l'information de couleur (la Teinte) de l'information d'éclairage (la Valeur).**

Reprenons notre objet rouge. Qu'il soit en plein soleil ou dans l'ombre, sa *Teinte* restera exactement la même (le rouge). Seule sa *Valeur* va diminuer dans l'ombre. 
Ainsi, pour programmer ton robot, tu n'as plus qu'à lui dire : "Cherche la Teinte rouge, et ce, peu importe si la Valeur de luminosité est forte ou faible !". Le robot devient alors extrêmement robuste aux changements d'éclairage.

## 4. Le Masquage (La Segmentation)

Maintenant que notre image est dans un format compréhensible (HSV), nous devons physiquement séparer l'objet du fond de l'image.

*   **Terme technique - Segmentation :** C'est l'action de découper une image en plusieurs morceaux distincts (ici : l'objet qui nous intéresse vs tout le reste).
*   **Terme technique - Masque Binaire :** C'est une image spéciale qui ne contient que deux couleurs : du noir et du blanc. Le système binaire ne comprend que des 0 (Noir) et des 1 (Blanc).

### Le processus de Masquage (la fonction `inRange`)

Pour isoler notre objet, nous allons utiliser une méthode de "filtrage par seuil" (Thresholding).

1.  Nous définissons une "fourchette" de couleurs (une limite basse et une limite haute) en HSV. Par exemple : "Toutes les teintes entre le rouge clair et le rouge foncé".
2.  L'ordinateur va analyser chaque pixel de l'image HSV un par un.
3.  **Si** la couleur du pixel tombe dans notre fourchette, on le colorie en **Blanc** sur une nouvelle image vide (le Masque). Le blanc signifie : "C'est l'objet, je le garde".
4.  **Si** la couleur du pixel est en dehors de la fourchette (le mur derrière, le sol, etc.), on le colorie en **Noir** sur le Masque. Le noir signifie : "C'est le décor, je l'ignore".

À la fin de ce processus, tu obtiens une image en noir et blanc (le fameux masque binaire). La silhouette de ton objet rouge sera parfaitement découpée en blanc sur un fond totalement noir. Le robot sait maintenant exactement quels pixels appartiennent à sa cible !

---

## 5. Exemples de code en C++ (OpenCV)

Voici comment on traduit toute cette théorie en quelques lignes de code C++ avec la bibliothèque OpenCV.

```cpp
#include <opencv2/opencv.hpp>
#include <iostream>

int main() {
    // 1. Lire l'image d'origine (avec le chemin absolu pour éviter les soucis d'exécution)
    cv::Mat image_bgr = cv::imread("/home/nico/vision_ws/src/ros2_vision_stack/vision_localization/balle_rouge.jpg");

    if (image_bgr.empty()) {
        std::cerr << "Erreur: Image introuvable !" << std::endl;
        return -1;
    }

    // 2. Créer une variable et convertir l'image de BGR vers HSV
    cv::Mat image_hsv;
    cv::cvtColor(image_bgr, image_hsv, cv::COLOR_BGR2HSV);

    // 3. Définir les deux plages pour le rouge (car la teinte rouge coupe le 0)
    // Plage 1 : Le rouge au tout début du cylindre (0 à 10)
    cv::Scalar rouge_bas_1(0, 120, 70);
    cv::Scalar rouge_haut_1(10, 255, 255);

    // Plage 2 : Le rouge à la toute fin du cylindre (160 à 179)
    cv::Scalar rouge_bas_2(160, 120, 70);
    cv::Scalar rouge_haut_2(179, 255, 255);

    // 4. Créer les variables pour stocker nos masques
    cv::Mat masque1, masque2, masque_final;

    // 5. Appliquer la segmentation (inRange) pour les deux moitiés
    cv::inRange(image_hsv, rouge_bas_1, rouge_haut_1, masque1);
    cv::inRange(image_hsv, rouge_bas_2, rouge_haut_2, masque2);

    // 6. Fusionner les deux masques en un seul
    // "bitwise_or" signifie : Si le pixel est blanc dans masque1 OU dans masque2, 
    // mets-le en blanc dans masque_final.
    cv::bitwise_or(masque1, masque2, masque_final);

    // 7. Afficher les résultats dans des fenêtres pour vérifier
    cv::imshow("Image Originale (BGR)", image_bgr);
    cv::imshow("Masque Rouge Final", masque_final);

    // Attendre que l'utilisateur appuie sur une touche pour fermer
    cv::waitKey(0);

    return 0;
}

```
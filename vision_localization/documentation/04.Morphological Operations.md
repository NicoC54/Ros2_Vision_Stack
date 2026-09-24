Le masque binaire (noir et blanc) que nous avons obtenu à l'étape précédente n'est jamais parfait dans le monde réel. Si ton robot regarde une balle rouge, il y aura peut-être un reflet blanc du soleil sur la balle (ce qui créera un "trou" noir dans ton masque) ou un petit câble rouge en arrière-plan (ce qui créera un "bruit" blanc parasite).

Si on donne cette image brute à l'algorithme de tracking, le robot va trembler ou hésiter entre plusieurs cibles. C'est ici qu'intervient le **nettoyage morphologique**.

Le terme "morphologique" signifie simplement que l'on va modifier l'image en se basant sur la **forme** (la morphologie) des objets qui s'y trouvent.

---

## Fiche de cours : Nettoyage Morphologique

### 1. L'outil de base : Le Noyau (Kernel)

Avant de faire la moindre opération, il faut définir un **Élément Structurant** (ou *Kernel* en anglais). Imagine que c'est un petit tampon (par exemple un carré de 5x5 pixels) que l'on va faire glisser sur chaque pixel de notre image, de gauche à droite et de haut en bas, un peu comme une loupe. C'est la taille et la forme de ce tampon qui vont déterminer la force de notre nettoyage.

### 2. Les deux opérations fondamentales

**A. L'Érosion (`cv::erode`) : "Manger les bords"**

* **La règle :** Quand on pose notre tampon sur un pixel, ce pixel ne reste blanc QUE SI *tous* les pixels sous le tampon sont blancs. Si le tampon touche ne serait-ce qu'un seul pixel noir, le pixel central devient noir.
* **L'effet :** Cela "ronge" les contours de notre objet blanc.
* **L'intérêt :** Si on a un tout petit point blanc parasite (du bruit) qui est plus petit que notre tampon, l'érosion va l'effacer complètement ! Le problème, c'est que notre vraie balle rouge va aussi rétrécir un peu.

**B. La Dilatation (`cv::dilate`) : "Gonfler les objets"**

* **La règle :** C'est l'inverse. Quand on pose le tampon, le pixel central devient blanc SI *au moins un* pixel sous le tampon est blanc.
* **L'effet :** Cela dilate, ça fait "gonfler" les objets blancs vers l'extérieur.
* **L'intérêt :** Si notre balle a un petit trou noir au milieu (un reflet), la dilatation va combler ce trou. Le problème, c'est que la balle devient artificiellement plus grosse qu'en réalité.

### 3. La vraie magie : Combiner les opérations

Puisque l'érosion efface les bruits mais rétrécit l'objet, et que la dilatation bouche les trous mais grossit l'objet, l'astuce de génie est de les enchaîner ! C'est ce qu'on appelle l'Ouverture et la Fermeture.

**L'Ouverture (Opening) : Érosion + Dilatation**

* **L'ordre :** On érode d'abord, puis on dilate.
* **Pourquoi faire ça ?** L'érosion va tuer les tout petits bruits parasites en arrière-plan. Une fois qu'ils ont disparu, ils sont morts. Ensuite, on lance la dilatation : la vraie balle (qui avait rétréci) va regonfler pour retrouver sa taille d'origine. Les bruits parasites, eux, ne peuvent pas regonfler puisqu'ils ont été totalement effacés !
* **Conclusion :** Parfait pour supprimer le bruit de fond (les faux positifs).

**La Fermeture (Closing) : Dilatation + Érosion**

* **L'ordre :** On dilate d'abord, puis on érode.
* **Pourquoi faire ça ?** La dilatation va gonfler la balle et refermer le petit trou noir à l'intérieur. Le trou disparaît. Ensuite, on érode : la balle dégonfle pour retrouver sa taille normale. Le trou, lui, ne réapparaît pas.
* **Conclusion :** Parfait pour boucher les trous dans l'objet (les faux négatifs).

---

### Code C++ : Mise en pratique avec OpenCV

Voici comment on implémente ces concepts en C++. La fonction magique s'appelle `cv::morphologyEx` (Morphology Extended).

```cpp
#include <opencv2/opencv.hpp>

int main() {
    // Supposons que 'masque_binaire' est l'image issue de l'étape précédente (cv::inRange)
    cv::Mat masque_binaire = cv::imread("masque_brut.png", cv::IMREAD_GRAYSCALE);
    
    // 1. Créer le noyau (Kernel) : un carré de 5x5 pixels
    // Plus le carré est grand, plus le nettoyage est agressif
    cv::Mat kernel = cv::getStructuringElement(cv::MORPH_RECT, cv::Size(5, 5));
    
    cv::Mat masque_propre;

    // 2. Faire une OUVERTURE (Supprimer les petits bruits blancs autour)
    // MORPH_OPEN = Érosion suivie d'une Dilatation
    cv::morphologyEx(masque_binaire, masque_propre, cv::MORPH_OPEN, kernel);

    // 3. Faire une FERMETURE par-dessus (Boucher les trous noirs dans la balle)
    // MORPH_CLOSE = Dilatation suivie d'une Érosion
    cv::morphologyEx(masque_propre, masque_propre, cv::MORPH_CLOSE, kernel);

    // A la fin de ces deux lignes, masque_propre contient un cercle blanc parfait !
    cv::imshow("Masque Brut", masque_binaire);
    cv::imshow("Masque Nettoye", masque_propre);
    cv::waitKey(0);
    
    return 0;
}

```
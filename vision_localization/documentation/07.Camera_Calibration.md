Imaginons que tu demandes à un robot d'attraper une pomme sur une table. Le robot regarde avec sa caméra et voit la pomme aux coordonnées pixels $(320, 240)$. Mais le bras robotique, lui, ne comprend pas les "pixels". Il a besoin de coordonnées physiques, en millimètres, dans l'espace 3D.

Le problème fondamental de la vision par ordinateur, c'est qu'une caméra est un outil qui écrase notre monde en 3 dimensions pour le transformer en une image plate en 2 dimensions. La **calibration caméra** est le processus mathématique qui permet de comprendre exactement *comment* cette caméra écrase la réalité, afin de pouvoir faire le chemin inverse : relier un pixel de l'image à un rayon lumineux dans le monde réel.

Pour comprendre ce processus, il faut décomposer le voyage d'un point 3D jusqu'au capteur de la caméra.

### Étape 1 : Le Modèle Idéal (Le Sténopé)

Avant de parler de vraies caméras avec des lentilles en verre, on utilise un modèle mathématique parfait appelé le **modèle du sténopé** (Pinhole camera model).
Imagine une boîte noire complètement fermée, avec juste un trou minuscule sur la face avant. La lumière du monde extérieur passe par ce trou et vient frapper le fond de la boîte (le capteur). Puisque la lumière voyage en ligne droite, un point dans l'espace 3D atterrit sur un point précis du capteur 2D.

Dans ce modèle parfait, deux concepts physiques définissent le comportement de la caméra :

1. **La Distance Focale (Focal Length) :** C'est la distance physique entre le trou (la lentille) et le capteur au fond de la boîte. Plus cette distance est grande, plus l'image est zoomée. On la mesure en pixels sur les axes X et Y, ce qui nous donne deux valeurs : $f_x$ et $f_y$.
2. **Le Centre Optique (Principal Point) :** C'est le point exact sur le capteur où frappe le rayon de lumière qui passe tout droit par le centre de la lentille. Idéalement, c'est le centre de l'image en pixels, noté $c_x$ et $c_y$.

Ces quatre valeurs ($f_x, f_y, c_x, c_y$) sont les **Paramètres Intrinsèques**. Ils représentent "l'ADN" de ta caméra. Ils ne changent jamais, que la caméra regarde le ciel ou le sol.

### Étape 2 : La Réalité Imparfaite (La Distorsion)

Le modèle du sténopé est parfait en mathématiques, mais un trou minuscule ne laisse pas passer assez de lumière. Dans le monde réel, on ajoute une lentille en verre pour capter plus de lumière. Mais le verre courbe la lumière, ce qui déforme l'image. C'est la **Distorsion**.

Il y en a deux types :

1. **La Distorsion Radiale :** La courbure de la lentille fait que les lignes droites dans la réalité apparaissent courbées sur l'image (l'effet "Fish-eye" ou "GoPro"). L'erreur est plus forte sur les bords de l'image qu'au centre.
2. **La Distorsion Tangentielle :** Elle se produit lors de la fabrication de la caméra, si la lentille n'est pas parfaitement parallèle au capteur d'image.

Pour corriger cela, on doit trouver les **Coefficients de Distorsion** (généralement 5 valeurs mathématiques notées $k_1, k_2, p_1, p_2, k_3$). Une fois qu'on les connaît, on peut "redresser" l'image mathématiquement.

### Étape 3 : La Position dans le Monde (Les Extrinsèques)

Maintenant que l'on sait comment la caméra déforme l'image (Intrinsèques) et comment corriger ses défauts (Distorsion), il reste un élément : où est placée la caméra dans la pièce ?
C'est le rôle des **Paramètres Extrinsèques**. Ils décrivent la **Rotation** (l'inclinaison de la caméra) et la **Translation** (sa position X, Y, Z) par rapport à un point de référence dans le monde. Contrairement aux intrinsèques, ces paramètres changent dès que tu bouges la caméra.

### Étape 4 : Comment on trouve toutes ces valeurs ? (La Calibration)

Pour calculer ces paramètres inconnus, l'ordinateur a besoin de repères absolus. Il faut lui montrer un objet dont on connaît *parfaitement* la géométrie 3D, pour voir comment la caméra le déforme en 2D.

L'objet standard est un **damier (Checkerboard)**.
Pourquoi un damier ?

* Il a des coins très nets (l'intersection entre les cases noires et blanches) que les algorithmes détectent très facilement.
* On sait qu'un damier est parfaitement plat (Z = 0).
* On connaît la taille exacte de chaque case (par exemple, 25 millimètres). On a donc les vraies coordonnées 3D de chaque coin.

**Le processus se déroule ainsi :**

1. Tu prends une vingtaine de photos de ce damier sous plein d'angles différents.
2. Pour chaque photo, l'algorithme trouve les coins en pixels 2D sur l'image.
3. L'algorithme connaît les vraies coordonnées 3D de ces coins sur le damier.
4. L'ordinateur résout un système d'équations complexe (souvent l'algorithme de Zhang) pour trouver la matrice intrinsèque et les coefficients de distorsion qui expliquent le mieux comment ces points 3D sont devenus ces pixels 2D.

---

# Fiche de Cours : Calibration de Caméra

## 1. Objectif de la Calibration

Le but de la calibration est de trouver la relation mathématique entre les coordonnées 3D d'un point dans l'espace physique (en mètres) et ses coordonnées 2D sur le capteur de la caméra (en pixels). Cela permet de :

1. Corriger les déformations visuelles causées par l'objectif.
2. Faire des mesures métriques de l'environnement depuis une image.

## 2. Le Modèle de Caméra (Sténopé)

La projection d'un point 3D $P(X,Y,Z)$ vers un pixel 2D $p(u,v)$ est régie par la **Matrice Intrinsèque** (notée $K$).

$$\begin{bmatrix} u \\ v \\ 1 \end{bmatrix} = \begin{bmatrix} f_x & 0 & c_x \\ 0 & f_y & c_y \\ 0 & 0 & 1 \end{bmatrix} \begin{bmatrix} X \\ Y \\ Z \end{bmatrix}$$

* **$f_x, f_y$** : Les distances focales exprimées en pixels (mise à l'échelle).
* **$c_x, c_y$** : Les coordonnées en pixels du centre optique (généralement proche du centre de l'image).
* *Note :* La matrice $K$ est interne à la caméra et indépendante de sa position.

## 3. La Distorsion (Les Défauts de l'Objectif)

Les lentilles introduisent des erreurs non linéaires qu'il faut compenser.

* **Distorsion Radiale :** Due à la courbure sphérique des lentilles. Les lignes droites deviennent courbes. Modélisée par les coefficients $k_1, k_2, k_3$.
* **Distorsion Tangentielle :** Due à un défaut d'alignement (non-parallélisme) entre la lentille et le plan du capteur d'image. Modélisée par $p_1, p_2$.

Vecteur de distorsion généré par OpenCV : `[k1, k2, p1, p2, k3]`

## 4. Paramètres Extrinsèques

Ils définissent la pose (position et orientation) de la caméra par rapport au repère du monde réel.

* **Matrice de Rotation ($R$) :** Orientations (Roll, Pitch, Yaw).
* **Vecteur de Translation ($t$) :** Position dans l'espace (X, Y, Z).

## 5. La Méthode de Calibration (Zhang)

1. **Cible de calibration :** Utilisation d'un motif géométrique connu, typiquement un damier (*Checkerboard*).
2. **Objectpoints (Points 3D) :** Les coordonnées théoriques des intersections du damier dans son propre repère (où $Z=0$).
3. **Imagepoints (Points 2D) :** Les coordonnées en pixels des intersections détectées sur l'image par OpenCV.
4. **Optimisation :** L'algorithme minimise l'erreur de reprojection (la différence entre la position des points projetés par le modèle mathématique et leur position réelle mesurée sur l'image).

---

## 6. Implémentation en C++ (OpenCV)

Voici le code standard pour extraire les paramètres intrinsèques et le vecteur de distorsion à partir d'une liste d'images d'un damier.

```cpp
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>

int main() {
    // 1. Définition de la grille du damier (par ex: 9 intersections par 6)
    cv::Size boardSize(9, 6);
    float squareSize = 0.025f; // Taille d'une case en mètres (25 mm)

    // Stockage des points
    std::vector<std::vector<cv::Point3f>> objectPoints; // Points 3D théoriques
    std::vector<std::vector<cv::Point2f>> imagePoints;  // Points 2D détectés

    // Création du modèle 3D du damier (Z est toujours 0)
    std::vector<cv::Point3f> objPts;
    for (int i = 0; i < boardSize.height; ++i) {
        for (int j = 0; j < boardSize.width; ++j) {
            objPts.push_back(cv::Point3f(j * squareSize, i * squareSize, 0.0f));
        }
    }

    // 2. Boucle sur les images capturées pour trouver les coins
    std::vector<cv::String> imageFiles;
    cv::glob("images_damier/*.jpg", imageFiles); // Charge les images

    cv::Size imageSize;
    for (const auto& file : imageFiles) {
        cv::Mat img = cv::imread(file);
        cv::Mat gray;
        cv::cvtColor(img, gray, cv::COLOR_BGR2GRAY);
        imageSize = gray.size();

        std::vector<cv::Point2f> corners;
        // Trouve les intersections du damier sur l'image
        bool found = cv::findChessboardCorners(gray, boardSize, corners, 
            cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CB_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);

        if (found) {
            // Affine la précision des coins au niveau sous-pixel
            cv::cornerSubPix(gray, corners, cv::Size(11, 11), cv::Size(-1, -1),
                cv::TermCriteria(cv::TermCriteria::EPS + cv::TermCriteria::MAX_ITER, 30, 0.1));
            
            imagePoints.push_back(corners);
            objectPoints.push_back(objPts);
        }
    }

    // 3. Exécution de l'algorithme de Calibration
    cv::Mat cameraMatrix, distCoeffs;
    std::vector<cv::Mat> rvecs, tvecs; // Paramètres extrinsèques pour chaque image

    double rmsError = cv::calibrateCamera(objectPoints, imagePoints, imageSize, 
                                          cameraMatrix, distCoeffs, rvecs, tvecs);

    std::cout << "Erreur RMS : " << rmsError << std::endl;
    std::cout << "Matrice Intrinsèque :\n" << cameraMatrix << std::endl;
    std::cout << "Coefficients Distorsion :\n" << distCoeffs << std::endl;

    // 4. Utilisation : "Redresser" une image déformée (Undistort)
    cv::Mat imageDeformee = cv::imread("images_damier/test.jpg");
    cv::Mat imageCorrigee;
    cv::undistort(imageDeformee, imageCorrigee, cameraMatrix, distCoeffs);

    return 0;
}

```


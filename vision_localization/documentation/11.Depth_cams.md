# Sujet 12. La profondeur matérielle : Vision RGB-D

**Prérequis :** Matrice intrinsèque et pont ROS 2.
**Objectif :** Maîtriser l'alignement spatial, l'extraction de distance, la déprojection mathématique et la structure `sensor_msgs::msg::PointCloud2`.

---

## 1. Les concepts de base : Qu'est-ce que la vision RGB-D ?

*   **RGB (Red, Green, Blue) :** Une image couleur standard. Chaque pixel est composé de trois valeurs d'intensité pour le rouge, le vert et le bleu (généralement codées sur 8 bits, de 0 à 255).
*   **D (Depth - Profondeur) :** Une image où chaque pixel ne contient pas de couleur, mais une **distance**. 
*   **Le matériel :** Les caméras RGB-D (comme Intel RealSense ou OAK-D) utilisent des capteurs distincts. Un objectif capture la couleur, tandis qu'un autre système (stéréovision infrarouge ou laser temps de vol) calcule la profondeur.

## 2. Le problème physique : Le décalage (Parallaxe)

Puisque les capteurs de couleur et de profondeur sont physiquement séparés sur la caméra, ils observent le monde sous un angle légèrement différent. 

Un objet visible au pixel `(x=100, y=100)` sur l'image couleur apparaîtra à des coordonnées légèrement différentes, disons `(x=115, y=102)`, sur l'image de profondeur. C'est ce qu'on appelle la **parallaxe**.

**L'alignement spatial (Registration) :** C'est l'opération mathématique requise pour corriger ce décalage. L'objectif est de s'assurer qu'un pixel de coordonnées `(u, v)` sur l'image couleur corresponde exactement à la même position physique dans le monde réel que le pixel `(u, v)` de l'image de profondeur.

## 3. L'ADN de la caméra : La Matrice Intrinsèque ($K$)

Pour effectuer l'alignement spatial et comprendre la géométrie de l'image, nous utilisons le modèle mathématique du sténopé (Pinhole model). Ce modèle dépend de la **matrice intrinsèque**, qui caractérise l'optique interne spécifique de chaque objectif.

Elle contient deux informations vitales :
1.  **Le centre optique ($c_x, c_y$) :** Le point central réel de l'image, qui diffère souvent du centre géométrique à cause des imperfections d'assemblage.
2.  **La distance focale ($f_x, f_y$) :** La distance entre la lentille et le capteur, exprimée en pixels. Elle dicte le champ de vision (FOV).

### Formules de projection (3D vers 2D)
Pour un point 3D de coordonnées $(X, Y, Z)$ dans l'espace, ses coordonnées en pixels $(u, v)$ sur l'image sont données par :
$$u = \frac{f_x \cdot X}{Z} + c_x$$
$$v = \frac{f_y \cdot Y}{Z} + c_y$$

## 4. Format des données et Extraction de la distance

Une profondeur ne peut pas être stockée comme une simple couleur de 0 à 255. Les images de profondeur utilisent un format **entier non signé de 16 bits** (`uint16_t` en C++, `CV_16U` dans OpenCV).

Cela permet de stocker des distances allant de 0 à 65 535. En robotique, l'unité standard de la valeur brute est le **millimètre**. Ainsi, si un pixel a une valeur de `1500`, l'objet se trouve à exactement 1,5 mètre de la caméra.

## 5. Communication et Architecture ROS 2

Le robot interagit avec ces données via son intergiciel ROS 2. Deux éléments sont fondamentaux :

*   **Le pont `cv_bridge` :** ROS 2 transporte des images sous forme de messages `sensor_msgs::msg::Image`. Pour les analyser et les modifier avec OpenCV (en objets `cv::Mat`), `cv_bridge` assure la traduction d'un format à l'autre.
*   **Le message `sensor_msgs::msg::PointCloud2` :** Plutôt que de traiter deux images séparément, les données RGB-D sont souvent fusionnées en un "nuage de points" 3D.
    *   *Structure :* Ce n'est pas une matrice 2D, mais un immense tableau d'octets unidimensionnel (`data`). 
    *   *Méta-données :* Le champ `fields` indique comment interpréter les octets (ex: les 4 premiers octets sont X, les 4 suivants Y, puis Z, puis les couleurs RGB).

---

## 💻 Exemples de Code C++

### 1. Extraire la distance d'un pixel (OpenCV)

```cpp
#include <opencv2/opencv.hpp>
#include <iostream>

void extractDepth(const cv::Mat& depth_image, int u, int v) {
    if (depth_image.type() != CV_16U) {
        std::cerr << "Format incorrect. CV_16U attendu." << std::endl;
        return;
    }
    // Lecture directe de l'entier 16 bits. OpenCV utilise (ligne, colonne) -> (v, u)
    uint16_t depth_mm = depth_image.at<uint16_t>(v, u);

    if (depth_mm > 0) {
        std::cout << "Distance : " << depth_mm << " mm (" << (depth_mm / 1000.0) << " m)\n";
    }
}
```

### 2. Récupérer l'image depuis ROS 2 avec cv_bridge

```cpp
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <cv_bridge/cv_bridge.h>

void imageCallback(const sensor_msgs::msg::Image::SharedPtr msg) {
    try {
        // Demande une copie encodée en 16 bits Unsigned, 1 Canal (TYPE_16UC1)
        cv_bridge::CvImagePtr cv_ptr = cv_bridge::toCvCopy(msg, sensor_msgs::image_encodings::TYPE_16UC1);
        cv::Mat depth_image = cv_ptr->image;
    } catch (cv_bridge::Exception& e) {
        RCLCPP_ERROR(rclcpp::get_logger("rclcpp"), "Erreur cv_bridge: %s", e.what());
    }
}
```

### 3. La Déprojection : Du pixel (2D) au Point 3D
En utilisant les formules du modèle sténopé, on isole $X$ et $Y$.

```cpp
struct Point3D { float x, y, z; };
struct CameraIntrinsics { float fx, fy, cx, cy; };

Point3D deprojectPixelTo3D(int u, int v, uint16_t depth_mm, const CameraIntrinsics& intrinsics) {
    Point3D point;
    point.z = depth_mm / 1000.0f; // Conversion en mètres pour ROS 2
    
    if (point.z <= 0.0f) return {0.0f, 0.0f, 0.0f};

    // Formules de déprojection : X = (u - cx) * Z / fx
    point.x = (u - intrinsics.cx) * point.z / intrinsics.fx;
    point.y = (v - intrinsics.cy) * point.z / intrinsics.fy;

    return point;
}
```

### 4. Générer un Nuage de Points Coloré
Cette fonction fusionne une image RGB et une Depth Map (déjà alignées) pour créer un environnement 3D.

```cpp
#include <vector>

struct PointXYZRGB { float x, y, z; uint8_t r, g, b; };

std::vector<PointXYZRGB> generatePointCloud(const cv::Mat& rgb, const cv::Mat& depth, const CameraIntrinsics& intr) {
    std::vector<PointXYZRGB> cloud;
    cloud.reserve(rgb.rows * rgb.cols); // Optimisation de la mémoire

    for (int v = 0; v < rgb.rows; ++v) {
        for (int u = 0; u < rgb.cols; ++u) {
            uint16_t d = depth.at<uint16_t>(v, u);
            if (d == 0) continue; // Ignorer les pixels sans données

            // 1. Calculer les coordonnées (X, Y, Z)
            Point3D p3d = deprojectPixelTo3D(u, v, d, intr);

            // 2. Extraire la couleur (OpenCV utilise BGR)
            cv::Vec3b color = rgb.at<cv::Vec3b>(v, u);

            // 3. Assembler le point
            cloud.push_back({p3d.x, p3d.y, p3d.z, color[2], color[1], color[0]});
        }
    }
    return cloud;
}
```
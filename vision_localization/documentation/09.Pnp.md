Imaginez que vous dessinez sur une vitre avec un feutre ce que vous voyez à travers. Si une voiture passe au loin, vous dessinerez un petit rectangle. Si elle est juste devant la vitre, le rectangle sera immense.

C'est exactement ce que fait une caméra : elle écrase un monde en 3 dimensions (3D) sur un capteur plat en 2 dimensions (2D). Le problème en robotique, c'est que le robot n'a que cette image 2D. S'il voit un marqueur ArUco (une sorte de QR code pour robots) qui fait 50 pixels de large sur son image, comment peut-il savoir si c'est un tout petit marqueur situé à 10 centimètres, ou un grand marqueur situé à 3 mètres ?

C'est là qu'interviennent le modèle de la caméra et l'algorithme PnP (Perspective-n-Point). Décomposons cela étape par étape pour que vous maîtrisiez parfaitement la logique avant vos entretiens.

### 1. Le Modèle Caméra (Comment la 3D devient 2D)

Pour que le robot puisse deviner la 3D à partir de la 2D, il doit d'abord comprendre mathématiquement comment sa propre caméra déforme la réalité pour créer l'image. On utilise pour cela le **modèle du sténopé** (pinhole camera en anglais).

Dans ce modèle, la lumière passe par un point unique (le centre optique de la lentille) pour aller frapper le capteur.
Pour lier un point de l'espace 3D à un pixel 2D sur l'image, on utilise une carte d'identité mathématique de la caméra : la **matrice intrinsèque**.

* **Matrice intrinsèque :** C'est une grille de nombres (une matrice 3x3) qui contient les caractéristiques physiques uniques de votre caméra. Elle est composée de deux éléments clés :
* *La distance focale ($f_x, f_y$) :* C'est la distance entre la lentille et le capteur. C'est ce qui détermine le "zoom" naturel de la caméra.
* *Le centre optique ($c_x, c_y$) :* C'est le point central de votre image en pixels. Idéalement c'est le milieu parfait, mais en réalité, les lentilles sont toujours un peu décentrées lors de la fabrication.


* **Coefficients de distorsion :** Les lentilles ne sont pas plates, elles sont bombées (comme un œil de poisson ou un judas de porte). Cela tord l'image sur les bords. On utilise des coefficients (souvent appelés $k_1, k_2, p_1, p_2$) pour redresser mathématiquement l'image avant de faire nos calculs.

**La connexion :** En connaissant la matrice intrinsèque, si je vous donne un point 3D, vous pouvez calculer exactement sur quel pixel 2D il va atterrir. Mais le robot a besoin de faire l'inverse !

### 2. Le problème PnP (Comment la 2D redevient 3D)

C'est ici qu'intervient **PnP (Perspective-n-Point)**.
Le PnP est un algorithme mathématique qui répond à cette question : *"Sachant où se trouvent $n$ points spécifiques sur mon image 2D, et connaissant la forme exacte de l'objet dans la réalité 3D, où est situé cet objet par rapport à ma caméra ?"*

Pour que l'algorithme fonctionne, vous devez lui fournir 4 ingrédients :

1. **Les points 3D de l'objet (Object Points) :** Vous devez connaître la taille réelle de l'objet. Pour un marqueur ArUco de 10 cm, on définit ses 4 coins dans son propre petit repère 3D local (ex: le coin haut-gauche est à $x=-5cm, y=5cm, z=0$).
2. **Les points 2D de l'image (Image Points) :** Les pixels exacts où se trouvent ces 4 coins sur la photo prise par le robot. (Ces points sont trouvés par un algorithme de détection d'image).
3. **La matrice intrinsèque :** Pour que l'algorithme sache comment la caméra déforme les distances.
4. **Les coefficients de distorsion :** Pour corriger l'effet bombé de la lentille.

### 3. Le Résultat : Extrinsèques, tvec et rvec

Quand vous donnez ces 4 ingrédients à l'algorithme PnP (souvent via la fonction `cv::solvePnP` d'OpenCV), il vous crache la solution sous forme de deux vecteurs qui décrivent ce qu'on appelle les paramètres **extrinsèques** (la position extérieure de l'objet) :

* **tvec (Translation Vector) :** Un vecteur de 3 nombres (X, Y, Z). C'est la distance exacte en mètres entre la caméra et le centre du marqueur.
* **rvec (Rotation Vector) :** Un vecteur de 3 nombres qui décrit comment le marqueur est incliné ou tourné par rapport à la caméra (tangage, roulis, lacet).

### 4. Connecter la vision au robot : TF2 Broadcaster

Avoir `tvec` et `rvec` dans votre code C++ est super, mais votre robot ne s'en sert pas encore. Dans l'écosystème ROS 2 (Robot Operating System), les différents organes du robot (roues, bras, caméras, capteurs laser) communiquent leurs positions relatives via un système appelé **TF2 (Transform Framework)**.

* **TF2 Broadcaster :** C'est un composant logiciel dont le seul but est de crier sur le réseau du robot : *"Écoutez tous ! Je sais où se trouve le repère 'marqueur_aruco' par rapport au repère 'camera_link' !"*
On prend donc notre `tvec` (pour la distance) et notre `rvec` (qu'on convertit souvent en quaternion pour éviter des bugs mathématiques liés aux rotations), et on les donne au TF2 Broadcaster. À partir de ce moment, n'importe quel algorithme du robot (par exemple, un algorithme de navigation) peut dire : *"Avance jusqu'au marqueur"*, car l'arbre TF2 connaît le chemin spatial entre les roues du robot et le marqueur.

---

Voici la fiche de cours détaillée et formatée avec le code.

# Fiche de Cours : Modèle Caméra, solvePnP et TF2

## 1. Modèle Caméra et Matrice Intrinsèque

Le modèle sténopé (pinhole) modélise la projection d'un point 3D de l'espace sur un plan image 2D.

**La Matrice Intrinsèque ($K$)** contient les paramètres internes de la caméra (indépendants de sa position dans l'espace) :


$$K = \begin{bmatrix} f_x & 0 & c_x \\ 0 & f_y & c_y \\ 0 & 0 & 1 \end{bmatrix}$$

* **$f_x, f_y$ :** Distances focales exprimées en pixels.
* **$c_x, c_y$ :** Coordonnées du centre optique (point principal) en pixels.

**Distorsion :** Les lentilles réelles induisent des distorsions radiales (effet barillet/coussinet) et tangentielles (si la lentille n'est pas parfaitement parallèle au capteur). Modélisé par les coefficients $D = [k_1, k_2, p_1, p_2, k_3]$.

## 2. Perspective-n-Point (PnP)

**Objectif :** Estimer la pose (position et orientation) d'un objet 3D calibré à partir de sa projection 2D sur une image.

**Entrées requises pour `cv::solvePnP` :**

1. `objectPoints` : Coordonnées 3D des points de l'objet dans le repère local de l'objet (ex: les 4 coins d'un marqueur ArUco de taille connue, avec $Z=0$ s'il est plat).
2. `imagePoints` : Coordonnées 2D (pixels) des projections de ces points sur l'image.
3. `cameraMatrix` : La matrice intrinsèque $K$.
4. `distCoeffs` : Le vecteur de distorsion $D$.

**Sorties : Paramètres Extrinsèques**

* `tvec` (Vecteur de translation) : Vecteur $[x, y, z]$ représentant la position de l'origine du repère de l'objet dans le repère de la caméra.
* `rvec` (Vecteur de rotation) : Représentation d'axe-angle (vecteur de Rodrigues) de l'orientation de l'objet. La norme du vecteur est l'angle de rotation, sa direction est l'axe de rotation.

## 3. Intégration ROS 2 avec TF2

Le calcul de pose est inutile si le reste du robot l'ignore. ROS 2 utilise **TF2** pour gérer un arbre temporel des repères (frames) du robot.

* **TransformStamped :** Le message standard qui contient le nom du repère parent (`camera_color_optical_frame`), le repère enfant (`aruco_marker_1`), la translation (issue de `tvec`), et la rotation (convertie de `rvec` vers un Quaternion).
* **TransformBroadcaster :** L'objet ROS 2 qui publie ce message en temps réel.

---

## 4. Exemples de Code C++

### A. Calcul de la pose avec cv::solvePnP

```cpp
#include <opencv2/opencv.hpp>
#include <vector>

void computeArucoPose() {
    // 1. Définition des points 3D de l'objet (Marqueur de 10cm x 10cm)
    // L'origine est au centre du marqueur
    float marker_size = 0.10f; 
    std::vector<cv::Point3f> objectPoints = {
        cv::Point3f(-marker_size/2,  marker_size/2, 0), // Haut gauche
        cv::Point3f( marker_size/2,  marker_size/2, 0), // Haut droite
        cv::Point3f( marker_size/2, -marker_size/2, 0), // Bas droite
        cv::Point3f(-marker_size/2, -marker_size/2, 0)  // Bas gauche
    };

    // 2. Points 2D détectés sur l'image (obtenus via cv::aruco::detectMarkers)
    std::vector<cv::Point2f> imagePoints = {
        cv::Point2f(320, 240),
        cv::Point2f(420, 245),
        cv::Point2f(415, 345),
        cv::Point2f(315, 340)
    };

    // 3. Matrice intrinsèque (Obtenue par calibration préalable)
    cv::Mat cameraMatrix = (cv::Mat_<double>(3,3) << 
        600.0, 0.0, 320.0,
        0.0, 600.0, 240.0,
        0.0, 0.0, 1.0);

    // 4. Coefficients de distorsion (Zéro ici pour simplifier)
    cv::Mat distCoeffs = cv::Mat::zeros(4, 1, CV_64F);

    // 5. Vecteurs de sortie
    cv::Mat rvec, tvec;

    // 6. Exécution de solvePnP
    cv::solvePnP(objectPoints, imagePoints, cameraMatrix, distCoeffs, rvec, tvec);

    std::cout << "Translation (X, Y, Z en mètres) : " << tvec << std::endl;
    std::cout << "Rotation (Vecteur de Rodrigues) : " << rvec << std::endl;
}

```

### B. Publication via TF2 Broadcaster (ROS 2)

```cpp
#include "rclcpp/rclcpp.hpp"
#include "tf2_ros/transform_broadcaster.h"
#include "geometry_msgs/msg/transform_stamped.hpp"
#include "tf2/LinearMath/Quaternion.h"
#include "tf2/LinearMath/Matrix3x3.h"
#include <opencv2/calib3d.hpp>

// Fonction simplifiée appelée après avoir calculé tvec et rvec
void broadcastArucoTF(std::shared_ptr<tf2_ros::TransformBroadcaster> tf_broadcaster, 
                      const cv::Mat& tvec, const cv::Mat& rvec, rclcpp::Time current_time) 
{
    geometry_msgs::msg::TransformStamped t;

    // Métadonnées du TF
    t.header.stamp = current_time;
    t.header.frame_id = "camera_optical_frame"; // Repère parent
    t.child_frame_id = "aruco_marker";          // Repère enfant

    // Assignation de la translation (tvec)
    t.transform.translation.x = tvec.at<double>(0);
    t.transform.translation.y = tvec.at<double>(1);
    t.transform.translation.z = tvec.at<double>(2);

    // Conversion de la rotation (rvec -> Matrice de rotation 3x3 -> Quaternion)
    cv::Mat R;
    cv::Rodrigues(rvec, R); // Convertit le vecteur d'angle-axe en matrice 3x3
    
    tf2::Matrix3x3 tf2_rot(
        R.at<double>(0,0), R.at<double>(0,1), R.at<double>(0,2),
        R.at<double>(1,0), R.at<double>(1,1), R.at<double>(1,2),
        R.at<double>(2,0), R.at<double>(2,1), R.at<double>(2,2)
    );

    tf2::Quaternion q;
    tf2_rot.getRotation(q);

    t.transform.rotation.x = q.x();
    t.transform.rotation.y = q.y();
    t.transform.rotation.z = q.z();
    t.transform.rotation.w = q.w();

    // Publication du TF
    tf_broadcaster->sendTransform(t);
}

```
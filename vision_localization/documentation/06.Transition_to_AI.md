Le pipeline de vision classique (HSV) que tu viens de coder avec brio est redoutable dans un environnement contrôlé, comme une ligne d'assemblage en usine où la lumière ne change jamais. Mais dès que ton robot sort dans le monde réel, ce système s'effondre.

Imagine ton robot cherchant sa balle rouge en extérieur. Un nuage passe devant le soleil : la luminosité chute, le rouge devient bordeaux, ta balle sort de tes seuils HSV, et le robot devient aveugle. Pire encore, si quelqu'un pose une pomme rouge à côté de la balle, ton masque binaire fusionnera les deux objets. Ton algorithme `cv::findContours` trouvera une forme géante et ton centre de masse sera calculé dans le vide entre la pomme et la balle.

### 1. La limite des Heuristiques

Ton code actuel est basé sur des **heuristiques**. Une heuristique est une règle logique stricte codée "en dur" par l'ingénieur, basée sur son observation humaine (ex: "Le rouge est compris entre la teinte 0 et 10").

La limite physique de cette approche, c'est qu'une couleur n'est pas une propriété absolue d'un objet : c'est l'interprétation d'une lumière réfléchie. Une ombre dure, un reflet vif, ou une caméra de mauvaise qualité suffisent à briser la règle heuristique. En entretien technique, c'est l'argument massue pour justifier le passage à l'Intelligence Artificielle.

### 2. Le changement de paradigme : Le Deep Learning et YOLO

Pour survivre dans le monde réel, on abandonne la définition manuelle des couleurs. On utilise le **Deep Learning** (Apprentissage Profond). Au lieu de dire "cherche des pixels rouges", on donne à un algorithme 10 000 photos de balles dans toutes les conditions possibles. Le réseau de neurones va extraire de lui-même les "caractéristiques" (features) d'une balle : sa rondeur, ses ombres typiques, sa texture.

L'algorithme star en robotique s'appelle **YOLO** (You Only Look Once). C'est un modèle de détection d'objets. Contrairement à notre code HSV qui cherche des contours exacts, YOLO divise l'image en grille et prédit directement des **Bounding Boxes** (des boîtes englobantes, c'est-à-dire les coordonnées d'un rectangle autour de la balle) et un score de confiance (ex: "Je suis sûr à 95% que ce rectangle contient une balle").

### 3. Le pont logiciel : `cv::dnn` et le format ONNX

Si tu entraînes ton modèle YOLOv8 en Python avec PyTorch, comment l'exécuter dans ton code C++ ultra-rapide sur le robot ?

C'est ici qu'intervient **ONNX** (Open Neural Network Exchange). C'est un format de fichier universel (l'équivalent du `.pdf` mais pour l'Intelligence Artificielle). Tu exportes ton modèle Python en `.onnx`.

Ensuite, OpenCV possède un module dédié appelé **`cv::dnn`** (Deep Neural Network). Ce module permet de lire un fichier ONNX directement en C++ et de faire passer ton flux vidéo à l'intérieur, sans avoir besoin d'installer des bibliothèques d'IA complexes comme TensorFlow ou PyTorch sur ton robot.

### 4. Le pont matériel : L'accélération via TensorRT sur NVIDIA Jetson

Un réseau de neurones comme YOLO effectue des milliards de multiplications matricielles pour analyser une seule image. Si tu fais tourner `cv::dnn` sur le processeur standard (CPU) de ton robot, tu obtiendras 1 ou 2 images par seconde. Le robot sera incapable de suivre une balle en mouvement.

Il faut un **accélérateur matériel**, généralement un GPU (Carte Graphique). En robotique embarquée, le standard de l'industrie est la famille de cartes **NVIDIA Jetson** (de petits ordinateurs de la taille d'une carte de crédit avec un puissant GPU intégré).

Pour que YOLO tourne à 60 images par seconde sur cette carte, on utilise **TensorRT**. C'est un moteur d'inférence (un optimiseur logiciel) développé par NVIDIA. Il prend ton modèle ONNX, supprime les calculs mathématiques redondants, et fusionne les couches du réseau de neurones pour que le modèle soit taillé sur mesure pour l'architecture physique de la puce Jetson.

---

## Fiche de cours : Transition vers l'IA Embarquée en Robotique

### 1. Limites de la Vision Classique (Heuristiques)

* **Sensibilité à la lumière :** L'espace colorimétrique (HSV, BGR) dépend directement de l'illumination. Les ombres et reflets modifient les valeurs des pixels, rendant les seuils statiques (`cv::inRange`) obsolètes.
* **Absence de contexte sémantique :** Une heuristique de couleur ne différencie pas deux objets de même teinte (ex: un t-shirt rouge en arrière-plan et la cible rouge). L'algorithme des moments calculera un barycentre faussé.

### 2. Deep Learning et Détection d'Objets

* **Modèle de base :** YOLO (You Only Look Once) est privilégié en robotique pour sa capacité à analyser l'image en une seule passe, garantissant une faible latence (temps de réponse).
* **Sortie de l'algorithme :** Au lieu d'un masque binaire de pixels, le modèle retourne une *Bounding Box* $[x_{min}, y_{min}, largeur, hauteur]$, un *Label* (Classe de l'objet) et un *Score de confiance*.

### 3. Intégration C++ et Standardisation

* **ONNX (Open Neural Network Exchange) :** Format de sérialisation indépendant du framework d'entraînement. Permet de transférer un modèle PyTorch (Python) vers un environnement de production C++.
* **`cv::dnn` :** Module natif d'OpenCV permettant le chargement et l'inférence (l'exécution) de modèles Deep Learning, évitant de surcharger les dépendances logicielles du système embarqué.
* **Blob :** Format de données attendu par les réseaux de neurones. La fonction `cv::dnn::blobFromImage` transforme l'image standard (Matrice [Hauteur, Largeur, Canaux]) en un tenseur mathématique normalisé (ex: [1, 3, 640, 640]) et redimensionné pour le modèle.

### 4. Accélération Matérielle Embarquée

* **NVIDIA Jetson :** Ordinateur monocarte (SBC) disposant d'un GPU intégré conçu pour le calcul parallèle à faible consommation énergétique.
* **TensorRT :** SDK d'optimisation développé par NVIDIA. Il réorganise l'architecture du réseau (fusion de couches, quantification de la précision des nombres flottants de 32 bits à 16 bits) pour maximiser le débit (framerate) sur les GPU NVIDIA.

---

### Exemple de Code C++ : Inférence d'un modèle avec `cv::dnn`

*Note : Le traitement (parsing) de la sortie de YOLOv8 est très lourd géométriquement. Ce code se concentre sur l'essentiel attendu en entretien : le chargement du modèle et la préparation de l'image (le Blob).*

```cpp
#include <opencv2/opencv.hpp>
#include <opencv2/dnn.hpp> // Inclusion obligatoire pour le Deep Learning
#include <iostream>

int main() {
    // 1. Charger l'image d'entrée
    cv::Mat image = cv::imread("balle_rouge.jpg");
    if (image.empty()) return -1;

    // 2. Charger le modèle ONNX (Préalablement entraîné)
    // C'est ce fichier qui contient toute "l'intelligence"
    cv::dnn::Net net = cv::dnn::readNetFromONNX("yolov8_custom_balle.onnx");

    // Optionnel mais recommandé : Demander à OpenCV d'utiliser le backend GPU (CUDA) 
    // si l'ordinateur/robot en est équipé
    net.setPreferableBackend(cv::dnn::DNN_BACKEND_CUDA);
    net.setPreferableTarget(cv::dnn::DNN_TARGET_CUDA);

    // 3. Préparer le "Blob"
    // Un réseau de neurones n'accepte pas les images telles quelles. 
    // Il faut la normaliser (mettre les valeurs des pixels entre 0 et 1), 
    // la redimensionner (souvent 640x640 pour YOLO), et réorganiser l'ordre des canaux.
    cv::Mat blob;
    cv::dnn::blobFromImage(image, blob, 1.0/255.0, cv::Size(640, 640), cv::Scalar(), true, false);

    // 4. Inférence (Faire passer l'image dans le réseau)
    net.setInput(blob);
    
    // Le réseau calcule et nous renvoie ses prédictions brutes (un gros tenseur multidimensionnel)
    cv::Mat predictions = net.forward();

    std::cout << "Inference terminee ! Le tenseur de sortie a " << predictions.size[1] << " colonnes." << std::endl;

    // L'étape suivante consisterait à filtrer ces prédictions mathématiques pour récupérer 
    // les coordonnées exactes du rectangle autour de la balle, mais cela dépend de la version précise de YOLO.

    return 0;
}

```

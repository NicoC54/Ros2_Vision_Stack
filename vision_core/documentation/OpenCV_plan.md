# Roadmap : Maîtrise de la Vision Robotique sous ROS 2 & C++

Ce document répertorie le plan de formation complet pour passer de l'optimisation mémoire bas niveau sous OpenCV jusqu'à l'estimation de pose 3D et l'intégration ROS 2, taillé spécifiquement pour réussir des entretiens techniques en robotique.

---

## BLOC 1 : Les Fondations (Mémoire & Pont ROS 2)
*L'objectif ici est de comprendre comment une image existe dans la RAM, comment l'optimiser, et comment l'importer depuis l'écosystème ROS 2.*

### 1. La structure `cv::Mat` et sa gestion mémoire
* **Prérequis :** Pointeurs C++ et pointeurs intelligents (`std::shared_ptr`).
* **Pourquoi commencer par là :** C'est le type de base d'OpenCV. Une mauvaise compréhension entraîne des fuites de RAM ou des chutes de FPS.
* **À maîtriser :**
    * Architecture séparée : Header (en-tête) vs Data Pointer (pointeur de données).
    * Copie superficielle (*Shallow Copy*, `cv::Mat A = B;`) vs Copie profonde (*Deep Copy*, `B.clone()`).
    * Accès rapide aux pixels via les pointeurs (`.ptr<T>(y)`) vs accès sécurisé mais lent (`.at<T>(y, x)`).
    * Optimisation spatiale via les **ROI (Region of Interest)** pour rogner l'image sans allocation mémoire supplémentaire.

### 2. Le pont ROS 2 / OpenCV : `cv_bridge`
* **Prérequis :** Comprendre `cv::Mat` et le système de callbacks de ROS 2.
* **Pourquoi c'est l'étape 2 :** Transformer un message brut `sensor_msgs::msg::Image` reçu d'un capteur en une structure manipulable par OpenCV.
* **À maîtriser :**
    * Le bloc de sécurité `try / catch` pour intercepter les exceptions `cv_bridge::Exception`.
    * La distinction critique : `toCvShare()` (lecture seule, zéro copie RAM) vs `toCvCopy()` (copie complète, indispensable si l'image est modifiée).

---

## BLOC 2 : Le Pipeline de Tracking 2D
*L'objectif est d'isoler un objet complexe dans l'image (ex: une cible colorée) et d'extraire sa position en coordonnées 2D (X, Y) en pixels.*

### 3. Espaces colorimétriques et Masquage (Segmentation)
* **Prérequis :** Manipulation de base d'une `cv::Mat`.
* **Le concept :** Convertir l'image BGR par défaut vers l'espace HSV (`cv::cvtColor`). Le HSV sépare la teinte de la luminosité, rendant le robot robuste aux variations d'éclairage et aux ombres.
* **À maîtriser :** Utiliser `cv::inRange` pour générer un masque binaire (l'objet ciblé devient blanc, le reste devient noir).

### 4. Nettoyage Morphologique
* **Prérequis :** Avoir généré le masque binaire de l'étape précédente.
* **Le concept :** Éliminer le bruit capteur (reflets parasites) et combler les trous à l'intérieur de l'objet pour éviter les mouvements erratiques du robot.
* **À maîtriser :** Opérations d'Érosion et de Dilatation, et plus particulièrement l'**Ouverture** (nettoyage extérieur) et la **Fermeture** (comblement intérieur).

### 5. Extraction Spatiale (Contours et Moments)
* **Prérequis :** Disposer d'un masque binaire propre.
* **Le concept :** Transformer des pixels bruts en coordonnées géométriques exploitables par le système de contrôle.
* **À maîtriser :**
    * `cv::findContours` pour détourer les formes.
    * Filtrage par `cv::contourArea` pour isoler la plus grande cible et ignorer le bruit résiduel.
    * Calcul du centre de masse avec `cv::moments` pour obtenir le point (X, Y) final à publier sur un topic ROS 2.

---

## BLOC 3 : La Vision Robotique 3D
*La compétence clé qui différencie un développeur logiciel classique d'un ingénieur en robotique : passer d'un pixel 2D à une position spatiale 3D réelle.*

### 6. Détection de repères : Marqueurs ArUco
* **Prérequis :** Manipulation d'images en niveaux de gris (`MONO8`).
* **Le concept :** Utiliser des balises visuelles normalisées (similaires à des QR codes) pour obtenir des ancres géométriques ultra-précises (drones, bras manipulateurs).
* **À maîtriser :** Génération de dictionnaires, appel à `detectMarkers()` pour récupérer les 4 coins du tag en pixels et son ID unique.

### 7. Modèle Caméra et PnP (Perspective-n-Point)
* **Prérequis :** Notions d'algèbre linéaire (matrices) et points 2D détectés.
* **Le concept :** Connaissant la taille réelle physique d'un marqueur ArUco et la calibration intrinsèque de la caméra, calculer la distance exacte et l'orientation 3D de l'objet par rapport au robot.
* **À maîtriser :** Compréhension des matrices intrinsèques de la caméra et utilisation de `cv::solvePnP`. Le résultat (vecteurs de Translation et Rotation) alimente directement un `TF2 Broadcaster` sous ROS 2.

---

## BLOC 4 : Culture Générale d'Entretien
*Des notions théoriques et historiques très fréquemment posées lors des quiz techniques en entreprise.*

### 8. Filtrage spatial et Détection de contours purs
* **Prérequis :** Le principe mathématique de la convolution.
* **À maîtriser rapidement :**
    * Le lissage (Flou Gaussien / `cv::GaussianBlur`) : pourquoi lisser une image avant d'extraire des contours (réduction du bruit haute fréquence).
    * Les gradients (Sobel) : extraction mathématique des variations d'intensité lumineuse pour trouver les arêtes.
    * L'algorithme de Canny (`cv::Canny`) : combinaison de Sobel et de seuils par hystérésis pour produire des contours fins et continus.
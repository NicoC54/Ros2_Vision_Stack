# Sujet 13. Traitement des Nuages de Points (PCL)

**Prérequis :** Vision RGB-D et compréhension du message `PointCloud2`.
**Objectif :** Maîtriser l'intégration de la PCL sous ROS 2, le sous-échantillonnage par Voxel Grid et la segmentation géométrique par RANSAC.

## 1. L'intégration de PCL dans ROS 2

Le message `sensor_msgs::msg::PointCloud2` est un format de transport réseau (un train d'octets). Pour appliquer des mathématiques complexes, il faut le convertir en un objet structuré issu de la **Point Cloud Library (PCL)**.

* **Le pont de conversion :** On utilise la bibliothèque `pcl_conversions`.

* **Les types C++ PCL :** On manipule généralement des pointeurs intelligents vers des objets `pcl::PointCloud<pcl::PointXYZ>` (pour le XYZ seul) ou `pcl::PointCloud<pcl::PointXYZRGB>` (si on inclut la couleur). Par convention moderne, on utilise `std::make_shared` pour leur allocation.

## 2. Le Filtrage : Voxel Grid

Un nuage brut est trop dense pour des calculs en temps réel (ex: SLAM ou path planning). Il faut le sous-échantillonner.

* **L'erreur à ne pas faire :** Supprimer des points aléatoirement (Random Downsampling). Cela détruit la densité locale, efface les petits objets et ne gère pas les zones sur-représentées.

* **La solution (Voxel Grid) :**

  1. L'espace 3D est divisé en une grille de cubes 3D (les *voxels*).

  2. Tous les points présents dans un même voxel sont moyennés pour former un unique point (le centroïde).

* **Résultat :** Un nuage beaucoup plus léger dont la répartition des points est parfaitement homogène dans l'espace.

## 3. La Segmentation : RANSAC (RANdom SAmple Consensus)

RANSAC est un algorithme itératif robuste utilisé pour estimer les paramètres d'un modèle mathématique (plan, cylindre, sphère) à partir d'un ensemble de données contenant beaucoup de bruit.

* **Cas d'usage principal :** Isoler le plan du sol ou la surface d'une table pour ensuite détecter les objets posés dessus.

* **Le principe en 4 étapes :**

  1. **Sélection :** Choisir aléatoirement le nombre minimum de points nécessaires pour définir le modèle (3 points pour un plan).

  2. **Modélisation :** Calculer l'équation mathématique du plan passant par ces points.

  3. **Évaluation :** Compter combien d'autres points du nuage se situent à une distance inférieure à un seuil défini (Threshold) de ce plan. Ce sont les *inliers*.

  4. **Itération :** Répéter ce processus $N$ fois et conserver le modèle ayant le plus grand nombre d'*inliers*.

## 💻 Exemples de Code C++ (Modern C++)

### 1. Conversion ROS 2 <-> PCL

```cpp
#include <pcl_conversions/pcl_conversions.h>
#include <sensor_msgs/msg/point_cloud2.hpp>
#include <pcl/point_cloud.h>
#include <pcl/point_types.h>

// 1. Depuis ROS 2 vers PCL (Utilisation de make_shared)
auto cloud = std::make_shared<pcl::PointCloud<pcl::PointXYZ>>();
pcl::fromROSMsg(*ros_msg, *cloud);

// 2. Depuis PCL vers ROS 2 (pour publication)
sensor_msgs::msg::PointCloud2 ros_msg_pub;
pcl::toROSMsg(*cloud, ros_msg_pub);
ros_msg_pub.header.frame_id = "camera_link"; // Toujours re-définir le frame_id !
```

### 2. Le Filtre Voxel Grid

```cpp
#include <pcl/filters/voxel_grid.h>

pcl::PointCloud<pcl::PointXYZ>::Ptr applyVoxelGrid(pcl::PointCloud<pcl::PointXYZ>::Ptr cloud) {
    // Allocation optimisée
    auto cloud_filtered = std::make_shared<pcl::PointCloud<pcl::PointXYZ>>();

    pcl::VoxelGrid<pcl::PointXYZ> voxel_filter;
    voxel_filter.setInputCloud(cloud);
    
    // Définition de la taille du voxel (en mètres). Ici un cube de 2cm x 2cm x 2cm
    voxel_filter.setLeafSize(0.02f, 0.02f, 0.02f); 
    
    // Application du filtre
    voxel_filter.filter(*cloud_filtered);

    return cloud_filtered;
}
```

### 3. Segmentation du sol (RANSAC 3D)

```cpp
#include <pcl/segmentation/sac_segmentation.h>
#include <pcl/filters/extract_indices.h>

void extractGround(pcl::PointCloud<pcl::PointXYZ>::Ptr cloud) {
    // Allocations optimisées
    auto ground_cloud = std::make_shared<pcl::PointCloud<pcl::PointXYZ>>();
    auto obstacles_cloud = std::make_shared<pcl::PointCloud<pcl::PointXYZ>>();

    // Objets pour la segmentation
    auto coefficients = std::make_shared<pcl::ModelCoefficients>();
    auto inliers = std::make_shared<pcl::PointIndices>();
    
    pcl::SACSegmentation<pcl::PointXYZ> seg;
    seg.setOptimizeCoefficients(true);
    // On cherche un modèle de plan
    seg.setModelType(pcl::SACMODEL_PLANE);
    // On utilise la méthode RANSAC
    seg.setMethodType(pcl::SAC_RANSAC);
    // Tolérance d'épaisseur du plan en mètres (ici 1,5 cm)
    seg.setDistanceThreshold(0.015); 
    
    seg.setInputCloud(cloud);
    seg.segment(*inliers, *coefficients); // Exécute l'algorithme

    if (inliers->indices.empty()) {
        std::cerr << "RANSAC n'a pas pu trouver de plan géométrique." << std::endl;
        return;
    }

    // Extraction physique des points pour créer deux nuages distincts
    pcl::ExtractIndices<pcl::PointXYZ> extract;
    extract.setInputCloud(cloud);
    extract.setIndices(inliers);
    
    // 1. Garder les inliers (Le sol)
    extract.setNegative(false);
    extract.filter(*ground_cloud);
    
    // 2. Garder le reste (Les obstacles)
    extract.setNegative(true);
    extract.filter(*obstacles_cloud);
}
```
#include <opencv2/opencv.hpp>
#include <iostream>
#include <vector>


int main(){



    //ETAPES PRECEDENTES : PASSAGE EN HSV PUIS EN MASQUE BINAIRE PUIS OPENING AND CLOSING

cv::Mat image_bgr = cv::imread("/home/nico/vision_ws/src/ros2_vision_stack/vision_localization/balle_rouge.jpg");

    if (image_bgr.empty()){
        std::cerr << "Erreur:Image introuvable !" << std::endl;
        return -1;
    }
    else{
        std::cout << "Image trouvée !" << std::endl;
    }

    cv::Mat image_hsv;

    cv::cvtColor(image_bgr, image_hsv, cv::COLOR_BGR2HSV);

    cv::Scalar limite_basse1(0,120,70);
    cv::Scalar limite_haute1(10,255,255);

    cv::Scalar limite_basse2(160,120,70);
    cv::Scalar limite_haute2(179,250,255);

    cv::Mat masque1, masque2, masque_final;

    cv::inRange(image_hsv, limite_basse1, limite_haute1, masque1);
    cv::inRange(image_hsv, limite_basse2, limite_haute2, masque2);

    cv::bitwise_or(masque1,masque2,masque_final);

    cv::Mat kernel = getStructuringElement(cv::MORPH_RECT, cv::Size(5,5));

    cv::Mat masque_ouverture;
    cv::Mat masque_fermeture;

    cv::morphologyEx(masque_final, masque_ouverture, cv::MORPH_OPEN, kernel);
    cv::imshow("image ouverture",masque_ouverture);

    cv::morphologyEx(masque_ouverture, masque_fermeture, cv::MORPH_CLOSE, kernel);
    cv::imshow("image fermeture",masque_fermeture);

    //NOUVELLE ETAPE : EXTRACTION DE CONTOUR PUIS DETERMINATION DU CENTRE DE LOBJET

    //déclaration d'un vecteur contenant des contours : un vecteur de point : donc contours est un vecteur de vecteur de points.
    std::vector<std::vector<cv::Point>> contours;

    //méthode opencv : findContours(masque binaire manipulé, passage par référence de contours, cv::RETR_EXTERNAL : indique on prend les contours exterieurs, cv::CHAIN_APPROX_SIMPLE : une facon d'optimiser la méthode en ne prenant pas tous les points)
    cv::findContours(masque_fermeture, contours, cv::RETR_EXTERNAL, cv::CHAIN_APPROX_SIMPLE);

    //déclaration d'une aire max et d'un index associé à cette aire max
    double aire_max = 0;
    int index_plus_grand_contour = -1;

    //boucle sur chaque contour présents dans contours ou on calcule l'aire du contour avec cv::contourArea(contours[index]) si un contour obtenu est plus grand que celui deja trouvé: il devient la nouvelle valeur;

    for (size_t i = 0; i<contours.size(); i++){
        double aire_actuelle = cv::contourArea(contours[i]);
        if (aire_actuelle>aire_max){
            aire_max = aire_actuelle;
            index_plus_grand_contour = i;
        }
    }

    //dans le cas ou on a un nouvel index sur contour différent de -1 (si effectivement on a trouvé un contour) : on détermine le moment de ce plus grand contour
    // cv::Moments m c'est la déclaration d'un moment d'image m, et cv::moments(contours[index_aire_contour_max])) permet de calculer les composantes de moments ce plus grand contour

    if (index_plus_grand_contour !=-1){

        cv::Moments m = cv::moments(contours[index_plus_grand_contour]);

        //sécurité si on a trouvé un contour (double sécurité ?) si on a déja trouvé un contour, comment sont aire peut etre nulle ? La réponse vient du fait qu'on peut avoir juste une ligne de contour ne se fermant pas : sans épaisseur : elle a effectivement une aire nulle
        //calcul de l'absisce et ordonnée du centre de ce contour X,Y (barycentres),

        if (m.m00 >0){
            int centre_x = m.m10/m.m00;
            int centre_y = m.m01/m.m00;
            std::cout << "Cible trouvee aux coordonnees : X=" << centre_x << ", Y=" << centre_y << std::endl;

            //dessin sur limage originale bgr du contour externe et du cercle du ''centre de masse'' du contour

            //Paramètres de cv::drawContours(image_base, contours passé par ref, index du plus grand contour et -1 donne tous les contours, couleur du tracé de contour : cv::Scalar(B,G,R), taille du trait ici on prend 2)
            cv::drawContours(image_bgr, contours, index_plus_grand_contour, cv::Scalar(255, 0, 0), 2) ;

            //Paramètres de cv::circle(image_base bgr, cv::Point(centre_x,centre_y), rayon du cercle 5 ici, couleur du trait cv::Scalar(B,G,R), taile du trait)
            cv::circle(image_bgr, cv::Point(centre_x,centre_y),5, cv::Scalar(0,255,0),-1);

        }
          } else {
        std::cout << "Aucune cible détectée à l'écran." << std::endl;
    }

    cv::imshow("Resultat du tracking", image_bgr);
    cv::waitKey(0);

    return 0;

    }
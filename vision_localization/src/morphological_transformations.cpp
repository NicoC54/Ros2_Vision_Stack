#include <opencv2/opencv.hpp>

int main(){

    cv::Mat image_bgr = cv::imread("/home/nico/vision_ws/src/ros2_vision_stack/vision_localization/balle_rouge.jpg");

    if (image_bgr.empty()){
        std::cerr << "Erreur:Image introuvable !" << std::endl;
        return -1;
    }
    else{
        std::cout << "Image trouvée !" << std::endl;
        //cv::imshow("Test_balle",image_bgr);
    }

    cv::Mat image_hsv;


    //cv::cvtColor(image de depart, image d'arrivée, opération réalisée) permet de passer du rgb (bgr en opencv) en hsv
    cv::cvtColor(image_bgr, image_hsv, cv::COLOR_BGR2HSV);

    //Définition des threshold de selection d'un pixel blanc: 
    cv::Scalar limite_basse1(0,120,70);
    cv::Scalar limite_haute1(10,255,255);

    //comme le rouge est particulier en openCV et découpé: plage H entre 0-10 et 160-179 on crée deux threshold et on va faire une opération OU.
    cv::Scalar limite_basse2(160,120,70);
    cv::Scalar limite_haute2(179,250,255);

    cv::Mat masque1, masque2, masque_final;

    //création des deux matrices noires-blanches/masques binaires : cv::inRange(matrice de depar, threshold bas, threshold up, matrice d'arrivée) crée cette matrice de pixels
    cv::inRange(image_hsv, limite_basse1, limite_haute1, masque1);
    cv::inRange(image_hsv, limite_basse2, limite_haute2, masque2);

    //porte logique ou sur les deux matrices : crée masque final.
    cv::bitwise_or(masque1,masque2,masque_final);

    //cv::namedWindow("image originale bgr", cv::WINDOW_NORMAL);
    //cv::imshow("image originale bgr", image_bgr);

    //cv::namedWindow("image hsv", cv::WINDOW_NORMAL);
    //cv::imshow("image hsv", image_hsv);
    
    //cv::namedWindow("masque rouge final", cv::WINDOW_NORMAL);
    //cv::imshow("masque rouge final", masque_final);

    //creation du tampon : l'objet qui va nous servir à éroder/dilater nos matrices noir-blanches. getStructuringElement(forme, taille) permet de creer cet objet
    //il existe d'autres formes de tampons : MORPH_ELLIPSE par exemple
    cv::Mat kernel = getStructuringElement(cv::MORPH_RECT, cv::Size(5,5));
    //cv::imshow("image tampon", kernel);

    cv::Mat masque_ouverture;
    cv::Mat masque_fermeture;

    //Opération douverture = erodage + dilatation (supprimer le bruit)= cv::MORPH_OPEN a linterieur de cv::morphologyEx(matrice d'entrée, matrice de sortie, opération: cv::MORPH_OPEN ou cv::MORPH_CLOSE, tampon)
    cv::morphologyEx(masque_final, masque_ouverture, cv::MORPH_OPEN, kernel);
    cv::imshow("image ouverture",masque_ouverture);

    //opération de fermeture = dilation + erodage (reboucher les trous liés à la lumière par exemple)
    cv::morphologyEx(masque_ouverture, masque_fermeture, cv::MORPH_CLOSE, kernel);
    cv::imshow("image fermeture",masque_fermeture);


    cv::waitKey(0);


    return 0;
}
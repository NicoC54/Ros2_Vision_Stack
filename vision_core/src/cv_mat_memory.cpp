#include <opencv2/opencv.hpp>
#include <iostream>

int main(){

    //création dune image 500x500 pixels, 8 bits 3 canaux(RGB) U = unsigned 0 a 255
    cv::Mat image_originale(500,500, CV_8UC3, cv::Scalar(255,255,255));

    //shallow copie : on ne cree pas une nouvelle image, les modifs sont gardées dans la meme image
    cv::Mat _shallow = image_originale;
    //creation d'une vraie copie
    cv::Mat _deep = image_originale.clone();

    //Création d'un rectangle à l'intérieur de l'image depart en x,y = 100,100 de taille 300x300
    cv::Rect zone(100,100,300,300);
    //on a le droit de passer la nouvellle matric en param de image_originale
    cv::Mat image_roi = image_originale(zone);

    //Coloration pixel en rouge attention methode pas opti
    image_originale.at<cv::Vec3b>(50,50) = cv::Vec3b(0,0,255);

    //on pointe vers le tout premier pixel Vec3b. On possède le pointeur qui est un tableau : en faisant pointeur[col] cela revient à faire *(pointeur + col)
    cv::Vec3b* premiere_ligne = image_originale.ptr<cv::Vec3b>(0);
    for (int col = 0; col < image_originale.cols; col++){
        premiere_ligne[col] = cv::Vec3b(255,0,0);
    }
    
}

/* MEMO COLONNE ET LIGNES pixels + pointeurs

//accès au numéro de ligne
cv::Vec3b* première_ligne = image.ptr<cv::Vec3b>(0);
cv::Vec3b* deuxième_ligne = image.ptr<cv::Vec3b>(1);

//accès au pixel par ligne
cv::Vec3b pixel_2_première_ligne = première_ligne[1]
cv::Vec3b pixel_2_deuxieme_ligne = deuxieme_ligne[1] = *(deuxième_ligne + 1)
*/
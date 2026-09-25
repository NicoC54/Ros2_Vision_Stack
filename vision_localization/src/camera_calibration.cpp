    #include <opencv2/opencv.hpp>
    #include <iostream>
    #include <vector>

    int main(){

        //définition et creation de la taille du damier DamierSize de taille 10 carré par 10
        cv::Size DamierSize(10,10);
        //définition et création dun vecteur contenant pour chaque image valide l'ensemble des corners 3d. = un vecteur de points par image
        std::vector<std::vector<cv::Point3f>> CornersDamier3dReel;
        //définition et création dun vecteur contenant pour chaque image valide l'ensemble des corners 2d calculés et trouvés à partir de l'image. = un vecteur de points par image
        std::vector<std::vector<cv::Point2f>> CornersDarmier2dCalculés;

        double CarreSize = 0.025; // taille dun carré

        std::vector<cv::Point3f> Damier3D; //déclaration du damier3D et remplissage de celui-ci utilisé dans la vraie vie pour faire la calibration : servira à remplir CornersDamier3dReel plus tard

        for (int i=0;i<DamierSize.height; i++){
            for (int j=0; j<DamierSize.width; j++){
                Damier3D.push_back(cv::Point3f(j*CarreSize, i*CarreSize, 0.0)); //attention erreur faite ici : ne pas oublier cv::Point3f et la dimensions z = 0
            }
        }

        //déclaration et création d'un vecteur contenant tous les files images
        std::vector<cv::String> ImageFiles;
        //on vient récuperer le files jpg du dossier image et les mettre dans ce vecteur
        cv::glob("/dossier_image/*.jpg", ImageFiles);

        //on boucle sur les files de ce vecteur image, pour chaque file=on le transforme en image puis on la convertit en gray image
        cv::Size ImageSize;
        for (const auto& ImageFile : ImageFiles){
            cv::Mat Image = cv::imread(ImageFile);
            cv::Mat GrayImage; 
            cv::cvtColor(Image,GrayImage,cv::COLOR_BGR2GRAY);

            //oubli déclaration de cv::Size pour ImageSize NB : on a déclaration dobjet avec cv::size mais on a aussi la méthode .size() ?
            ImageSize = GrayImage.size();

            //on déclare un vecteur qui contiendra pour chaque image les corners2d calculés et trouvés
            std::vector<cv::Point2f> cornersFound;

            //erreur faite inversion param DamierSize et cornersFound + oubli de lecriture des CALIB_CB
            //findChessBoardCorners sert à trouver les corners2d de limage, il renvoie true sil a trouvé autant de corners que la taille de DamierSize (ici 100)
            bool full = cv::findChessboardCorners(GrayImage, DamierSize, cornersFound,cv::CALIB_CB_ADAPTIVE_THRESH | cv::CALIB_CV_FAST_CHECK | cv::CALIB_CB_NORMALIZE_IMAGE);


        
            //si en gros pour chaque image on a trouvé tous les corners : on ne traite pas les images de faible qualité
            if (full){
                //erreur faite ici : oubli du nom de cv::TermCriteria
                
                //Optimisation de la précision CornerSubPix vient faire une fouille des niveaux de gris autour de chaque pixel corner et donner une précision sur leur position, avec virgule. Ici deux paramètres
                //servent à indiquer le nombre diterations de calculs max par pixel (30) ou sinon de sarreter si on est en dessous de 0.1 pixel pres en precision.
                cv::cornerSubPix(GrayImage, cornersFound , cv::Size(11,11), cv::Size(-1,-1), cv::TermCriteria(cv::TermCriteria::MAX_ITER + cv::TermCriteria::EPS, 30, 0.1));
                
                //insertion des corners 2D calculés vs le Damier 3D utilisé en réel (vecteur instancié au départ Damier3D)
                CornersDarmier2dCalculés.push_back(cornersFound);
                CornersDamier3dReel.push_back(Damier3D);

            }   }

        //erreur faites ici : déclaration de distortioncoeff comme un vecteur au lieu dune matrice
        //déclaration de la matrice instrinseque/distortion de coeff et des tvec,rvecs (translations et rotations = matrice extrinseque)
        cv::Mat MatrixIntra;
        cv::Mat DistortionCoeffs;
        std::vector<cv::Mat> rvecs,tvecs;

        //calibration de la caméra a partir du vecteur de vecteur de points 3D , du vecteur de vecteur de corners 2D calculés, de la taille dune image grise (en pixel avec .size() ?): la matrice intrinseque ,coeff de distrortions, rvec, tvec : elle sort une précision RMS (plsu cest bas plus cest mieux = en moyenne de combien de pixel on pense se tromper), elle rempli la matrice intrinseque, la matrice de distrortion, tvec et rvec qui sont plus utilisés arpes
        double rms_error = cv::calibrateCamera(CornersDamier3dReel,CornersDarmier2dCalculés, ImageSize, MatrixIntra, DistortionCoeffs, rvecs, tvecs);

        //déclaration d'une image qu'on vient de prendre quelqconque, image_corrigé elle correspond à la sortie après avoir 'détordue l'image)
        cv::Mat image_prise_distordue = cv::imread("dossier_image/image_distordue");
        cv::Mat image_corrigee;
        //renvoi l'image corrigée à partir des paramètre retournés par calibrateCamera
        cv::undistort(image_prise_distordue,image_corrigee, MatrixIntra, DistortionCoeffs);
    
        return 0;}
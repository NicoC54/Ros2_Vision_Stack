#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <std_msgs/msg/header.hpp>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>
#include <chrono> // timer


class PublisherImage : public rclcpp::Node{

    public: 

        PublisherImage() : Node("PublisherImage"){
            publisher_ = this->create_publisher<sensor_msgs::msg::Image>("topic_image",10);
            timer_ = this->create_wall_timer(std::chrono::seconds(1),[this](){this->callbackImage();});
        }

    private:


        void callbackImage(){
            //imaginons que image traitée est une image qu'un noeud OpenCv vient de traiter
            cv::Mat image_traitee; 

            if (image_traitee.empty()){
                return;
            }
            //On a besoin de fournir un Header à Ros dans lequel, on va mettre un timestamp et une frame_id
            std_msgs::msg::Header header;
            header.stamp = this->get_clock()->now();
            header.frame_id = "camera frame";

            // on crée un shared ptr msg sous format ros : sensor_msgs::msg::Image::SharedPtr : Pourquoi ? car CvImage(header,encodage,image_traitee) crée un objet conteneur d'image, et toImageMsg vient le transformer en msg, mais donne un shared ptr vers ce msg (cela évite de faire une copie de la matrice).
            sensor_msgs::msg::Image::SharedPtr msg = cv_bridge::CvImage(header,"bgr8",image_traitee).toImageMsg();

            // on publie l'objet associé à ce ptr : l'image
            publisher_->publish(*msg);
        }

        rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher_;
        rclcpp::TimerBase::SharedPtr timer_;
};


/**
 * ============================================================================
 * RAPPELS / PIÈGES CLÉS : ROS 2, OpenCV & cv_bridge (PUBLISHER)
 * ============================================================================
 * 
 * 1. SYNTAXE DU TIMER (create_wall_timer) :
 *    - Un timer ne reçoit pas de données extérieures. Sa fonction callback ne 
 *      doit prendre AUCUN argument.
 *    - Mauvais : [this](const msg&) { return callbackImage(msg); }
 *    - Bon     : [this]() { this->callbackImage(); }
 * 
 * 2. CRÉATION DU MESSAGE AVEC cv_bridge :
 *    - CvImage n'est pas le message final, c'est un conteneur temporaire C++.
 *    - Il faut obligatoirement appeler .toImageMsg() à la fin.
 *    - toImageMsg() renvoie un SharedPtr, ce qui évite de saturer la RAM.
 *    - Syntaxe : cv_bridge::CvImage(header, "bgr8", image_traitee).toImageMsg();
 * 
 * 3. PUBLICATION DU MESSAGE :
 *    - Puisque msg est un pointeur intelligent (SharedPtr), la fonction publish() 
 *      demande l'objet réel. Il faut donc le déréférencer avec l'étoile (*).
 *    - Mauvais : publisher_->publish(msg);
 *    - Bon     : publisher_->publish(*msg);
 * 
 * 4. DANGERS AU RUNTIME (Exécution) :
 *    - Passer une cv::Mat vide (sans pixels alloués) à cv_bridge provoque un 
 *      crash immédiat (Segmentation Fault). Toujours vérifier avec .empty().
 * 
 * 5. STRUCTURE DE LA CLASSE :
 *    - Ne pas oublier de déclarer publisher_ et timer_ en variables membres (private).
 *    - Ne pas oublier le point-virgule (;) à la toute fin de la classe.
 * ============================================================================
 */
#include <rclcpp/rclcpp.hpp>
#include <sensor_msgs/msg/image.hpp>
#include <std_msgs/msg/header.hpp>
#include <cv_bridge/cv_bridge.h>
#include <opencv2/opencv.hpp>



class VisionNode : public rclcpp::Node{

    public:

    VisionNode() : Node("Vision_node"){

       
        subscriber_ = this->create_subscription<sensor_msgs::msg::Image>("nom_topic_image",10, std::bind(&VisionNode::callbackimage,this,std::placeholders::_1));

    }



    private:

        void callbackimage(const sensor_msgs::msg::Image::SharedPtr msg) const {


            try {

                cv::Mat cv_image = cv_bridge::toCvShare(msg, "bgr8") -> image;
                RCLCPP_INFO(this->get_logger(),"image placée, de dimensions %d x %d", cv_image.cols, cv_image.rows);
                 }

                 catch (const cv_bridge::Exception& e){
                    RCLCPP_ERROR(this->get_logger(), "Erreur de conversion cv_bridge : %s", e.what());
                 }


        }

        std::shared_ptr<rclcpp::Subscription<sensor_msgs::msg::Image>> subscriber_;


};


/**
 * ============================================================================
 * RAPPELS / PIÈGES CLÉS : ROS 2, OpenCV & cv_bridge
 * ============================================================================
 * 
 * 1. TYPES DE MESSAGES ROS 2 :
 *    - Mauvais : std_msgs::sensor::Image
 *    - Bon     : sensor_msgs::msg::Image
 * 
 * 2. PARAMÈTRE DU CALLBACK (Subscriber) :
 *    - cv_bridge a besoin du pointeur intelligent pour partager la mémoire.
 *    - Mauvais : const sensor_msgs::msg::Image& msg (référence)
 *    - Bon     : const sensor_msgs::msg::Image::SharedPtr msg (pointeur partagé)
 * 
 * 3. CRÉATION DE L'ABONNÉ (create_subscription) :
 *    - Ne pas oublier la taille de la file d'attente (queue size, ex: 10).
 *    - Syntaxe : this->create_subscription<Type>(topic, queue_size, callback);
 * 
 * 4. SYNTAXE DE cv_bridge :
 *    - Namespace : cv_bridge (avec underscore, pas d'espace).
 *    - Fonction  : cv_bridge::toCvShare(msg, "bgr8") ou toCvCopy(msg, "bgr8")
 *    - Encodage  : Toujours entre guillemets doubles ("bgr8"), jamais simples ('bgr8').
 *    - Extraction: Utiliser la flèche minuscule ->image (et non -> Image).
 * 
 * 5. DÉCLARATION DE LA VARIABLE MEMBRE :
 *    - Ne pas oublier de nommer la variable à la fin du type :
 *    - Bon : rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscriber_;
 * ============================================================================
 */



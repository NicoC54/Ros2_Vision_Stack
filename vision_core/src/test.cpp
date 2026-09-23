class SubscriberNode : public rclcpp::Node{
    public:
        SubscriberNode() : Node("SubscriberNode") {
            subscriber_ = this->create_subscription<sensor_msgs::msg::Image>("topic_image",10, [this](const sensor_msgs::msg::Image::SharedPtr msg){this->callback(msg);});
        }

    private:
        void callback(const sensor_msgs::msg::Image::SharedPtr msg) {
            try {
                cv::Mat image = cv_bridge::toCvShare(msg, "bgr8") -> image;
                RCLCPP_INFO(this->get_logger(), "Matrice bien reçue");
                }

            catch (const cv_bridge::Exception& e){
                RCLCPP_ERROR(this->get_logger(),"Erreur de conversation cv_bridge : %s", e.what());
            }
                
        }

        rclcpp::Subscription<sensor_msgs::msg::Image>::SharedPtr subscriber_;
    };


class PublisherNode : public rclcpp::Node{
    public:

        PublisherNode() : Node("PublisherNode"){

            publisher_ = this -> create_publisher<sensor_msgs::msg::Image>("topic_node",10);
            timer_ = this -> create_wall_timer(std::chrono::milliseconds(10), [this](){this->callbackTimer();})

             }

    private:
    
            void callbackTimer(){

                std_msgs::msg::Header header;
                header.frame_id = "camera_link";
                cv::Mat image_traitee;

                if (image_traitee.empty()){
                    return;
                }

                header.stamp = this -> get_clock() -> now();
                std_msgs::msg::Image:SharedPtr msg = cv_bridge::CvImage(header,"bgr8",image_traitee).toMsgImage();

                publisher_-> publish(*msg);

            }

        rclcpp::Publisher<sensor_msgs::msg::Image>::SharedPtr publisher_;
        rclcpp::TimerBase timer_;
        
    };


/**
 * ============================================================================
 * MÉMO DES ERREURS DE SYNTAXE (À NE PAS REFAIRE) :
 * ============================================================================
 * 
 * 1. NOM DU PAQUET DE MESSAGES IMAGE :
 *    - Erreur : std_msgs::msg::Image
 *    - Correct: sensor_msgs::msg::Image (l'image vient du paquet sensor_msgs).
 * 
 * 2. DOUBLE DEUX-POINTS POUR LE POINTEUR PARTAGÉ :
 *    - Erreur : sensor_msgs::msg::Image:SharedPtr (un seul :)
 *    - Correct: sensor_msgs::msg::Image::SharedPtr (toujours :: pour les sous-types).
 * 
 * 3. NOM DE LA MÉTHODE DE CONVERSION PUBLISHER :
 *    - Erreur : .toMsgImage()
 *    - Correct: .toImageMsg() (Image d'abord, Msg ensuite).
 * 
 * 4. OUBLI DE POINT-VIRGULE (;) :
 *    - Souvent oublié à la fin de l'initialisation du timer :
 *    - Correct: timer_ = this->create_wall_timer(...);
 * 
 * 5. OUBLI DE VIRGULE DANS LE LOGGER (RCLCPP) :
 *    - Erreur : RCLCPP_ERROR(this->get_logger() "Mon message d'erreur")
 *    - Correct: RCLCPP_ERROR(this->get_logger(), "Mon message d'erreur")
 * ============================================================================
 */








        


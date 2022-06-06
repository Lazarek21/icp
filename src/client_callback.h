#ifndef CLIENT_CALLBACK_H
#define CLIENT_CALLBACK_H
#include <mqtt/async_client.h>
#include "subscriber.h"
#include <QObject>
#include <QImage>
#include <QLabel>
#include <fstream>
#include <QPixmap>
class client_callback : public QObject, public virtual mqtt::callback, public virtual mqtt::iaction_listener
{
    Q_OBJECT
    subscriber subscriber_;
    mqtt::async_client &client_;
    std::string subscriber_topic_;

    void connected (const std::string& cause) override {
        emit(successfully_connected());
        client_.subscribe(subscriber_topic_,2,nullptr,subscriber_);
    };
    void connection_lost(const std::string& cause) override {
        std::cout << "connection lost!: " << cause<<std::endl;
    };
    void delivery_complete	(	mqtt::delivery_token_ptr 	tok) override {

    };
    void message_arrived	(	mqtt::const_message_ptr 	msg	) override {
        QByteArray arr = QByteArray::fromStdString(msg->get_payload_str());
        emit(newMessageArrived(QString::fromStdString(msg->get_topic()),arr));

    };

    void on_success(const mqtt::token &asyncActionToken) override {
        std::cout << "Successfully connected!"<<std::endl;
    };

    void on_failure(const mqtt::token &asyncActionToken) override {
        std::cerr << "connection failed!" << std::endl;
    };
public:
    client_callback(mqtt::async_client& client) : client_(client){};
    void set_subscriber_topic(std::string topic){
        subscriber_topic_ = topic;
    }
signals:
    void newMessageArrived(QString topic, QByteArray payload);
    void successfully_connected();
};

#endif // CLIENT_CALLBACK_H

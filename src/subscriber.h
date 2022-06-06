#ifndef SUBSCRIBER_H

#define SUBSCRIBER_H
#include "mqtt/async_client.h"
#include <iostream>

class subscriber : public virtual mqtt::iaction_listener {
    void on_success(const mqtt::token &asyncActionToken) override {
    };
    void on_failure(const mqtt::token &asyncActionToken) override {
    };
public:
    subscriber(){};
};


#endif // SUBSCRIBER_H

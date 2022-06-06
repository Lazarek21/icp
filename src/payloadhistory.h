#ifndef PAYLOADHISTORY_H
#define PAYLOADHISTORY_H
#include <vector>
#include <QByteArray>
typedef enum  {NUM=0 ,STR=1, PIC=2} PayloadType;

class PayloadHistory
{

public:
    PayloadType type = NUM;
    PayloadType last_type;
    std::vector<QByteArray> history;
    PayloadHistory();
    void push_back(QByteArray arr);

};

#endif // PAYLOADHISTORY_H

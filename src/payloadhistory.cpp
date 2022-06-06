#include "payloadhistory.h"
#include <mainwindow.h>


PayloadHistory::PayloadHistory()
{

}

void PayloadHistory::push_back(QByteArray arr)
{
    if (MainWindow::isPicture(arr))
    {
        type = PIC;
    } else {
        if (type == NUM)
        {
            bool is_double = false;
            arr.toDouble(&is_double);
            if (!is_double)
            {
                type = STR;
            }
        }
    }
    history.push_back(arr);
}



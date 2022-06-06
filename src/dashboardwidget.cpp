#include "dashboardwidget.h"
#include <iostream>

DashboardWidget::DashboardWidget(QObject *parent) : QObject(parent)
{


}

DashboardWidget::DashboardWidget(const DashboardWidget &DW)
{
    box = DW.box;
    picture = DW.picture;
    browser = DW.browser;
    removeButton = DW.removeButton;
    type = DW.type;
    publishButton = DW.publishButton;
}

void DashboardWidget::connect_button()
{
    connect(removeButton,SIGNAL(clicked()),this, SLOT(remove_button_clicked()));
    connect(publishButton, SIGNAL(clicked()),this, SLOT(publish_button_clicked()));
}

void DashboardWidget::remove_button_clicked()
{
 box->deleteLater();
 box = nullptr;
}

void DashboardWidget::publish_button_clicked()
{
    emit new_topic_to_publish(box->accessibleName());
}


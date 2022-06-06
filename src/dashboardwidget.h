#ifndef DASHBOARDWIDGET_H
#define DASHBOARDWIDGET_H

#include <QObject>
#include <QPushButton>
#include <QTextBrowser>
#include <QGroupBox>
#include <QLabel>

typedef enum {TXT=0, PICT=1} DashboardWidgetType;
class DashboardWidget : public QObject
{
    Q_OBJECT

public:
    DashboardWidgetType type;
    QGroupBox *box = nullptr;
    QTextBrowser *browser = nullptr;
    QLabel *picture = nullptr;
    QPushButton *removeButton = nullptr;
    QTextBrowser *payload = nullptr;
    QPushButton *publishButton = nullptr;
    DashboardWidget(QObject *parent = nullptr);
    DashboardWidget(const DashboardWidget &DW);

    void connect_button();
private slots:
    void remove_button_clicked();
    void publish_button_clicked();

signals:
    void new_topic_to_publish(QString topic);

};

#endif // DASHBOARDWIDGET_H

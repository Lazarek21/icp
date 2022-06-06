#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include "mqtt/async_client.h"
#include <memory>
#include "client_callback.h"
//#include <QtCharts>
#include <QTreeWidgetItem>
#include <map>
#include <payloadhistory.h>
#include <QGroupBox>
#include "dashboardwidget.h"

QT_BEGIN_NAMESPACE
namespace Ui { class MainWindow; }
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    QString publish_file;
    std::map<std::string,PayloadHistory> HistoryMap;
    std::map<std::string,DashboardWidget> DashboardMap;

    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void connect_to_broker(QString server, QString username);

    std::string getConnectString();
    std::string get_subscriber_topic();
    QString get_publisher_topic();

    void connectionEstablished();
    void disconnect_from_broker();

    void addToTree(std::string topic, QByteArray mess, bool publish);
    void closeEvent(QCloseEvent* event);

    QString treeItemToFullPath(QTreeWidgetItem *treeItem);

    void addToHistoryMap(QByteArray arr, std::string path);
    static bool isPicture(QByteArray arr);
    QWidget *createHistoryWidget(std::string topic, QWidget *wid, unsigned int record);
    void addToDashboard(std::string topic);

    void onNewMessageToPublish(QString topic, QByteArray payload);

private slots:
    void on_tabWidget_tabBarClicked(int index);

    void on_tabWidget_tabCloseRequested(int index);

    void on_connectButton_released();

    void on_disconnectButton_released();

    void on_clearButton_released();

    void on_customTopics_clicked();

    void on_allTopics_clicked();

    void onNewMessageArrived(QString topic, QByteArray payload);

    void on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column);

    void on_selectFileButton_clicked();

    void on_treeWidget_itemClicked(QTreeWidgetItem *item, int column);

    void on_save_clicked();

    void on_clearTree_clicked();

    void client_connected();

    void on_selectDashWidgetButton_clicked();

    void on_newDashWidgetButton_clicked();

    void on_publishButton_clicked();
    void on_dashboard_publish_button_clicked(QString topic);


private:
    Ui::MainWindow *ui;
    std::unique_ptr<mqtt::async_client> client_;
    std::unique_ptr<client_callback> call_back_;
};


#endif // MAINWINDOW_H

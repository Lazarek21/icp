#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <iostream>
#include <QDebug>
#include <QTime>
#include <QMessageBox>
#include <QPlainTextEdit>
#include <string>
#include <QThread>
#include "mqtt/async_client.h"
#include <client_callback.h>
#include <memory>
#include <utility>
#include <subscriber.h>
#include <filesystem>
#include <QImage>
#include <fstream>
#include <QCloseEvent>
#include <QTextBrowser>
#include <QFileDialog>
#include <QList>
#include <stack>
#include <QPainterPath>
#include <QScrollBar>

void delay()
{
    QTime dieTime= QTime::currentTime().addSecs(1);
    while (QTime::currentTime() < dieTime)
        QCoreApplication::processEvents(QEventLoop::AllEvents, 100);
}

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->scrollAreaWidgetContents->setLayout(new QVBoxLayout(ui->scrollAreaWidgetContents));
    ui->scrollAreaWidgetContents->layout()->setSpacing(30);

}

MainWindow::~MainWindow()
{
    delete ui;
}

std::string MainWindow::getConnectString(){
    return ui->connectString->text().toUtf8().constData();
}

std::string MainWindow::get_subscriber_topic(){
    if(ui->allTopics->isChecked() || ui->topic->text() == ""){
        return "$SYS/#";
    }else{
        return ui->topic->text().toStdString();
    }

}

QString MainWindow::get_publisher_topic()
{
    return ui->publishTopic->text();
}

bool MainWindow::isPicture(QByteArray arr){
    QPixmap img(arr);

    QImage img1 = QImage::fromData(arr);

    return !img1.isNull() || !img.toImage().isNull();
}

void MainWindow::addToHistoryMap(QByteArray arr, std::string path){
    if (HistoryMap[path].history.size() == 0){
        ui->topicList->addItem(QString::fromStdString(path));
    }
    HistoryMap[path].push_back(arr);
}

void MainWindow::addToTree(std::string topic, QByteArray mess, bool publish){
    QList<QTreeWidgetItem *>parent_list = ui->treeWidget->findItems(QString::fromStdString(topic.substr(0, topic.find_first_of("/"))), Qt::MatchExactly);
    std::filesystem::path topic_path = std::filesystem::path(topic);
    QTreeWidgetItem *parent;

    if(parent_list.isEmpty()){
        QTreeWidgetItem *root = new QTreeWidgetItem(ui->treeWidget);
        root->setText(0,QString::fromStdString(topic.substr(0, topic.find_first_of("/"))));
        ui->treeWidget->addTopLevelItem(root);
    }

    parent_list = ui->treeWidget->findItems(QString::fromStdString(topic.substr(0, topic.find_first_of("/"))), Qt::MatchExactly);
    parent = parent_list[0];
    bool first = true;
    std::string sim_path = "";
    for(std::string var : topic_path){
        QTreeWidgetItem *child;
        bool found =false;
        if(first){
            sim_path = var;
            first = false;
        } else {
            sim_path = sim_path.append("/");
            sim_path = sim_path.append(var);
            QList<QTreeWidgetItem *> child_list = ui->treeWidget->findItems(QString::fromStdString(var), Qt::MatchExactly|Qt::MatchRecursive);
            if(child_list.isEmpty()){
                child = new QTreeWidgetItem(parent);
                child->setText(0,QString::fromStdString(var));
                parent->addChild(child);
                parent = child;
            } else {
                for(int i=0;i<child_list.count();i++){
                    if(child_list[i]->parent()==parent){
                        parent = child_list[i];
                        child = parent;
                        found = true;
                    }
                }
                if(!found){
                    child = new QTreeWidgetItem(parent);
                    child->setText(0,QString::fromStdString(var));
                    parent->addChild(child);
                    parent = child;
                }
            }
        }
        if(sim_path == topic){
            if(publish){
                parent->setForeground(0,Qt::red);
            } else {
                if(isPicture(mess)){
                    parent->setText(1,"Picture");
                } else {
                    parent->setText(1,QString::fromStdString(mess.toStdString().substr(0,20)));
                }
                addToHistoryMap(mess, topic);
            }
        }

    }
}

void MainWindow::connectionEstablished(){
        ui->frame->setEnabled(true);
}

void MainWindow::connect_to_broker(QString server, QString username){
    QMessageBox messageBox;
    client_ = std::make_unique<mqtt::async_client>(server.toStdString(), username.toStdString());
    call_back_ = std::make_unique<client_callback>(*client_);
    connect(call_back_.get(),SIGNAL(successfully_connected()),
            this, SLOT(client_connected()));
    connect(call_back_.get(),SIGNAL(newMessageArrived(QString,QByteArray)),
            this, SLOT(onNewMessageArrived(QString,QByteArray)));
    mqtt::connect_options connOpts;

    //firstly install callback
    client_->set_callback(*call_back_);
    call_back_->set_subscriber_topic(get_subscriber_topic());
    ui->statusbar->showMessage("Connecting...");

    try {
        client_->connect(connOpts, nullptr, *call_back_);
        QList<QWidget*> list = ui->frame->findChildren<QWidget*>();
        foreach (auto obj, list) {
             obj->setEnabled(false);
        }
        ui->widget->setEnabled(true);
        ui->widget_2->setVisible(false);
        ui->widget_4->setVisible(false);
        ui->disconnectButton->setEnabled(true);
        ui->publishWidget->setEnabled(true);

    } catch (const mqtt::exception& exc) {
        messageBox.critical(0,"Error","Unable to connect to MQTT server");
    }

}

void MainWindow::disconnect_from_broker(){
    QMessageBox messageBox;
    try {
        client_->disconnect();
    }  catch (const mqtt::exception& e) {
        messageBox.critical(0,"Error",e.what());
        messageBox.show();
    }
    QList<QWidget*> list = ui->frame->findChildren<QWidget*>();
    foreach (auto obj, list) {
         obj->setEnabled(true);
    }
    ui->disconnectButton->setEnabled(false);
    ui->widget_2->setVisible(true);
    ui->widget_4->setVisible(true);
    ui->publishWidget->setEnabled(false);
    ui->statusbar->showMessage("Disconnected", 2000);
}


void MainWindow::on_tabWidget_tabBarClicked(int index)
{
    QTabWidget tab;

    if(ui->tabWidget->tabText(index) == "+"){
        ui->tabWidget->setTabText(index, "Dashboard");
        ui->tabWidget->setTabsClosable(true);
    }
}

void MainWindow::on_tabWidget_tabCloseRequested(int index)
{
    if(ui->tabWidget->tabText(index) == "Dashboard"){
        ui->tabWidget->setTabText(index, "+");
        ui->tabWidget->setTabsClosable(false);
        ui->tabWidget->setCurrentIndex(0);
    }
}

void MainWindow::on_connectButton_released()
{
    QString server =  ui->connectString->text();
    QString username = ui->username->text();
    QMessageBox messageBox;
    messageBox.setFixedSize(500,200);

    if (server.isEmpty())
    {
        messageBox.critical(0,"Error","Enter server name");
        messageBox.show();
        return;
    }

    if (username.isEmpty())
    {
        messageBox.critical(0,"Error","Enter username");
        messageBox.show();
        return;
    }
    connect_to_broker(server,username);
}

void MainWindow::on_disconnectButton_released()
{
    disconnect_from_broker();
}



void MainWindow::on_clearButton_released()
{
    ui->connectString->setText("");
}

void MainWindow::on_customTopics_clicked()
{
    ui->widget_7->setVisible(true);
}

void MainWindow::on_allTopics_clicked()
{
    ui->widget_7->setVisible(false);
}


void MainWindow::onNewMessageArrived(QString topic, QByteArray payload)
{
    std::string topic_s = topic.toStdString();
    addToTree(topic_s, payload,false);
    if (DashboardMap[topic_s].box != nullptr)
    {
        if (HistoryMap[topic_s].type == PIC)
        {
            if(HistoryMap[topic_s].last_type == PIC){
                DashboardMap[topic_s].picture->setPixmap(QPixmap::fromImage(QImage::fromData(HistoryMap[topic_s].history.back())));
                DashboardMap[topic_s].picture->setVisible(true);
                DashboardMap[topic_s].browser->setVisible(false);
            } else {
                DashboardMap[topic_s].browser->setText(HistoryMap[topic_s].history.back());
                DashboardMap[topic_s].picture->setVisible(false);
                DashboardMap[topic_s].browser->setVisible(true);
            }
        } else if (HistoryMap[topic_s].type == STR){

            DashboardMap[topic_s].browser->append("\n"+HistoryMap[topic_s].history.back());
            DashboardMap[topic_s].picture->setVisible(false);
            DashboardMap[topic_s].browser->setVisible(true);

        } else {

        }
    }
}


void MainWindow::closeEvent (QCloseEvent *event)
{
    QMessageBox::StandardButton resBtn = QMessageBox::question( this, "MQTTExplorer",
                                                                tr("Do you want to exit?\n"),
                                                                QMessageBox::No | QMessageBox::Yes);
    if (resBtn != QMessageBox::Yes) {
        event->ignore();
    } else {
        //firstly disconnect
        if (client_ != nullptr)
        {
            if (client_->is_connected())
            {
                disconnect_from_broker();
            }
        }
        event->accept();
    }
}

void MainWindow::on_treeWidget_itemDoubleClicked(QTreeWidgetItem *item, int column)
{
    std::string full_path = treeItemToFullPath(item).toStdString();

        if(!HistoryMap[full_path].history.empty()){
            QLabel* myLabel = new QLabel(this);
            myLabel->setLayout(new QVBoxLayout);

            for(unsigned int i=0;i<HistoryMap[full_path].history.size()&&i<5;i++){
                myLabel->layout()->addWidget(createHistoryWidget(full_path,myLabel,HistoryMap[full_path].history.size()-i-1));
            }

            myLabel->setWindowFlags(Qt::Window);

            myLabel->setWindowTitle(QString::fromStdString(full_path));
            myLabel->show();
        }
}

QString MainWindow::treeItemToFullPath(QTreeWidgetItem* treeItem)
{
    QString fullPath= treeItem->text(0);

    while (treeItem->parent() != NULL)
    {
        fullPath= treeItem->parent()->text(0) + "/" + fullPath;
        treeItem = treeItem->parent();
    }
    return fullPath;
}

void MainWindow::on_selectFileButton_clicked()
{
    if(ui->selectFileButton->text()=="Clear"){
        ui->label_6->setEnabled(true);
        ui->publishText->setEnabled(true);
        ui->FileName->setText("");
        ui->selectFileButton->setText("Select File");
        publish_file = "";
    } else{
        publish_file = QFileDialog::getOpenFileName(this, "Select a file...", QDir::homePath(),tr("Images (*.png *.xpm *.jpg);;All files (*)"));
        std::filesystem::path filePath = std::filesystem::path(publish_file.toStdString());
        if (publish_file !=""){
            ui->label_6->setEnabled(false);
            ui->publishText->setEnabled(false);
            ui->FileName->setText(QString::fromStdString(filePath.filename()));
            ui->selectFileButton->setText("Clear");
        }
    }
}

void MainWindow::on_treeWidget_itemClicked(QTreeWidgetItem *item, int column)
{
    ui->publishTopic->setText(treeItemToFullPath(item));
}



void MainWindow::on_publishButton_clicked()
{
    QByteArray payload;
    mqtt::const_message_ptr msg;
    if (publish_file != "")
    {
        QFile src_file(publish_file);
        if(!src_file.open(QIODevice::ReadOnly)) {
            ui->statusbar->showMessage("invalid file",2000);
            ui->label_6->setEnabled(true);
            ui->publishText->setEnabled(true);
            ui->FileName->setText("");
            ui->selectFileButton->setText("Select File");
            return;
        }
        payload = src_file.readAll();
    } else {
        payload = QByteArray::fromStdString(ui->publishText->toPlainText().toStdString());
    }
    onNewMessageToPublish(get_publisher_topic(), payload);
}

void MainWindow::on_dashboard_publish_button_clicked(QString topic)
{
    QByteArray payload = QByteArray::fromStdString(DashboardMap[topic.toStdString()].payload->toPlainText().toStdString());
    onNewMessageToPublish(topic, payload);
    std::cout << "co se deje" <<std::endl;
}

void MainWindow::onNewMessageToPublish(QString topic, QByteArray payload)
{
    addToTree(topic.toStdString(), payload,true);
    mqtt::const_message_ptr msg = mqtt::make_message(topic.toStdString(),payload,payload.size());
    client_->publish(msg);
}




void MainWindow::on_save_clicked()
{
    if(!HistoryMap.empty()){
        using namespace std;

        filesystem::path dirPath;
        filesystem::path pathFolder = filesystem::path(QFileDialog::getExistingDirectory(0, ("Select Output Folder"), QDir::currentPath()).toStdString());
        if(pathFolder != ""){
            filesystem::current_path(pathFolder);
            for(auto node : HistoryMap){
                if(!filesystem::create_directories(node.first)){
                    QMessageBox msg(this);
                    msg.critical(0,"Error","Could not create directory");
                    msg.show();
                    return;
                }
                filesystem::current_path(node.first);

                QFile file;
                if(isPicture(node.second.history[node.second.history.size()-1])){
                    file.setFileName("./payload.jpg");
                }else {
                    file.setFileName("./payload.txt");
                }
                if(!file.open(QIODevice::WriteOnly)){
                    QMessageBox msg(this);
                    msg.critical(0,"Error","Could not open file");
                    msg.show();
                    return;
                }
                file.write(node.second.history[node.second.history.size()-1]);
                file.close();
                filesystem::current_path(pathFolder);
            }
        }
    } else {
        ui->statusbar->showMessage("Nothing to save...",2000);
    }
}

void MainWindow::on_clearTree_clicked()
{
    HistoryMap.clear();
    ui->treeWidget->clear();
}

void MainWindow::client_connected()
{
    ui->statusbar->showMessage("Connected to " + ui->connectString->text());
    ui->frame->setEnabled(true);
}

QWidget *MainWindow::createHistoryWidget(std::string topic, QWidget *wid, unsigned int record){
    if(isPicture(HistoryMap[topic].history[record])){
        QLabel *label = new QLabel(wid);
        label->setPixmap(QPixmap::fromImage(QImage::fromData(HistoryMap[topic].history[record])));
        return label;
    } else {
        QTextBrowser *browser = new QTextBrowser(wid);
        browser->setText(HistoryMap[topic].history[record]);
        return browser;
    }
}

void MainWindow::on_selectDashWidgetButton_clicked()
{

    addToDashboard(ui->topicList->currentText().toStdString());
    ui->scrollArea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);

}

void MainWindow::addToDashboard(std::string topic){

    if(DashboardMap[topic].box==nullptr){
        DashboardMap[topic].box = new QGroupBox(ui->scrollAreaWidgetContents);
        DashboardMap[topic].box->setTitle(QString::fromStdString(topic));
        DashboardMap[topic].box->setFont(QFont("Calibri",20));
        DashboardMap[topic].box->setAccessibleName(QString::fromStdString(topic));
        DashboardMap[topic].box->setLayout(new QVBoxLayout(DashboardMap[topic].box));
        DashboardMap[topic].box->setStyleSheet("background-color: turquoise");

        DashboardMap[topic].removeButton = new QPushButton(DashboardMap[topic].box);
        DashboardMap[topic].removeButton->setText("Remove");
        DashboardMap[topic].box->layout()->addWidget(DashboardMap[topic].removeButton);

        DashboardMap[topic].picture = new QLabel(DashboardMap[topic].box);
        DashboardMap[topic].browser = new QTextBrowser(DashboardMap[topic].box);
        DashboardMap[topic].box->layout()->addWidget(DashboardMap[topic].picture);
        DashboardMap[topic].box->layout()->addWidget(DashboardMap[topic].browser);

        if(isPicture(HistoryMap[topic].history.back())){
           DashboardMap[topic].browser->setVisible(false);
           DashboardMap[topic].picture->setPixmap(QPixmap::fromImage(QImage::fromData(HistoryMap[topic].history.back())));
           DashboardMap[topic].type = PICT;
        } else{
            DashboardMap[topic].picture->setVisible(false);
            DashboardMap[topic].browser->setText(HistoryMap[topic].history.back());
            DashboardMap[topic].type = TXT;
        }


        DashboardMap[topic].publishButton = new QPushButton(DashboardMap[topic].box);
        DashboardMap[topic].publishButton->setText("Publish");
        DashboardMap[topic].box->layout()->addWidget(DashboardMap[topic].publishButton);
        connect(&DashboardMap[topic],SIGNAL(new_topic_to_publish(QString)),this, SLOT(on_dashboard_publish_button_clicked(QString)));

        DashboardMap[topic].connect_button();

        DashboardMap[topic].payload = new QTextBrowser(DashboardMap[topic].box);
        DashboardMap[topic].payload->setReadOnly(false);
        DashboardMap[topic].box->layout()->addWidget(DashboardMap[topic].payload);

        ui->scrollAreaWidgetContents->layout()->addWidget(DashboardMap[topic].box);

        DashboardMap[topic].browser->setFont(QFont("Calibri",11));
        DashboardMap[topic].payload->setFont(QFont("Calibri",11));
        DashboardMap[topic].removeButton->setFont(QFont("Calibri",11));
        DashboardMap[topic].publishButton->setFont(QFont("Calibri",11));

        QCoreApplication::processEvents(QEventLoop::AllEvents, 250);
        ui->scrollArea->verticalScrollBar()->triggerAction(QAbstractSlider::SliderToMaximum);
    }

}

void MainWindow::on_newDashWidgetButton_clicked()
{

}

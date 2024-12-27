#include "loginwindow.h"
#include "ui_loginwindow.h"
#include "homewindow.h"
#include "widget/titlebar/loginTitleBar/logintitlebar.h"

LoginWindow::LoginWindow(QWidget *parent)
    : QMainWindow(parent),
      ui(new Ui::LoginWindow),
      networkManager(new QNetworkAccessManager(this))
{
    ui->setupUi(this);

    // 禁用默认标题栏
   setWindowFlags(Qt::FramelessWindowHint);
   // 创建自定义标题栏并设置为窗口的标题栏
   LoginTitleBar *customTitleBar = new LoginTitleBar(this);
   setMenuWidget(customTitleBar);  // 将自定义标题栏作为菜单栏

   setStyleSheet("QMainWindow { "
                                "background-image: url(:/qss/lightblue/bg.png); "
                                "}");

   connect(networkManager, &QNetworkAccessManager::finished, this, &LoginWindow::onNetworkReply);
}

LoginWindow::~LoginWindow()
{
    delete ui;
}


void LoginWindow::on_loginBtn_clicked()
{
    QString username = ui->usernameEdit->text();
    QString pwd = ui->pwdEdit->text();

    if(username.isEmpty() || pwd.isEmpty()){
        // 创建信息提示框
        QMessageBox::information(this, "Information", "请输入用户名或密码！");
        return;
    }


    HomeWindow *home = new HomeWindow(ui->usernameEdit->text());
    home->show();
    this->close();
    return;

    QUrl url("http://49.232.203.132:8891/listenVisionAuthParent/api/auth/login");
    QNetworkRequest request(url);
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    QJsonObject json;
    json["username"] = username;
    json["password"] = pwd;

    QJsonDocument doc(json);
    QByteArray data = doc.toJson();

    networkManager->post(request, data);
}

void LoginWindow::onNetworkReply(QNetworkReply *reply)
{
    QByteArray responseData = reply->readAll();

    if (reply->error() == QNetworkReply::NoError) {
        QJsonDocument doc = QJsonDocument::fromJson(responseData);
        if (doc.isObject()) {
            QJsonObject jsonResponse = doc.object();
            int code = jsonResponse["code"].toInt();
            if (code == 200) {
                HomeWindow *home = new HomeWindow(ui->usernameEdit->text());
                home->show();
                this->close();
            } else {
                QString message = jsonResponse["message"].toString();
                // 创建信息提示框
                QMessageBox::information(this, "Information", message);
            }
        }
    } else {
        // 创建信息提示框
        QMessageBox::information(this, "Information", "错误："+reply->errorString());
    }

    reply->deleteLater();
}

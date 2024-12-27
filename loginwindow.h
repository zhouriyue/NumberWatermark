#ifndef LOGINWINDOW_H
#define LOGINWINDOW_H

#include <QMainWindow>
#include <QJsonDocument>
#include <QJsonObject>
#include <QUrlQuery>
#include <QNetworkRequest>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QDebug>

QT_BEGIN_NAMESPACE
namespace Ui { class LoginWindow; }
QT_END_NAMESPACE

class LoginWindow : public QMainWindow
{
    Q_OBJECT

public:
    LoginWindow(QWidget *parent = nullptr);
    ~LoginWindow();

private slots:
    void on_loginBtn_clicked();
    void onNetworkReply(QNetworkReply *reply);  // 网络请求响应的槽函数

private:
    Ui::LoginWindow *ui;
    QNetworkAccessManager *networkManager;  // 网络管理器
};
#endif // LOGINWINDOW_H

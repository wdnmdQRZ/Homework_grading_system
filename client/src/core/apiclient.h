#ifndef APICLIENT_H
#define APICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>  // 负责发送 HTTP 请求的核心类
#include <QNetworkReply>          // 服务器回复数据的封装类
#include <QJsonObject>            // JSON 对象，用于传递解析后的数据

class ApiClient : public QObject
{
    Q_OBJECT
public:
    explicit ApiClient(QObject *parent = nullptr);

    // 登录：向 POST /api/auth/login 发送 username + password
    void login(const QString &username, const QString &password);

    // 注册：向 POST /api/auth/register 发送 username + password
    void registerUser(const QString &username, const QString &password);

signals:
    // 登录结果信号
    void loginResult(bool ok, const QJsonObject &data, const QString &message);

    // 注册结果信号
    void registerResult(bool ok, const QString &message);

private slots:
    // QNetworkAccessManager::finished 信号触发，统一处理所有网络回复
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager;
    static const QString BASE_URL;

    // 追踪当前请求类型
    enum RequestType { None, Login, Register };
    RequestType m_pendingRequest = None;
};

#endif // APICLIENT_H
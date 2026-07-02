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

signals:
    // 登录结果信号，请求完成后自动发射
    // ok = true 表示后端返回 code==200，data 包含 token/user 信息
    void loginResult(bool ok, const QJsonObject &data, const QString &message);

private slots:
    // QNetworkAccessManager::finished 信号触发，统一处理所有网络回复
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager; // HTTP 请求管理器，整个类共用一个
    static const QString BASE_URL;    // 后端基础地址，改端口只需改一处
};

#endif // APICLIENT_H
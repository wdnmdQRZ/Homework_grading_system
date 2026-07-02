#include "apiclient.h"
#include <QJsonDocument>  // QJsonDocument::toJson() 序列化，fromJson() 反序列化
#include <QNetworkReply>   // 回复数据

// 在 .cpp 里给静态常量赋值
const QString ApiClient::BASE_URL = "http://localhost:8080";

ApiClient::ApiClient(QObject *parent)
    : QObject{parent}
    , m_manager(new QNetworkAccessManager(this))  // 创建管理器，传 this 加入 Qt 对象树
{
    // 把管理器的 finished 信号连到自己的 onReplyFinished 槽
    // 只要任意请求完成，onReplyFinished 就自动被调用
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &ApiClient::onReplyFinished);
}

void ApiClient::login(const QString &username, const QString &password)
{
    QJsonObject body;
    body["username"] = username;
    body["password"] = password;

    QNetworkRequest request(QUrl(BASE_URL + "/api/auth/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_pendingRequest = Login;
    m_manager->post(request, QJsonDocument(body).toJson());
}

void ApiClient::registerUser(const QString &username, const QString &password)
{
    QJsonObject body;
    body["username"] = username;
    body["password"] = password;

    QNetworkRequest request(QUrl(BASE_URL + "/api/auth/register"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    m_pendingRequest = Register;
    m_manager->post(request, QJsonDocument(body).toJson());
}

void ApiClient::onReplyFinished(QNetworkReply *reply)
{
    if (reply->error() != QNetworkReply::NoError) {
        if (m_pendingRequest == Login)
            emit loginResult(false, QJsonObject(), "网络错误：" + reply->errorString());
        else if (m_pendingRequest == Register)
            emit registerResult(false, "网络错误：" + reply->errorString());
        m_pendingRequest = None;
        reply->deleteLater();
        return;
    }

    QByteArray raw = reply->readAll();
    QJsonDocument doc = QJsonDocument::fromJson(raw);
    QJsonObject json = doc.object();

    int code = json["code"].toInt();
    QString message = json["message"].toString();
    QJsonObject data = json["data"].toObject();

    if (m_pendingRequest == Login) {
        emit loginResult(code == 200, data, message);
    } else if (m_pendingRequest == Register) {
        emit registerResult(code == 200, message);
    }

    m_pendingRequest = None;
    reply->deleteLater();
}
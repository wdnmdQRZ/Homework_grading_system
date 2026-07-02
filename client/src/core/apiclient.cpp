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
    // 1. 构造 JSON 请求体
    QJsonObject body;
    body["username"] = username;
    body["password"] = password;

    // 2. 创建请求对象，设置 URL 和 Content-Type
    QNetworkRequest request(QUrl(BASE_URL + "/api/auth/login"));
    request.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");

    // 3. 发出 POST 请求
    //    - QJsonDocument(body).toJson() 把 QJsonObject 序列化成纯 JSON 字符串
    //    - m_manager->post() 是异步的，立刻返回，不阻塞界面
    m_manager->post(request, QJsonDocument(body).toJson());
}

void ApiClient::onReplyFinished(QNetworkReply *reply)
{
    // 1. 先检查有没有网络层面的错误（连不上服务器、DNS 解析失败等）
    if (reply->error() != QNetworkReply::NoError) {
        emit loginResult(false, QJsonObject(),
                         "网络错误：" + reply->errorString());
        reply->deleteLater();  // Qt 建议用 deleteLater 安全释放 reply
        return;
    }

    // 2. 读取服务器返回的全部数据
    QByteArray raw = reply->readAll();

    // 3. 解析 JSON —— fromJson 把字节数组转成 QJsonDocument
    QJsonDocument doc = QJsonDocument::fromJson(raw);
    QJsonObject json = doc.object();

    // 4. 取出后端统一返回格式的三个字段
    int code = json["code"].toInt();          // 200 表示成功
    QString message = json["message"].toString();
    QJsonObject data = json["data"].toObject();

    // 5. 通过信号把结果发射出去
    //    code == 200 → ok = true，否则 false
    emit loginResult(code == 200, data, message);

    reply->deleteLater();
}
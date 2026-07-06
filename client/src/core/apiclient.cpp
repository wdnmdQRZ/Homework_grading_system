#include "apiclient.h"
#include <QJsonDocument>
#include <QJsonArray>
#include <QNetworkReply>
#include <QNetworkProxy>
#include <QHttpMultiPart>
#include <QFileInfo>
#include <QDir>
#include <QFile>
#include <QUrl>

const QString ApiClient::BASE_URL = "http://localhost:8080";

ApiClient::ApiClient(QObject *parent)
    : QObject{parent}
    , m_manager(new QNetworkAccessManager(this))
{
    m_manager->setProxy(QNetworkProxy::NoProxy);
    connect(m_manager, &QNetworkAccessManager::finished,
            this, &ApiClient::onReplyFinished);
}

// ========== 通用请求方法 ==========

QNetworkReply* ApiClient::requestGet(const QString &path, const Callback &cb)
{
    QNetworkRequest req(QUrl(BASE_URL + path));
    if (!m_token.isEmpty())
        req.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());
    auto *reply = m_manager->get(req);
    if (cb) m_callbacks[reply] = cb;
    return reply;
}

QNetworkReply* ApiClient::requestPost(const QString &path, const QJsonDocument &doc, const Callback &cb)
{
    QNetworkRequest req(QUrl(BASE_URL + path));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!m_token.isEmpty())
        req.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());
    auto *reply = m_manager->post(req, doc.toJson());
    if (cb) m_callbacks[reply] = cb;
    return reply;
}

QNetworkReply* ApiClient::requestPut(const QString &path, const QJsonDocument &doc, const Callback &cb)
{
    QNetworkRequest req(QUrl(BASE_URL + path));
    req.setHeader(QNetworkRequest::ContentTypeHeader, "application/json");
    if (!m_token.isEmpty())
        req.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());
    auto *reply = m_manager->put(req, doc.toJson());
    if (cb) m_callbacks[reply] = cb;
    return reply;
}

QNetworkReply* ApiClient::requestDelete(const QString &path, const Callback &cb)
{
    QNetworkRequest req(QUrl(BASE_URL + path));
    if (!m_token.isEmpty())
        req.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());
    auto *reply = m_manager->deleteResource(req);
    if (cb) m_callbacks[reply] = cb;
    return reply;
}

// ========== 认证 ==========

void ApiClient::login(const QString &username, const QString &password)
{
    QJsonObject body;
    body["username"] = username;
    body["password"] = password;
    m_pendingRequest = Login;
    requestPost("/api/auth/login", QJsonDocument(body), Callback());
}

void ApiClient::registerUser(const QString &username, const QString &password)
{
    QJsonObject body;
    body["username"] = username;
    body["password"] = password;
    m_pendingRequest = Register;
    requestPost("/api/auth/register", QJsonDocument(body), Callback());
}

// ========== 作业 ==========

void ApiClient::getHomeworks(const Callback &cb)
{
    requestGet("/api/homeworks", cb);
}

void ApiClient::getHomework(int hid, const Callback &cb)
{
    requestGet(QString("/api/homeworks/%1").arg(hid), cb);
}

void ApiClient::createHomework(const QJsonObject &data, const Callback &cb)
{
    requestPost("/api/homeworks", QJsonDocument(data), cb);
}

void ApiClient::updateHomework(int hid, const QJsonObject &data, const Callback &cb)
{
    requestPut(QString("/api/homeworks/%1").arg(hid), QJsonDocument(data), cb);
}

void ApiClient::deleteHomework(int hid, const Callback &cb)
{
    requestDelete(QString("/api/homeworks/%1").arg(hid), cb);
}

// ========== 提交 ==========

void ApiClient::getSubmissions(int homeworkId, const Callback &cb)
{
    QString path = "/api/submissions";
    if (homeworkId > 0)
        path += QString("?homework_id=%1").arg(homeworkId);
    requestGet(path, cb);
}

void ApiClient::getSubmission(int sid, const Callback &cb)
{
    requestGet(QString("/api/submissions/%1").arg(sid), cb);
}

void ApiClient::submitHomework(int hid, const QString &filePath, const QString &content, const Callback &cb)
{
    if (!content.isEmpty()) {
        // JSON 方式提交文本内容
        QJsonObject body;
        body["homework_id"] = hid;
        body["content"] = content;
        requestPost("/api/submissions", QJsonDocument(body), cb);
        return;
    }

    // 文件上传（multipart）
    QHttpMultiPart *multiPart = new QHttpMultiPart(QHttpMultiPart::FormDataType);

    QHttpPart hwPart;
    hwPart.setHeader(QNetworkRequest::ContentDispositionHeader,
        QVariant(QString("form-data; name=\"homework_id\"")));
    hwPart.setBody(QString::number(hid).toUtf8());
    multiPart->append(hwPart);

    if (!filePath.isEmpty()) {
        QFile *file = new QFile(filePath, multiPart);
        if (file->open(QIODevice::ReadOnly)) {
            QFileInfo fi(filePath);
            QHttpPart filePart;
            filePart.setHeader(QNetworkRequest::ContentTypeHeader,
                QVariant("application/octet-stream"));
            filePart.setHeader(QNetworkRequest::ContentDispositionHeader,
                QVariant("form-data; name=\"file\"; filename=\"" + fi.fileName() + "\""));
            filePart.setBodyDevice(file);
            multiPart->append(filePart);
        }
    }

    QNetworkRequest req(QUrl(BASE_URL + "/api/submissions"));
    if (!m_token.isEmpty())
        req.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());
    auto *reply = m_manager->post(req, multiPart);
    multiPart->setParent(reply);
    if (cb) m_callbacks[reply] = cb;
}

void ApiClient::gradeSubmission(int sid, int score, const QString &comment, const Callback &cb)
{
    QJsonObject body;
    body["score"] = score;
    body["comment"] = comment;
    requestPut(QString("/api/submissions/%1/grade").arg(sid), QJsonDocument(body), cb);
}

// ========== 文件下载 ==========

void ApiClient::downloadFile(const QString &serverPath, const QString &localSavePath)
{
    QUrl url(BASE_URL + "/api/uploads/" + serverPath);
    QNetworkRequest req(url);
    if (!m_token.isEmpty())
        req.setRawHeader("Authorization", "Bearer " + m_token.toUtf8());
    auto *reply = m_manager->get(req);

    connect(reply, &QNetworkReply::finished, this, [this, reply, localSavePath]() {
        reply->deleteLater();
        if (reply->error() != QNetworkReply::NoError) {
            QString errMsg = reply->errorString();
            QByteArray body = reply->readAll();
            if (!body.isEmpty()) {
                QJsonDocument doc = QJsonDocument::fromJson(body);
                QString serverMsg = doc.object()["message"].toString();
                if (!serverMsg.isEmpty()) errMsg = serverMsg;
            }
            emit fileDownloaded(QString(), false, errMsg);
            return;
        }
        QFile file(localSavePath);
        if (file.open(QIODevice::WriteOnly)) {
            file.write(reply->readAll());
            file.close();
            emit fileDownloaded(localSavePath, true, QString());
        } else {
            emit fileDownloaded(QString(), false, "无法创建本地文件：" + localSavePath);
        }
    });
}

// ========== 统一回复处理 ==========

void ApiClient::onReplyFinished(QNetworkReply *reply)
{
    // 1. login/register 兼容旧信号
    if (m_pendingRequest != None) {
        bool ok = (reply->error() == QNetworkReply::NoError);
        if (!ok) {
            if (m_pendingRequest == Login)
                emit loginResult(false, QJsonObject(), "网络错误：" + reply->errorString());
            else
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
        if (m_pendingRequest == Login)
            emit loginResult(code == 200, data, message);
        else
            emit registerResult(code == 200, message);
        m_pendingRequest = None;
        reply->deleteLater();
        return;
    }

    // 2. 回调模式
    if (m_callbacks.contains(reply)) {
        Callback cb = m_callbacks.take(reply);
        if (reply->error() != QNetworkReply::NoError) {
            cb(false, QJsonDocument(), "网络错误：" + reply->errorString());
        } else {
            QByteArray raw = reply->readAll();
            QJsonDocument doc = QJsonDocument::fromJson(raw);
            QJsonObject json = doc.object();
            int code = json["code"].toInt();
            // 提取 data 字段传给回调（后端返回 {code, message, data}）
            QJsonValue dataVal = json["data"];
            QJsonDocument dataDoc;
            if (dataVal.isArray())
                dataDoc = QJsonDocument(dataVal.toArray());
            else if (dataVal.isObject())
                dataDoc = QJsonDocument(dataVal.toObject());
            cb(code == 200, dataDoc, json["message"].toString());
        }
        reply->deleteLater();
    }
}

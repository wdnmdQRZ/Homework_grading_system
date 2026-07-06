#ifndef APICLIENT_H
#define APICLIENT_H

#include <QObject>
#include <QNetworkAccessManager>
#include <QNetworkReply>
#include <QJsonObject>
#include <QJsonDocument>
#include <QHash>
#include <functional>
#include <QTemporaryFile>

class ApiClient : public QObject
{
    Q_OBJECT
public:
    using Callback = std::function<void(bool ok, const QJsonDocument &data, const QString &msg)>;

    explicit ApiClient(QObject *parent = nullptr);

    void setToken(const QString &token) { m_token = token; }
    QString token() const { return m_token; }

    // ===== 认证 =====
    void login(const QString &username, const QString &password);
    void registerUser(const QString &username, const QString &password);

    // ===== 作业 =====
    void getHomeworks(const Callback &cb);
    void getHomework(int hid, const Callback &cb);
    void createHomework(const QJsonObject &data, const Callback &cb);
    void updateHomework(int hid, const QJsonObject &data, const Callback &cb);
    void deleteHomework(int hid, const Callback &cb);

    // ===== 提交 =====
    void getSubmissions(int homeworkId, const Callback &cb);
    void getSubmission(int sid, const Callback &cb);
    void submitHomework(int hid, const QString &filePath, const QString &content, const Callback &cb);
    void gradeSubmission(int sid, int score, const QString &comment, const Callback &cb);

    // ===== 文件下载 =====
    void downloadFile(const QString &serverPath, const QString &localSavePath);

signals:
    void loginResult(bool ok, const QJsonObject &data, const QString &message);
    void registerResult(bool ok, const QString &message);
    void fileDownloaded(const QString &localPath, bool ok, const QString &errorMsg);

private slots:
    void onReplyFinished(QNetworkReply *reply);

private:
    QNetworkAccessManager *m_manager;
    QHash<QNetworkReply*, Callback> m_callbacks;
    QString m_token;
    static const QString BASE_URL;

    enum RequestType { None, Login, Register };
    RequestType m_pendingRequest = None;

    QNetworkReply* requestGet(const QString &path, const Callback &cb);
    QNetworkReply* requestPost(const QString &path, const QJsonDocument &doc, const Callback &cb);
    QNetworkReply* requestPut(const QString &path, const QJsonDocument &doc, const Callback &cb);
    QNetworkReply* requestDelete(const QString &path, const Callback &cb);
};

#endif // APICLIENT_H

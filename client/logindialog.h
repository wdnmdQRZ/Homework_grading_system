#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QJsonObject>

namespace Ui {
class logindialog;
}

class ApiClient;
class loginpage;
class registerpage;

class logindialog : public QDialog
{
    Q_OBJECT

public:
    explicit logindialog(QWidget *parent = nullptr);
    ~logindialog();

    QString token() const;
    QString username() const;
    QString role() const;

private slots:
    void onLoginRequested(const QString &username, const QString &password);
    void onRegisterRequested(const QString &username, const QString &password);

private:
    Ui::logindialog *ui;
    ApiClient *m_apiClient;
    loginpage *m_loginPage;
    registerpage *m_registerPage;

    QString m_token;
    QString m_username;
    QString m_role;
};

#endif // LOGINDIALOG_H

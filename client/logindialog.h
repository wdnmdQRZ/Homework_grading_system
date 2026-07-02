#ifndef LOGINDIALOG_H
#define LOGINDIALOG_H

#include <QDialog>
#include <QJsonObject>

namespace Ui {
class logindialog;
}

class ApiClient;

class logindialog : public QDialog
{
    Q_OBJECT

public:
    explicit logindialog(QWidget *parent = nullptr);
    ~logindialog();

    // 获取登录结果
    QString token() const;
    QString username() const;
    QString role() const;

private slots:
    void onLoginClicked();

private:
    Ui::logindialog *ui;
    ApiClient *m_apiClient;

    // 登录成功时保存的数据
    QString m_token;
    QString m_username;
    QString m_role;
};

#endif // LOGINDIALOG_H
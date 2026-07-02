#include "logindialog.h"
#include "ui_logindialog.h"
#include "src/core/apiclient.h"
#include <QApplication>
#include <QStyle>
#include <QMessageBox>

logindialog::logindialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::logindialog)
    , m_apiClient(new ApiClient(this))
{
    ui->setupUi(this);
    setWindowTitle("作业管理系统");
    setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));

    ui->labelWarning->setText("");

    connect(ui->btnLogin, &QPushButton::clicked,
            this, &logindialog::onLoginClicked);

    connect(m_apiClient, &ApiClient::loginResult,
            this, [this](bool ok, const QJsonObject &data, const QString &message) {
        if (ok) {
            m_token = data["token"].toString();
            QJsonObject user = data["user"].toObject();
            m_username = user["username"].toString();
            m_role = user["role"].toString();
            accept();
        } else {
            ui->labelWarning->setText(message);
            ui->labelWarning->setVisible(true);
        }
    });
}

logindialog::~logindialog()
{
    delete ui;
}

QString logindialog::token() const    { return m_token; }
QString logindialog::username() const { return m_username; }
QString logindialog::role() const     { return m_role; }

void logindialog::onLoginClicked()
{
    ui->labelWarning->setVisible(false);

    QString username = ui->editUsername->text().trimmed();
    QString password = ui->editPassword->text();

    if (username.isEmpty() || password.isEmpty()) {
        ui->labelWarning->setText("用户名和密码不能为空");
        ui->labelWarning->setVisible(true);
        return;
    }

    m_apiClient->login(username, password);
}
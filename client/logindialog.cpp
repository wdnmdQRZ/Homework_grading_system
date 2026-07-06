#include "logindialog.h"
#include "ui_logindialog.h"
#include "loginpage.h"
#include "registerpage.h"
#include "src/core/apiclient.h"
#include <QApplication>
#include <QStyle>

logindialog::logindialog(QWidget *parent)
    : QDialog(parent)
    , ui(new Ui::logindialog)
    , m_apiClient(nullptr)
    , m_loginPage(new loginpage)
    , m_registerPage(new registerpage)
{
    ui->setupUi(this);
    setWindowTitle("作业管理系统");
    setWindowIcon(QApplication::style()->standardIcon(QStyle::SP_ComputerIcon));

    // 把两个页面添加到 QStackedWidget
    ui->stackedWidget->addWidget(m_loginPage);    // Page 0
    ui->stackedWidget->addWidget(m_registerPage); // Page 1

    // === 页面切换 ===
    connect(m_loginPage, &loginpage::goToRegister, this, [this]() {
        ui->stackedWidget->setCurrentIndex(1);
        m_registerPage->clearFields();
    });

    connect(m_registerPage, &registerpage::goToLogin, this, [this]() {
        ui->stackedWidget->setCurrentIndex(0);
        m_loginPage->clearFields();
    });
}

logindialog::~logindialog()
{
    delete ui;
}

QString logindialog::token() const    { return m_token; }
QString logindialog::username() const { return m_username; }
QString logindialog::role() const     { return m_role; }

void logindialog::setApiClient(ApiClient *api)
{
    m_apiClient = api;

    // === 登录请求 ===
    connect(m_loginPage, &loginpage::loginRequested,
            this, &logindialog::onLoginRequested);

    connect(m_apiClient, &ApiClient::loginResult,
            this, [this](bool ok, const QJsonObject &data, const QString &message) {
        if (ok) {
            m_token = data["token"].toString();
            m_apiClient->setToken(m_token);
            QJsonObject user = data["user"].toObject();
            m_username = user["username"].toString();
            m_role = user["role"].toString();
            accept();
        } else {
            m_loginPage->showWarning(message);
        }
    });

    // === 注册请求 ===
    connect(m_registerPage, &registerpage::registerRequested,
            this, &logindialog::onRegisterRequested);

    connect(m_apiClient, &ApiClient::registerResult,
            this, [this](bool ok, const QString &message) {
        if (ok) {
            ui->stackedWidget->setCurrentIndex(0);
            m_loginPage->clearFields();
            m_loginPage->setUsername(m_registerPage->username());
            m_loginPage->showWarning("注册成功，请登录");
        } else {
            m_registerPage->showWarning(message);
        }
    });
}

void logindialog::onLoginRequested(const QString &username, const QString &password)
{
    m_apiClient->login(username, password);
}

void logindialog::onRegisterRequested(const QString &username, const QString &password)
{
    m_apiClient->registerUser(username, password);
}

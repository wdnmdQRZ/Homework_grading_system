#include "loginpage.h"
#include "ui_loginpage.h"

loginpage::loginpage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::loginpage)
{
    ui->setupUi(this);
    ui->labelWarning->setText("");

    connect(ui->btnLogin, &QPushButton::clicked,
            this, &loginpage::onLoginClicked);

    connect(ui->btnToRegister, &QPushButton::clicked,
            this, &loginpage::goToRegister);
}

loginpage::~loginpage()
{
    delete ui;
}

QString loginpage::username() const { return ui->editUsername->text().trimmed(); }
QString loginpage::password() const { return ui->editPassword->text(); }

void loginpage::showWarning(const QString &msg)
{
    ui->labelWarning->setText(msg);
}

void loginpage::clearFields()
{
    ui->editUsername->clear();
    ui->editPassword->clear();
    ui->labelWarning->setText("");
}

void loginpage::setUsername(const QString &name)
{
    ui->editUsername->setText(name);
}

void loginpage::onLoginClicked()
{
    ui->labelWarning->setText("");

    QString user = ui->editUsername->text().trimmed();
    QString pass = ui->editPassword->text();

    if (user.isEmpty() || pass.isEmpty()) {
        ui->labelWarning->setText("用户名和密码不能为空");
        return;
    }

    emit loginRequested(user, pass);
}

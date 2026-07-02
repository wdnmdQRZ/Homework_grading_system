#include "registerpage.h"
#include "ui_registerpage.h"

registerpage::registerpage(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::registerpage)
{
    ui->setupUi(this);
    ui->labelWarning->setText("");

    connect(ui->btnRegister, &QPushButton::clicked,
            this, &registerpage::onRegisterClicked);

    connect(ui->btnToLogin, &QPushButton::clicked,
            this, &registerpage::goToLogin);
}

registerpage::~registerpage()
{
    delete ui;
}

void registerpage::showWarning(const QString &msg)
{
    ui->labelWarning->setText(msg);
}

void registerpage::clearFields()
{
    ui->editUsername->clear();
    ui->editPassword->clear();
    ui->editConfirmPassword->clear();
    ui->labelWarning->setText("");
}

void registerpage::setUsername(const QString &name)
{
    ui->editUsername->setText(name);
}

QString registerpage::username() const
{
    return ui->editUsername->text().trimmed();
}

void registerpage::onRegisterClicked()
{
    ui->labelWarning->setText("");

    QString user = ui->editUsername->text().trimmed();
    QString pass = ui->editPassword->text();
    QString confirm = ui->editConfirmPassword->text();

    if (user.isEmpty() || pass.isEmpty()) {
        ui->labelWarning->setText("用户名和密码不能为空");
        return;
    }
    if (pass.length() < 6) {
        ui->labelWarning->setText("密码不能少于6位");
        return;
    }
    if (pass.contains(' ')) {
        ui->labelWarning->setText("密码不能包含空格");
        return;
    }
    if (pass != confirm) {
        ui->labelWarning->setText("两次密码输入不一致");
        return;
    }

    emit registerRequested(user, pass);
}

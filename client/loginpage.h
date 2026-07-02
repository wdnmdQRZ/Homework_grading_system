#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>

namespace Ui {
class loginpage;
}

class loginpage : public QWidget
{
    Q_OBJECT

public:
    explicit loginpage(QWidget *parent = nullptr);
    ~loginpage();

    QString username() const;
    QString password() const;
    void showWarning(const QString &msg);
    void clearFields();
    void setUsername(const QString &name);

signals:
    void loginRequested(const QString &username, const QString &password);
    void goToRegister();

private slots:
    void onLoginClicked();

private:
    Ui::loginpage *ui;
};

#endif // LOGINPAGE_H

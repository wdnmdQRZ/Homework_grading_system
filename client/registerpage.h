#ifndef REGISTERPAGE_H
#define REGISTERPAGE_H

#include <QWidget>

namespace Ui {
class registerpage;
}

class registerpage : public QWidget
{
    Q_OBJECT

public:
    explicit registerpage(QWidget *parent = nullptr);
    ~registerpage();

    void showWarning(const QString &msg);
    void clearFields();
    void setUsername(const QString &name);
    QString username() const;

signals:
    void registerRequested(const QString &username, const QString &password);
    void goToLogin();

private slots:
    void onRegisterClicked();

private:
    Ui::registerpage *ui;
};

#endif // REGISTERPAGE_H

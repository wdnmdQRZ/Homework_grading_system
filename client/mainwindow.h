#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QStackedWidget;   // 前置声明
class studentpanel;     // 前置声明
class teacherpanel;     // 前置声明

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    // 登录成功后调用：传入 role 决定显示哪个面板
    void setRole(const QString &role);

private:
    Ui::MainWindow *ui;
    QStackedWidget *m_stackedWidget;
    studentpanel *m_studentPanel;
    teacherpanel *m_teacherPanel;
};

#endif // MAINWINDOW_H

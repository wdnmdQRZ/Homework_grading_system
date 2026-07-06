#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

class QStackedWidget;
class studentpanel;
class teacherpanel;
class ApiClient;

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

    void setApiClient(ApiClient *api);
    void setRole(const QString &role);
    void setUsername(const QString &username);

private:
    Ui::MainWindow *ui;
    QStackedWidget *m_stackedWidget;
    studentpanel *m_studentPanel;
    teacherpanel *m_teacherPanel;
    ApiClient *m_apiClient;
};

#endif // MAINWINDOW_H

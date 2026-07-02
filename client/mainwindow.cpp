#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "studentpanel.h"
#include "teacherpanel.h"
#include <QStackedWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    // 创建页面切换器
    m_stackedWidget = new QStackedWidget(this);
    m_studentPanel = new studentpanel(this);
    m_teacherPanel = new teacherpanel(this);

    m_stackedWidget->addWidget(m_studentPanel);  // index 0
    m_stackedWidget->addWidget(m_teacherPanel);  // index 1

    setCentralWidget(m_stackedWidget);
    m_stackedWidget->setCurrentIndex(0);  // 默认学生页
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::setRole(const QString &role)
{
    if (role == "teacher")
        m_stackedWidget->setCurrentIndex(1);
    else
        m_stackedWidget->setCurrentIndex(0);
}
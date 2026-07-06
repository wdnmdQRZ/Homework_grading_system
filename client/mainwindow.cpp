#include "mainwindow.h"
#include "ui_mainwindow.h"
#include "studentpanel.h"
#include "teacherpanel.h"
#include "src/core/apiclient.h"
#include <QStackedWidget>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
    , m_apiClient(nullptr)
{
    ui->setupUi(this);

    m_stackedWidget = new QStackedWidget(this);
    m_studentPanel = new studentpanel(this);
    m_teacherPanel = new teacherpanel(this);

    m_stackedWidget->addWidget(m_studentPanel);
    m_stackedWidget->addWidget(m_teacherPanel);

    setCentralWidget(m_stackedWidget);
    m_stackedWidget->setCurrentIndex(0);

    connect(m_studentPanel, &studentpanel::logoutRequested, this, &MainWindow::close);
    connect(m_teacherPanel, &teacherpanel::logoutRequested, this, &MainWindow::close);
}

MainWindow::~MainWindow() { delete ui; }

void MainWindow::setApiClient(ApiClient *api)
{
    m_apiClient = api;
    m_studentPanel->setApiClient(api);
    m_teacherPanel->setApiClient(api);
}

void MainWindow::setRole(const QString &role)
{
    if (role == "teacher")
        m_stackedWidget->setCurrentIndex(1);
    else
        m_stackedWidget->setCurrentIndex(0);
}

void MainWindow::setUsername(const QString &username)
{
    if (m_stackedWidget->currentIndex() == 0)
        m_studentPanel->setUsername(username);
    else
        m_teacherPanel->setUsername(username);
}

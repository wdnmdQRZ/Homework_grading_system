#include "mainwindow.h"
#include "logindialog.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    // 加载样式表
    QFile styleFile(":/style.qss");
    if (styleFile.open(QIODevice::ReadOnly)) {
        a.setStyleSheet(styleFile.readAll());
        styleFile.close();
    }

    // 先显示登录窗口
    logindialog dlg;
    if (dlg.exec() == QDialog::Accepted) {
        // 登录成功，打开主窗口并根据角色切换页面
        MainWindow w;
        w.setRole(dlg.role());   // 传入 role："teacher" 或 "student"
        w.show();
        return a.exec();
    } else {
        // 登录取消/失败，退出程序
        return 0;
    }
}
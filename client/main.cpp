#include "mainwindow.h"
#include "logindialog.h"
#include "src/core/apiclient.h"
#include <QApplication>
#include <QFile>

int main(int argc, char *argv[])
{
    QApplication a(argc, argv);

    QFile styleFile(":/style.qss");
    if (styleFile.open(QIODevice::ReadOnly)) {
        a.setStyleSheet(styleFile.readAll());
        styleFile.close();
    }

    while (true) {
        ApiClient api;
        logindialog dlg;
        dlg.setApiClient(&api);
        if (dlg.exec() == QDialog::Accepted) {
            MainWindow w;
            w.setApiClient(&api);
            w.setRole(dlg.role());
            w.setUsername(dlg.username());
            w.show();
            a.exec();
        } else {
            break;
        }
    }

    return 0;
}

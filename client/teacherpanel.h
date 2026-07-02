#ifndef TEACHERPANEL_H
#define TEACHERPANEL_H

#include <QWidget>

namespace Ui {
class teacherpanel;
}

class teacherpanel : public QWidget
{
    Q_OBJECT

public:
    explicit teacherpanel(QWidget *parent = nullptr);
    ~teacherpanel();

private:
    Ui::teacherpanel *ui;
};

#endif // TEACHERPANEL_H

#ifndef STUDENTPANEL_H
#define STUDENTPANEL_H

#include <QWidget>

namespace Ui {
class studentpanel;
}

class studentpanel : public QWidget
{
    Q_OBJECT

public:
    explicit studentpanel(QWidget *parent = nullptr);
    ~studentpanel();

private:
    Ui::studentpanel *ui;
};

#endif // STUDENTPANEL_H

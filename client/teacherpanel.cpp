#include "teacherpanel.h"
#include "ui_teacherpanel.h"

teacherpanel::teacherpanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::teacherpanel)
{
    ui->setupUi(this);
}

teacherpanel::~teacherpanel()
{
    delete ui;
}

#include "studentpanel.h"
#include "ui_studentpanel.h"

studentpanel::studentpanel(QWidget *parent)
    : QWidget(parent)
    , ui(new Ui::studentpanel)
{
    ui->setupUi(this);
}

studentpanel::~studentpanel()
{
    delete ui;
}

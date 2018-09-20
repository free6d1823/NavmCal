#include "control2.h"
#include "ui_control2.h"

Control2::Control2(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Control2)
{
    ui->setupUi(this);
}

Control2::~Control2()
{
    delete ui;
}

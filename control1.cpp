#include "control1.h"
#include "ui_control1.h"

Control1::Control1(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Control1)
{
    ui->setupUi(this);
}

Control1::~Control1()
{
    delete ui;
}

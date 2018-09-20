#include "control0.h"
#include "ui_control0.h"

Control0::Control0(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Control0)
{
    ui->setupUi(this);
}

Control0::~Control0()
{
    delete ui;
}

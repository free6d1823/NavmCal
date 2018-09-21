#include "controlpanel.h"
#include "ui_control4.h"
#include "mainwindow.h"
#include "common.h"
Control4::Control4(TYPE id,QWidget *parent) :
    ControlPanel(id, parent),
    ui(new Ui::Control4)
{
    ui->setupUi(this);
}

Control4::~Control4()
{
    delete ui;
}
///
/// \brief start to prepare things when this step is started
///
void Control4::start()
{
    gpMainWin->changeView(mId);
}
///
/// \brief stop to do things before this step is fnished
///
void Control4::stop()
{

}

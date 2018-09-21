#include "controlpanel.h"
#include "ui_control2.h"
#include "mainwindow.h"
#include "common.h"
Control2::Control2(TYPE id,QWidget *parent) :
    ControlPanel(id, parent),
    ui(new Ui::Control2)
{
    ui->setupUi(this);
}

Control2::~Control2()
{
    delete ui;
}
///
/// \brief start to prepare things when this step is started
///
void Control2::start()
{
    gpMainWin->changeView(mId);
}
///
/// \brief stop to do things before this step is fnished
///
void Control2::stop()
{

}

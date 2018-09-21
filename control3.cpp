#include "controlpanel.h"
#include "ui_control3.h"
#include "mainwindow.h"
#include "common.h"
Control3::Control3(TYPE id,QWidget *parent) :
    ControlPanel(id, parent),
    ui(new Ui::Control3)
{
    ui->setupUi(this);
}

Control3::~Control3()
{
    delete ui;
}
///
/// \brief start to prepare things when this step is started
///
void Control3::start()
{
    gpMainWin->changeView(mId);
}
///
/// \brief stop to do things before this step is fnished
///
void Control3::stop()
{

}

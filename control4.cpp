#include "controlpanel.h"
#include "ui_control4.h"
#include "mainwindow.h"
#include "common.h"
Control4::Control4(TYPE id,QWidget *parent) :
    ControlPanel(id, parent),
    ui(new Ui::Control4)
{
    ui->setupUi(this);

    connect(ui->btnSave, SIGNAL(clicked()), SLOT(onSave()));
    connect(ui->checkShowCam0, SIGNAL(toggled(bool)), SLOT(onShowCam0(bool)));
    connect(ui->checkShowCam1, SIGNAL(toggled(bool)), SLOT(onShowCam1(bool)));
    connect(ui->checkShowCam2, SIGNAL(toggled(bool)), SLOT(onShowCam2(bool)));
    connect(ui->checkShowCam3, SIGNAL(toggled(bool)), SLOT(onShowCam3(bool)));
    connect(ui->checkShowCar, SIGNAL(toggled(bool)), SLOT(onShowCar(bool)));
//    connect(ui->dirSlider, SIGNAL(valueChanged(int value)), this, SLOT(onSteerChanged(int)));

    connect(ui->steer, SIGNAL(valueChanged(int)),
            this, SLOT(setDirValue(int)));

}

Control4::~Control4()
{
    delete ui;
}
void Control4::onShowCam0(bool show)
{
    gpMainWin->sendMessage(MESSAGE_VIEW_SHOW_CAMERA0, (show?1:0));
}
void Control4::onShowCam1(bool show)
{
    gpMainWin->sendMessage(MESSAGE_VIEW_SHOW_CAMERA1, (show?1:0));
}
void Control4::onShowCam2(bool show)
{
    gpMainWin->sendMessage(MESSAGE_VIEW_SHOW_CAMERA2, (show?1:0));
}
void Control4::onShowCam3(bool show)
{
    gpMainWin->sendMessage(MESSAGE_VIEW_SHOW_CAMERA3, (show?1:0));
}
void Control4::onShowCar(bool show)
{
    gpMainWin->sendMessage(MESSAGE_VIEW_SHOW_FEATUREPOINTS, (show?1:0));
}
void Control4::setDirValue(int value)
{
    gpMainWin->sendMessage(MESSAGE_VIEW_STEER_CHANGED100, value);
}

///
/// \brief start to prepare things when this step is started
///
void Control4::start()
{
    gpMainWin->changeView(mPanelTypeId);
    gpMainWin->sendMessage(MESSAGE_VIEW_DO_STITCHING, 0);
    gpMainWin->sendMessage(MESSAGE_VIEW_SCALE_1000IMAGE,
                            (long)(gpMainWin->getCurrentZoomFactor()*1000.0));



}
///
/// \brief stop to do things before this step is fnished
///
void Control4::stop()
{

}
void Control4::onSave()
{
    gpMainWin->onFileSaveAs();
}

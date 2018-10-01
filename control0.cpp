#include "controlpanel.h"
#include "ui_control0.h"
#include "mainwindow.h"
#include "common.h"
#include "./imglab/ImgProcess.h"
#include <QMessageBox>
Control0::Control0(TYPE id,QWidget *parent) :
    ControlPanel(id, parent),
    ui(new Ui::Control0)
{
    ui->setupUi(this);
}

Control0::~Control0()
{
    delete ui;
}
///
/// \brief start to prepare things when this step is started
///
void Control0::start()
{
    if (gpInputImage){
        QImage* newImage = new QImage(gpInputImage->buffer,
                gpInputImage->width, gpInputImage->height, QImage::Format_RGBA8888);

        if (newImage->isNull()) {
            QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                     tr("Cannot create image"));
            return;
        }

        gpMainWin->setImage(gpInputImage);
    }
    gpMainWin->changeView(mPanelTypeId);
    gpMainWin->sendMessage(MESSAGE_VIEW_SCALE_1000IMAGE,
                            (long)(gpMainWin->getCurrentZoomFactor()*1000.0));

}
///
/// \brief stop to do things before this step is fnished
///
void Control0::stop()
{

}

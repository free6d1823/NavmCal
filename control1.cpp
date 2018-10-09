#include "controlpanel.h"
#include "ui_control1.h"
#include "mainwindow.h"
#include "common.h"
#include <stdlib.h>

Control1::Control1(TYPE id,QWidget *parent) :
    ControlPanel(id, parent),
    ui(new Ui::Control1)
{
    mEditMode = EM_NONE;
    ui->setupUi(this);
    createUi();
    updateUi();
    ui->btnRoi->setEnabled(false);
    ui->btnManual->setEnabled(false);
    ui->btnAuto->setEnabled(false);
    ui->btnLink->setEnabled(false);
    ui->sliderThreshold->setEnabled(false);
    ui->btnAccept->setEnabled(false);
    ui->btnReset->setEnabled(false);
}

Control1::~Control1()
{
    delete ui;
}

void Control1::createUi()
{
    connect(ui->frontCam, SIGNAL(clicked()), SLOT(onCamera0()));
    connect(ui->rightCam, SIGNAL(clicked()), SLOT(onCamera1()));
    connect(ui->rearCam, SIGNAL(clicked()), SLOT(onCamera2()));
    connect(ui->leftCam, SIGNAL(clicked()), SLOT(onCamera3()));
    connect(ui->checkBox, SIGNAL(toggled(bool)), SLOT(onShowFp(bool)));
    connect(ui->btnAuto, SIGNAL(clicked()), SLOT(onAutoMode()));
    connect(ui->btnRoi , SIGNAL(clicked()), SLOT(onRoiMode()));
    connect(ui->btnManual , SIGNAL(clicked()), SLOT(onManualMode()));
    connect(ui->btnLink , SIGNAL(clicked()), SLOT(onLinkMode()));
    connect(ui->btnAccept , SIGNAL(clicked()), SLOT(onAccept()));
    connect(ui->btnReset , SIGNAL(clicked()), SLOT(onReset()));
    connect(ui->sliderThreshold, SIGNAL(valueChanged(int)),
            this, SLOT(setThreshold(int)));


}
void Control1::updateUi()
{
        ui->btnRoi->setChecked(mEditMode == EM_ROI);
        ui->btnAuto->setChecked(mEditMode == EM_AUTO);
        ui->btnManual->setChecked(mEditMode == EM_MANUAL);
        ui->btnLink->setChecked(mEditMode == EM_LINK);

}

/*   Image location vs id :
 *   front(0) | right(1) |
 *   left(3)  | rear(2)  |
 */
///
/// \brief LoadImagebyArea create an image of the camera, duplicate buffer
/// \param nCamId
/// \return
///
static nfImage* LoadImagebyArea(int nCamId)
{
    if(!gpInputImage)
        return NULL;
    unsigned char* pIn;
    int nOutWidth = gpInputImage->width/2;
    int nOutHeight = gpInputImage->height/2;

    switch (nCamId) {
    case 0:
        pIn = gpInputImage->buffer;
        break;
    case 1:
        pIn = gpInputImage->buffer+ nOutWidth*4;
        break;
    case 3:
        pIn = gpInputImage->buffer + gpInputImage->stride*nOutHeight;
        break;
    case 2:
    default:
        pIn = gpInputImage->buffer + gpInputImage->stride*nOutHeight+ nOutWidth*4;
        break;
    }
    nfImage* pOut = nfImage::create(nOutWidth, nOutHeight, 4);
    nfPByte pDes = pOut->buffer;
    nfPByte pSrc = pIn;
    for(unsigned int i=0; i<pOut->height; i++){
        memcpy(pDes, pSrc, nOutWidth*4);
        pDes += pOut->stride;
        pSrc += gpInputImage->stride;
    }
    return pOut;
}

void Control1::loadCamera(int cam)
{
    mCamId = cam;
    if (gpInputImage){
        nfImage* pImage = LoadImagebyArea(cam);
        gpMainWin->setImage(pImage);
        nfImage::dettach(&pImage);//releas buffer to receiver
        gpMainWin->sendMessage(MESSAGE_VIEW_SET_CAMERAID, (long) cam);
        gpMainWin->scaleImage(1);//update scrollbar and image to keep previous zoomfactor
    }
    char buffer[64];
    sprintf(buffer, "Max Feature Points %d", gpTexProcess->mAreaSettings[mCamId].nFpCounts);
    ui->labelLink->setText(buffer);
}

void Control1::onCamera0(){
    ui->frontCam->setChecked(true);
    ui->rightCam->setChecked(false);
    ui->rearCam->setChecked(false);
    ui->leftCam->setChecked(false);
    loadCamera(0);
}
void Control1::onCamera1(){
    ui->frontCam->setChecked(false);
    ui->rightCam->setChecked(true);
    ui->rearCam->setChecked(false);
    ui->leftCam->setChecked(false);
    loadCamera(1);
}
void Control1::onCamera2(){
    ui->frontCam->setChecked(false);
    ui->rightCam->setChecked(false);
    ui->rearCam->setChecked(true);
    ui->leftCam->setChecked(false);
    loadCamera(2);
}
void Control1::onCamera3(){
    ui->frontCam->setChecked(false);
    ui->rightCam->setChecked(false);
    ui->rearCam->setChecked(false);
    ui->leftCam->setChecked(true);
    loadCamera(3);
}
///
/// \brief start to prepare things when this step is started
///
void Control1::start()
{
    gpMainWin->changeView(mPanelTypeId);
    onCamera0();//after this iamge is available, zoom to current ZoomFactor
    gpMainWin->sendMessage(MESSAGE_VIEW_SCALE_1000IMAGE,
                            (long)(gpMainWin->getCurrentZoomFactor()*1000.0));

    gpMainWin->sendMessage(MESSAGE_VIEW_SHOW_FEATUREPOINTS,
                           (ui->checkBox->checkState() == Qt::Checked?1:0));

}
///
/// \brief stop to do things before this step is fnished
///
void Control1::stop()
{

}

void Control1::onShowFp(bool show)
{
    gpMainWin->sendMessage(MESSAGE_VIEW_SHOW_FEATUREPOINTS, (show?1:0));
    if (show == false){
        mEditMode = EM_NONE;
        ui->btnRoi->setEnabled(false);
        ui->btnManual->setEnabled(false);
        ui->btnAuto->setEnabled(false);
        ui->btnLink->setEnabled(false);
        ui->sliderThreshold->setEnabled(false);
        ui->btnAccept->setEnabled(false);
        ui->btnReset->setEnabled(false);
    }else {
        ui->btnRoi->setEnabled(true);
        ui->btnManual->setEnabled(true);
        ui->btnAuto->setEnabled(true);
        ui->btnLink->setEnabled(true);
        ui->sliderThreshold->setEnabled(true);
        ui->btnAccept->setEnabled(true);
        ui->btnReset->setEnabled(true);
    }
    updateUi();
}
void Control1::onAutoMode()
{
    mEditMode = EM_AUTO;
    gpMainWin->sendMessage(MESSAGE_VIEW_SET_AUTO_MODE,
                           ui->sliderThreshold->value());
    updateUi();
}

void Control1::onRoiMode()
{
    mEditMode = EM_ROI;
    updateUi();
    gpMainWin->sendMessage(MESSAGE_VIEW_SET_ROI_MODE, 0);
}
void Control1::onManualMode()
{
    mEditMode = EM_MANUAL;
    updateUi();
    gpMainWin->sendMessage(MESSAGE_VIEW_SET_MANUAL_MODE, 0);
}
void Control1::onLinkMode()
{
    mEditMode = EM_LINK;
    updateUi();
    gpMainWin->sendMessage(MESSAGE_VIEW_SET_LINK_MODE, 0);
}
void Control1::onAccept()
{
    gpMainWin->sendMessage(MESSAGE_VIEW_DO_ACCEPT, 0);
}
void Control1::onReset()
{
    mEditMode = EM_NONE;
    updateUi();
    gpMainWin->sendMessage(MESSAGE_VIEW_DO_RESET, 0);

}

void Control1::setThreshold(int value)
{
    char buffer[32];
    sprintf(buffer, "%d", value);
    ui->labelThreshold->setText(buffer);
}

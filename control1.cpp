#include "controlpanel.h"
#include "ui_control1.h"
#include "mainwindow.h"
#include "common.h"

Control1::Control1(TYPE id,QWidget *parent) :
    ControlPanel(id, parent),
    ui(new Ui::Control1)
{
    ui->setupUi(this);
    createUi();
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
    connect(ui->pushButton, SIGNAL(clicked()), SLOT(onAutoDetection()));

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
    }
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
}
void Control1::onAutoDetection()
{
    gpMainWin->sendMessage(MESSAGE_VIEW_DO_AUTODETECTION, 0);
}

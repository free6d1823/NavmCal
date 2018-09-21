#include "controlpanel.h"
#include "ui_control2.h"
#include "mainwindow.h"
#include "common.h"
Control2::Control2(TYPE id,QWidget *parent) :
    ControlPanel(id, parent),
    ui(new Ui::Control2)
{
    ui->setupUi(this);
    connect(ui->frontCam, SIGNAL(clicked()), SLOT(onCamera0()));
    connect(ui->rightCam, SIGNAL(clicked()), SLOT(onCamera1()));
    connect(ui->rearCam, SIGNAL(clicked()), SLOT(onCamera2()));
    connect(ui->leftCam, SIGNAL(clicked()), SLOT(onCamera3()));
}

Control2::~Control2()
{
    delete ui;
}
void Control2::createUi()
{
    connect(ui->frontCam, SIGNAL(clicked()), SLOT(onCamera0()));
    connect(ui->rightCam, SIGNAL(clicked()), SLOT(onCamera1()));
    connect(ui->rearCam, SIGNAL(clicked()), SLOT(onCamera2()));
    connect(ui->leftCam, SIGNAL(clicked()), SLOT(onCamera3()));
}
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
void Control2::loadCamera(int cam)
{
    if (gpInputImage){
        nfImage* pImage = LoadImagebyArea(cam);
        gpMainWin->setImage(pImage);
        nfImage::dettach(&pImage);//releas buffer to QImage
        gpMainWin->sendMessage(MESSAGE_VIEW_SET_CAMERAID, (long) cam);
    }
}

void Control2::onCamera0(){
    ui->frontCam->setChecked(true);
    ui->rightCam->setChecked(false);
    ui->rearCam->setChecked(false);
    ui->leftCam->setChecked(false);
    loadCamera(0);
}
void Control2::onCamera1(){
    ui->frontCam->setChecked(false);
    ui->rightCam->setChecked(true);
    ui->rearCam->setChecked(false);
    ui->leftCam->setChecked(false);
    loadCamera(1);
}
void Control2::onCamera2(){
    ui->frontCam->setChecked(false);
    ui->rightCam->setChecked(false);
    ui->rearCam->setChecked(true);
    ui->leftCam->setChecked(false);
    loadCamera(2);
}
void Control2::onCamera3(){
    ui->frontCam->setChecked(false);
    ui->rightCam->setChecked(false);
    ui->rearCam->setChecked(false);
    ui->leftCam->setChecked(true);
    loadCamera(3);
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

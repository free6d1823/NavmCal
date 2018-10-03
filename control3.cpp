#include "controlpanel.h"
#include "ui_control3.h"
#include "mainwindow.h"
#include "common.h"
Control3::Control3(TYPE id,QWidget *parent) :
    ControlPanel(id, parent),
    ui(new Ui::Control3)
{
    ui->setupUi(this);
    createUi();
}

Control3::~Control3()
{
    delete ui;
}

void Control3::createUi()
{
    connect(ui->frontCam, SIGNAL(clicked()), SLOT(onCamera0()));
    connect(ui->rightCam, SIGNAL(clicked()), SLOT(onCamera1()));
    connect(ui->rearCam, SIGNAL(clicked()), SLOT(onCamera2()));
    connect(ui->leftCam, SIGNAL(clicked()), SLOT(onCamera3()));
    connect(ui->checkShowFp, SIGNAL(toggled(bool)), SLOT(onShowFp(bool)));
    connect(ui->checkShowGrideLines, SIGNAL(toggled(bool)), SLOT(onShowGrideLines(bool)));

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
void Control3::loadCamera(int cam)
{
    mCamId = cam;
    if (gpInputImage){
        nfImage* pImage = LoadImagebyArea(cam);
        gpMainWin->setImage(pImage);
        nfImage::dettach(&pImage);//releas buffer to QImage
        gpMainWin->sendMessage(MESSAGE_VIEW_SET_CAMERAID, (long) cam);
    }
    gpMainWin->scaleImage(1);//update scrollbar and image to keep previous zoomfactor
}

void Control3::onCamera0(){
    ui->frontCam->setChecked(true);
    ui->rightCam->setChecked(false);
    ui->rearCam->setChecked(false);
    ui->leftCam->setChecked(false);
    loadCamera(0);
}
void Control3::onCamera1(){
    ui->frontCam->setChecked(false);
    ui->rightCam->setChecked(true);
    ui->rearCam->setChecked(false);
    ui->leftCam->setChecked(false);
    loadCamera(1);
}
void Control3::onCamera2(){
    ui->frontCam->setChecked(false);
    ui->rightCam->setChecked(false);
    ui->rearCam->setChecked(true);
    ui->leftCam->setChecked(false);
    loadCamera(2);
}
void Control3::onCamera3(){
    ui->frontCam->setChecked(false);
    ui->rightCam->setChecked(false);
    ui->rearCam->setChecked(false);
    ui->leftCam->setChecked(true);
    loadCamera(3);
}
void Control3::onShowFp(bool show)
{
    gpMainWin->sendMessage(MESSAGE_VIEW_SHOW_FEATUREPOINTS, (show?1:0));

}

void Control3::onShowGrideLines(bool show)
{
    gpMainWin->sendMessage(MESSAGE_VIEW_SHOW_GRIDELINES, (show?1:0));

}
///
/// \brief start to prepare things when this step is started
///
void Control3::start()
{
    gpMainWin->changeView(mPanelTypeId);
    onCamera0();//after this iamge is available, zoom to current ZoomFactor
    gpMainWin->sendMessage(MESSAGE_VIEW_SCALE_1000IMAGE,
                            (long)(gpMainWin->getCurrentZoomFactor()*1000.0));

    gpMainWin->sendMessage(MESSAGE_VIEW_SHOW_FEATUREPOINTS,
                           (ui->checkShowFp->checkState() == Qt::Checked?1:0));
    gpMainWin->sendMessage(MESSAGE_VIEW_SHOW_GRIDELINES,
                           (ui->checkShowGrideLines->checkState() == Qt::Checked?1:0));


}
///
/// \brief stop to do things before this step is fnished
///
void Control3::stop()
{

}

#include "controlpanel.h"
#include "ui_control2.h"
#include "mainwindow.h"
#include "common.h"

#define RADIAN_TO_DEGREE(r) (r*180.0/M_PI)
#define DEGREE_TO_RADIAN(d) (d*M_PI/180.0)


Control2::Control2(TYPE id,QWidget *parent) :
    ControlPanel(id, parent),
    ui(new Ui::Control2)
{
    ui->setupUi(this);
    createUi();
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
    connect(ui->spinFov, SIGNAL(valueChanged(double)), SLOT(onFovValueChanged(double)));
    connect(ui->spina, SIGNAL(valueChanged(double)), SLOT(onIntricAChanged(double)));
    connect(ui->spinb, SIGNAL(valueChanged(double)), SLOT(onIntricBChanged(double)));
    connect(ui->spinc, SIGNAL(valueChanged(double)), SLOT(onIntricCChanged(double)));
    connect(ui->spinK1, SIGNAL(valueChanged(double)), SLOT(onK1ValueChanged(double)));
    connect(ui->spinK2, SIGNAL(valueChanged(double)), SLOT(onK2ValueChanged(double)));
    connect(ui->spinCenterX, SIGNAL(valueChanged(double)), SLOT(onCenterXChanged(double)));
    connect(ui->spinCenterY, SIGNAL(valueChanged(double)), SLOT(onCenterYChanged(double)));
    connect(ui->spinPitch, SIGNAL(valueChanged(double)), SLOT(onPitchChanged(double)));
    connect(ui->spinYaw, SIGNAL(valueChanged(double)), SLOT(onYawChanged(double)));
    connect(ui->spinRoll, SIGNAL(valueChanged(double)), SLOT(onRollChanged(double)));
    connect(ui->checkShowFp, SIGNAL(toggled(bool)), SLOT(onShowFp(bool)));
    connect(ui->checkShowGrideLines, SIGNAL(toggled(bool)), SLOT(onShowGrideLines(bool)));
    connect(ui->btnApply, SIGNAL(clicked()), SLOT(onApplyFec()));

}

void Control2::updateUi()
{
    AreaSettings* pAs = & gpTexProcess->mAreaSettings[mCamId];
    //normalize
    ui->spinFov->setValue(RADIAN_TO_DEGREE(pAs->fec.fov));
    ui->spina->setValue(pAs->fec.a);
    ui->spinb->setValue(pAs->fec.b);
    ui->spinc->setValue(pAs->fec.c);
    ui->spinK1->setValue(pAs->fec.k1);
    ui->spinK2->setValue(pAs->fec.k2);
    ui->spinCenterX->setValue(pAs->fec.ptCenter.x);
    ui->spinCenterY->setValue(pAs->fec.ptCenter.y);
    ui->spinPitch->setValue(RADIAN_TO_DEGREE(pAs->fec.pitch));
    ui->spinYaw->setValue(RADIAN_TO_DEGREE(pAs->fec.yaw));
    ui->spinRoll->setValue(RADIAN_TO_DEGREE(pAs->fec.roll));

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
    mCamId = cam;

    if (gpInputImage){
        nfImage* pImage = LoadImagebyArea(cam);
        gpMainWin->setImage(pImage);
        nfImage::dettach(&pImage);//ImageWin will free the buffer
        gpMainWin->sendMessage(MESSAGE_VIEW_SET_CAMERAID, (long) cam);


    }
    updateUi();
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
void Control2::onApplyFec()
{
    gpMainWin->sendMessage(MESSAGE_VIEW_UPDATE_FEC, (long) mCamId);
}

void Control2::onFovValueChanged(double value)
{
    AreaSettings* pAs = & gpTexProcess->mAreaSettings[mCamId];
    //degree to gradian
    pAs->fec.fov = DEGREE_TO_RADIAN(value);
}
void Control2::onIntricAChanged(double value){
    gpTexProcess->mAreaSettings[mCamId].fec.a = value;
}
void Control2::onIntricBChanged(double value){
    gpTexProcess->mAreaSettings[mCamId].fec.b = value;
}
void Control2::onIntricCChanged(double value){
    gpTexProcess->mAreaSettings[mCamId].fec.c = value;
}
void Control2::onK1ValueChanged(double value){
    gpTexProcess->mAreaSettings[mCamId].fec.k1 = value;
}
void Control2::onK2ValueChanged(double value){
    gpTexProcess->mAreaSettings[mCamId].fec.k2 = value;
}
void Control2::onCenterXChanged(double value){
    gpTexProcess->mAreaSettings[mCamId].fec.ptCenter.x = value;
}
void Control2::onCenterYChanged(double value){
    gpTexProcess->mAreaSettings[mCamId].fec.ptCenter.y = value;
}
void Control2::onPitchChanged(double value){
    gpTexProcess->mAreaSettings[mCamId].fec.pitch = DEGREE_TO_RADIAN(value);
}
void Control2::onYawChanged(double value){
    gpTexProcess->mAreaSettings[mCamId].fec.yaw = DEGREE_TO_RADIAN(value);
}
void Control2::onRollChanged(double value){
    gpTexProcess->mAreaSettings[mCamId].fec.roll = DEGREE_TO_RADIAN(value);
}
void Control2::onShowFp(bool show)
{
    gpMainWin->sendMessage(MESSAGE_VIEW_SHOW_FEATUREPOINTS, (show?1:0));

}

void Control2::onShowGrideLines(bool show)
{
    gpMainWin->sendMessage(MESSAGE_VIEW_SHOW_GRIDELINES, (show?1:0));

}

///
/// \brief start to prepare things when this step is started
///
void Control2::start()
{
    gpMainWin->changeView(mPanelTypeId);
    onCamera0();
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
void Control2::stop()
{

}

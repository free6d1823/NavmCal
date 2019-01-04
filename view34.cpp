#include "fpview.h"
#include "common.h"
#include "./imglab/ImgProcess.h"
#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#include <QMatrix3x3>



/* ==========================================================
 * Homograph Image Viewer
 * ==========================================================
 */
HomoView::HomoView(QWidget *parent) : ImageWin(parent),
    mShowFp(false),mShowGrideLines(false),
    mpSourceImage(nullptr)
{
    mCamId = 0;
    mViewType = 3;
}
HomoView::~HomoView()
{
    if(mpSourceImage)
        nfImage::destroy(&mpSourceImage);
}
void HomoView::setImage(nfImage* pSource)
{
    if(mpSourceImage)
        nfImage::destroy(&mpSourceImage);
    mpSourceImage = nfImage::clone(pSource);
    udateImage();

    setVisible(true);
}

static void applyAll(int camId, nfPByte pSrc, unsigned int width, unsigned int inStride,  unsigned int height,
                       nfPByte pTar, unsigned int outWidth, unsigned int OutHeight, unsigned int outStride)
{

    AreaSettings* pAs = & gpTexProcess->mAreaSettings[camId];

    float x,y,u,v;
    unsigned int nX, nY;

    for (int k=0; k< pAs->nFpAreaCounts; k++) {
        QRect region; //target region
        region.setLeft(static_cast<int>(pAs->region[k].l * outWidth));
        region.setRight(static_cast<int>(pAs->region[k].r * outWidth));
        region.setTop(static_cast<int>(pAs->region[k].t * OutHeight));
        region.setBottom(static_cast<int>(pAs->region[k].b * OutHeight));
        for(unsigned int i=static_cast<unsigned int> (region.top()) ;i< static_cast<unsigned int> (region.bottom()); i++) {
            float t = static_cast<float>(i)/static_cast<float>(OutHeight);
            for(unsigned int j= static_cast<unsigned int> (region.left()); j<static_cast<unsigned int> (region.right()); j++) {
                float s = static_cast<float>(j)/static_cast<float>(outWidth); //normalized coordingnates

                if (nfDoHomoTransform(s,t,u,v, pAs->homo[k].h)) {
                    nfDoFec(u,v,x,y, &pAs->fec);
                    if ( x>=0 && x<1 && y>=0 && y< 1){
                        nX = static_cast<unsigned int>(x * static_cast<float>(width+0.5));
                        nY = static_cast<unsigned int>(y * static_cast<float>( height+0.5));

                        pTar[i*outStride + j*4  ] = pSrc[nY*inStride+nX*4  ];//B
                        pTar[i*outStride + j*4+1] = pSrc[nY*inStride+nX*4+1];//G
                        pTar[i*outStride + j*4+2] = pSrc[nY*inStride+nX*4+2];//R
                        pTar[i*outStride + j*4+3] = pSrc[nY*inStride+nX*4+3];//A
                        continue;
                    }
                }
                //no match point, set black
                pTar[i*outStride + j*4  ] = 0;
                pTar[i*outStride + j*4+1] = 0;
                pTar[i*outStride + j*4+2] = 0;
                pTar[i*outStride + j*4+3] = 0xff;
            }
        }
    }

}
void HomoView::udateImage()
{

    nfPByte pOutBuffer = static_cast<nfPByte>(malloc(HOMO_IMAGE_WIDTH*HOMO_IMAGE_HEIGHT*4));
    memset(pOutBuffer, 0, HOMO_IMAGE_WIDTH*HOMO_IMAGE_HEIGHT*4);
    if (gpTexProcess->getDataState(mCamId) < DATA_STATE_FPF){
        gpTexProcess->normalizeFpf(mCamId);
    }
    if (gpTexProcess->getDataState(mCamId) < DATA_STATE_FPS){
        gpTexProcess->calculateFps(mCamId);
    }

    gpTexProcess->calculateHomo(mCamId);
    applyAll(mCamId, mpSourceImage->buffer,  mpSourceImage->width, mpSourceImage->stride,
            mpSourceImage->height, pOutBuffer, HOMO_IMAGE_WIDTH, HOMO_IMAGE_HEIGHT, HOMO_IMAGE_WIDTH*4);

    mImage = QImage(pOutBuffer,HOMO_IMAGE_WIDTH, HOMO_IMAGE_HEIGHT, QImage::Format_RGBA8888);
    mImageLabel->setPixmap(QPixmap::fromImage(mImage));

    mImageLabel->update();
}

void HomoView::processMessage(unsigned int command, long data)
{
    switch (command) {
    case MESSAGE_VIEW_SCALE_1000IMAGE:
        scaleImage((static_cast<double>(data))/1000.0);
        break;
    case MESSAGE_VIEW_SET_CAMERAID:
        mCamId = static_cast<int>(data);
        udateImage();
        if(mShowFp)
            loadFps();
        if(mShowGrideLines)
            loadRegions();
        mImageLabel->update();
        break;
    case MESSAGE_VIEW_SHOW_FEATUREPOINTS:
        if(data == 0) {
            mShowFp = false;
        }else {
            mShowFp = true;
            loadFps();
        }
        mImageLabel->update();
        break;
    case MESSAGE_VIEW_SHOW_GRIDELINES:
        if(data == 0) {
            mShowGrideLines = false;
        }else {
            mShowGrideLines = true;
            loadRegions();
        }
        mImageLabel->update();
        break;

    default:
        break;
    }
}
void HomoView::loadFps()
{
    if(gpTexProcess && mCamId >=0 && mCamId <MAX_CAMERAS){
        AreaSettings* pAs = & gpTexProcess->mAreaSettings[mCamId];
        mFpsList.clear();
        for (int i=0; i<pAs->nFpCounts; i++){
            mFpsList.push_back(pAs->fpt[i]);
        }
    }
}
void HomoView::loadRegions()
{
    if(gpTexProcess && mCamId >=0 && mCamId <MAX_CAMERAS){
        AreaSettings* pAs = & gpTexProcess->mAreaSettings[mCamId];
        mRegionList.clear();
        for (int i=0; i<pAs->nFpAreaCounts; i++){
            mRegionList.push_back(pAs->region[i]);
        }
    }
}

void HomoView::onPostDraw(QPainter* painter)
{
    QRect rcTarget = mImageLabel->rect();
    if(mShowGrideLines) {
        QPen pen1(REGION_COLOR);
        pen1.setWidth(GRIDELINE_WIDTH);
        painter->setPen(pen1);
        QRectF r;
        for (unsigned int i=0; i<mRegionList.size(); i++){
            r.setLeft(static_cast<double>(mRegionList[i].l)* rcTarget.width());
            r.setTop(static_cast<double>(mRegionList[i].t)* rcTarget.width());
            r.setRight(static_cast<double>(mRegionList[i].r)* rcTarget.width());
            r.setBottom(static_cast<double>(mRegionList[i].b)* rcTarget.width());

            painter->drawRect(r);
        }
    }
    if(mShowFp) {
        vector <QPointF> fps;
        QPointF pt;
        for (unsigned int i=0; i<mFpsList.size(); i++){
            pt.setX(static_cast<double>(mFpsList[i].x)* rcTarget.width());
            pt.setY(static_cast<double>(mFpsList[i].y)* rcTarget.height());
            fps.push_back(pt);
        }
        QPen pen1;
        pen1.setWidth(FP_WIDTH);
        pen1.setColor(FP_COLOR);
        painter->setPen(pen1);
        for (unsigned int i=0; i<fps.size(); i++) {
            painter->drawPoint(fps[i]);
        }


    }
}
/* ==========================================================
 * All stitched Image Viewer
 * ==========================================================
 */
AllView::AllView(QWidget *parent) : ImageWin(parent)
{
mShowCam[0] = mShowCam[1] = mShowCam[2] = mShowCam[3] = true;
    mViewType = 4;
    mShowCar = false;
    mSteerWheelAngle = 0;
}
AllView::~AllView()
{

}
void AllView::processMessage(unsigned int command, long data)
{
    switch (command) {
    case MESSAGE_VIEW_DO_STITCHING:
        udateImage();
        break;
    case MESSAGE_VIEW_SHOW_CAMERA0:
        mShowCam[0] = (data != 0);
        udateImage();
        break;
    case MESSAGE_VIEW_SHOW_CAMERA1:
        mShowCam[1] = (data != 0);
        udateImage();
        break;
    case MESSAGE_VIEW_SHOW_CAMERA2:
        mShowCam[2] = (data != 0);
        udateImage();
        break;
    case MESSAGE_VIEW_SHOW_CAMERA3:
        mShowCam[3] = (data != 0);
        udateImage();
        break;
    case MESSAGE_VIEW_SCALE_1000IMAGE:
        scaleImage(static_cast<double>(data)/1000.0);
        break;
    case MESSAGE_VIEW_SHOW_FEATUREPOINTS:
        if(data == 0) {
            mShowCar = false;
        }else {
            mShowCar = true;
        }
        mImageLabel->update();
        break;
    case MESSAGE_VIEW_STEER_CHANGED100:
        mSteerWheelAngle = (static_cast<double>(data)*M_PI/18000.0);
        mImageLabel->update();
        break;
    }
}
///
/// \brief RefImagebyArea get a reference image of the camera
/// \param nCamId
/// \return
///
extern nfImage* gpInputImage;
static nfImage* RefImagebyArea(int nCamId)
{
    if(!gpInputImage)
        return nullptr;
    unsigned char* pIn;
    unsigned int nOutWidth = gpInputImage->width/2;
    unsigned int nOutHeight = gpInputImage->height/2;

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
    nfImage* pOut = nfImage::ref(pIn, nOutWidth, nOutHeight, gpInputImage->stride);
    return pOut;
}
void AllView::udateImage()
{
//    nfPByte pOutBuffer = (nfPByte)malloc(HOMO_IMAGE_WIDTH*HOMO_IMAGE_HEIGHT*4);
    nfImage* pOut = nfImage::create(HOMO_IMAGE_WIDTH, HOMO_IMAGE_HEIGHT, 4);
    memset(pOut->buffer, 0, HOMO_IMAGE_WIDTH*HOMO_IMAGE_HEIGHT*4);

    for (int m = 0; m < MAX_CAMERAS; m++) {
        if (mShowCam[m]) {
            nfImage* pSource = RefImagebyArea(m);
            if (gpTexProcess->getDataState(m) < DATA_STATE_FPF){
                gpTexProcess->normalizeFpf(m);
            }
            if (gpTexProcess->getDataState(m) < DATA_STATE_FPS){
                gpTexProcess->calculateFps(m);
            }
            if (gpTexProcess->getDataState(m) < DATA_STATE_HOMO){
                gpTexProcess->calculateHomo(m);
            }

            applyAll(m, pSource->buffer, pSource->width, pSource->stride,
                    pSource->height, pOut->buffer, HOMO_IMAGE_WIDTH, HOMO_IMAGE_HEIGHT, HOMO_IMAGE_WIDTH*4);


            nfImage::destroy(&pSource);
        }
    }

    gpMainWin->setImage(pOut);
    nfImage::dettach(&pOut);//releas buffer to receiver
    /*
    mImage = QImage(pOutBuffer,HOMO_IMAGE_WIDTH, HOMO_IMAGE_HEIGHT, QImage::Format_RGBA8888);
    mImageLabel->setPixmap(QPixmap::fromImage(mImage));
    mImageLabel->update();*/
}
QPointF doTransform(QPointF p, QPointF ct, double ang)
{
    double px = p.x()- ct.x();
    double py = p.y() - ct.y();
    double tx, ty;
    tx = px * cos(ang) - py* sin(ang) + ct.x();
    ty = px * sin(ang) + py*cos(ang) + ct.y();
    return QPointF(tx,ty);
}
QPolygonF AllView::findTrack(QRectF car, double angle)
{
    /* CAR parameters
     *  |-      O          O    -|
     *  |   a   |    b     |  c  |
     *  |       |--------- +     d
     *  |       |          |     |
     *  |_      O          O    _|
     */
#define a 0.1862
#define b 0.5967
#define c 0.2171
    double d = car.width();
    double R1 = abs(b/tan( mSteerWheelAngle) )* car.height();
    double xCarOrg = car.left() + d/2;
    double yCarOrg = car.top()+ b* car.height();
    QPointF center;

    if(mSteerWheelAngle > 0){
        center.setX(xCarOrg+ R1);
        angle = -angle;
    }else {
        center.setX(xCarOrg - R1);
    }
    center.setY(yCarOrg);

    QPolygonF newCar;
    QPointF p1 = doTransform(car.topLeft(), center,  angle);
    newCar.append(p1);
    QPointF p = doTransform(car.bottomLeft(), center,  angle);
    newCar.append(p);
    p = doTransform(car.bottomRight(), center,  angle);
    newCar.append(p);
    p = doTransform(car.topRight(), center,  angle);
    newCar.append(p);
    newCar.append(p1);

    return newCar;
}
//////
/// \brief AllView::findTrack2
/// \param car dimension in pixel
/// \param dist1 distance, in pixel, of the start line after bumper
/// \param dist2 distance, in pixel, of the end line after bumper
/// \return the polyline of the track (p0~p7)
///     p0--p1--p2--p3
/// +--dist1---dist2|
///     p7--p6--p5--p4
QPolygonF AllView::findTrack2(QRectF car, double dist1, double dist2)
{
#define a 0.1862
#define b 0.5967
#define c 0.2171
    double d = car.width();
    double R1 = abs(b/tan( mSteerWheelAngle) )* car.height();
    double xCarOrg = car.left() + d/2;
    double yCarOrg = car.top()+ b* car.height();
    QPointF center;
    QPolygonF track;
    if(mSteerWheelAngle > 0)
        R1 = -R1;
    center.setX(xCarOrg-R1);
    center.setX(xCarOrg- R1);
    center.setY(yCarOrg);


    double angle[4];
    //1. first line
    angle[0] = dist1/R1;
    angle[3] = dist2/R1;
    angle[1] = (angle[0]*2+angle[3])/3;
    angle[2] = (angle[0]+angle[3]*2)/3;
    QPointF p;
    for(int i=0;i<4;i++) {
        p = doTransform(car.bottomRight(), center,  angle[i]);
        track.append(p); //p0~p3
    }
    for(int i=3;i>=0;i--) {
        p = doTransform(car.bottomLeft(), center,  angle[i]);
        track.append(p); //p4~p7
    }
    return track;
}

void AllView::onPostDraw(QPainter* painter)
{
    QRect rcTarget = mImageLabel->rect();
    QRectF rcCar;
    rcCar.setRect(rcTarget.width()*0.4, rcTarget.height()*0.23, rcTarget.width()*.2, rcTarget.height()*.5);

    if (mSteerWheelAngle != 0.0) {
        QPen pen1;
        pen1.setWidth(TRACK_WIDTH);
        pen1.setColor(TRACK_COLOR1);
        painter->setPen(pen1);
        QPolygonF rcShadow1 = findTrack2(rcCar, rcCar.width()*5/170, rcCar.width()*50/170);//50cm
        painter->drawPolyline(rcShadow1);

        pen1.setColor(TRACK_COLOR2);
        painter->setPen(pen1);
        QPolygonF rcShadow2 = findTrack2(rcCar, rcCar.width()*55/170, rcCar.width()*100/170);//100cm
        painter->drawPolyline(rcShadow2);

        pen1.setColor(TRACK_COLOR3);
        painter->setPen(pen1);
        QPolygonF rcShadow3 = findTrack2(rcCar, rcCar.width()*105/170, rcCar.width()*300/170);//300cm
        painter->drawPolyline(rcShadow3);

/*
        for (float i = -60; i <60; i+=4)
        {
            QPolygonF rcShadow = findTrack(rcCar, -i*M_PI/180.0);
            painter->drawPolyline(rcShadow);
        }
*/
    }

    if( mShowCar) {
        QImage car = QImage(":/images/NavmCal.png");
        painter->drawImage(rcCar, car, QRectF(QPointF(0,0), car.size()));

    }

}

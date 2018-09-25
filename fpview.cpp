#include "fpview.h"
#include "common.h"
#include "./imglab/ImgProcess.h"
#include "mainwindow.h"
#include <QApplication>
#include <QMessageBox>
#define FEC_IMAGE_WIDTH 1000
#define FEC_IMAGE_HEIGHT    1000
#define HOMO_IMAGE_WIDTH 1000
#define HOMO_IMAGE_HEIGHT 1000

/* ==========================================================
 * Input source Image Viewer
 * ==========================================================
 */

SourceView::SourceView(QWidget *parent) : ImageWin(parent)
{

    mViewType = 0;
}
SourceView::~SourceView()
{

}

/* ==========================================================
 * Single Camera & Feature Points Image Viewer
 * ==========================================================
 */

SingleView::SingleView(QWidget *parent) : ImageWin(parent)
{
    mShowFp = false;
    mFpsList.clear();
    mViewType = 1;
    mCamId = 0;
}
SingleView::~SingleView()
{

}
void SingleView::processMessage(unsigned int command, long data)
{
    switch (command) {
    case MESSAGE_VIEW_SET_CAMERAID:
        mCamId = (int ) data;
        if(mShowFp){
             loadFps();
        }
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
    case MESSAGE_VIEW_DO_AUTODETECTION:
        doAutoDetection();
        break;
    default:
        break;
    }
}
void SingleView::doAutoDetection()
{

}

void SingleView::loadFps()
{
    if(gpTexProcess && mCamId >=0 && mCamId <MAX_CAMERAS){
        AreaSettings* pAs = & gpTexProcess->mAreaSettings[mCamId];
        //normalize
        if (pAs->fpf[3].x > 10 || pAs->fpf[3].y > 10)
        { //normalize to [0,1]
            for (int i=0; i<FP_COUNTS; i++){
                pAs->fpf[i].x /= (float)mImage.width();
                pAs->fpf[i].y /= (float)mImage.height();
            }
        }
        //
        mFpsList.clear();
        for (int i=0; i<FP_COUNTS; i++){
            mFpsList.push_back(pAs->fpf[i]);
        }
    }
}
void SingleView::onPostDraw(QPainter* painter)
{
    QRect rcTarget = mImageLabel->rect();
    if(mShowFp) {
        vector <QPointF> fps;
        QPointF pt;
        for (unsigned int i=0; i<mFpsList.size(); i++){
            pt.setX(mFpsList[i].x* rcTarget.width());
            pt.setY(mFpsList[i].y* rcTarget.height());
            fps.push_back(pt);
        }
        QPen pen1;
        pen1.setWidth(3);
        pen1.setColor(Qt::red);
        painter->setPen(pen1);
        for (unsigned int i=0; i<fps.size(); i++) {
            painter->drawPoint(fps[i]);
        }


    }
}


/* ==========================================================
 * FEC Image Viewer
 * ==========================================================
 */
FecView::FecView(QWidget *parent) : ImageWin(parent),mShowFp(false),
    mShowGrideLines(false), mpSourceImage(NULL)
{
    mCamId = 0;
    mViewType = 2;
}
FecView::~FecView()
{
    if(mpSourceImage)
        nfImage::destroy(&mpSourceImage);
}
/* get the buffer of pSource as our fisheye image */
void FecView::setImage(nfImage* pSource)
{
    if(mpSourceImage)
        nfImage::destroy(&mpSourceImage);
    mpSourceImage = nfImage::clone(pSource);
    udateImage();

    setVisible(true);
}

void FecView::udateImage()
{

    nfPByte pOutBuffer = (nfPByte)malloc(FEC_IMAGE_WIDTH*FEC_IMAGE_HEIGHT*4);
    applyFec(mpSourceImage->buffer, mpSourceImage->width, mpSourceImage->stride,
            mpSourceImage->height, pOutBuffer, FEC_IMAGE_WIDTH, FEC_IMAGE_HEIGHT, FEC_IMAGE_WIDTH*4);

    mImage = QImage(pOutBuffer,FEC_IMAGE_WIDTH, FEC_IMAGE_HEIGHT, QImage::Format_RGBA8888);
    mImageLabel->setPixmap(QPixmap::fromImage(mImage));

    mImageLabel->update();
}
void FecView::applyFec(nfPByte pSrc, int width, int inStride,  int height,
                       nfPByte pTar, int outWidth, int OutHeight, int outStride)
{
    FecParam* pFec = & gpTexProcess->mAreaSettings[mCamId].fec;
    float x,y,u,v;
    int nX, nY;
    for (int i=0; i< OutHeight; i++) {
        v = (float)i/(float)OutHeight;
        for (int j=0; j<outWidth; j++) {
            u = (float)j/(float)outWidth;
            nfDoFec(u,v,x,y, pFec);

            if ( x>=0 && x<1 && y>=0 && y< 1){
                nX = (int) (x * (float) width+0.5);
                nY = (int) (y * (float) height+0.5);

                pTar[i*outStride + j*4  ] = pSrc[nY*inStride+nX*4  ];//B
                pTar[i*outStride + j*4+1] = pSrc[nY*inStride+nX*4+1];//G
                pTar[i*outStride + j*4+2] = pSrc[nY*inStride+nX*4+2];//R
                pTar[i*outStride + j*4+3] = pSrc[nY*inStride+nX*4+3];//A
            }
            else {
                pTar[i*outStride + j*4  ] = 0;
                pTar[i*outStride + j*4+1] = 0;
                pTar[i*outStride + j*4+2] = 0;
                pTar[i*outStride + j*4+3] = 0xff;
            }
        }
    }
}
void FecView::processMessage(unsigned int command, long data)
{
    switch (command) {
    case MESSAGE_VIEW_SET_CAMERAID:
        mCamId = (int ) data;
        //do update FEC, intent ignore "break" here
    case MESSAGE_VIEW_UPDATE_FEC:
        udateImage();
        if(mShowFp)
            loadFps();
        if(mShowGrideLines)
            mCenter = gpTexProcess->mAreaSettings[mCamId].fec.ptCenter;
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
            mCenter = gpTexProcess->mAreaSettings[mCamId].fec.ptCenter;
        }
        mImageLabel->update();
        break;

    default:
        break;
    }
}
void FecView::loadFps()
{
    if(gpTexProcess && mCamId >=0 && mCamId <MAX_CAMERAS){
        AreaSettings* pAs = & gpTexProcess->mAreaSettings[mCamId];
        //normalize
        if (pAs->fpf[3].x > 10 || pAs->fpf[3].y > 10)
        {
            QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                     tr("Please run Step 1 and show feature points first!"));
            return;
        }
        //update fps
        for (int i=0; i<FP_COUNTS; i++){
            nfInvFec(pAs->fpf[i].x, pAs->fpf[i].y, pAs->fps[i].x, pAs->fps[i].y, &pAs->fec);
        }

        //
        mFpsList.clear();
        for (int i=0; i<FP_COUNTS; i++){
            mFpsList.push_back(pAs->fps[i]);
        }
    }
}
void FecView::onPostDraw(QPainter* painter)
{
    QRect rcTarget = mImageLabel->rect();
    if(mShowGrideLines) {
        QPen pen1;
        pen1.setWidth(1);
        pen1.setColor(Qt::blue);
        painter->setPen(pen1);

        QPointF pt1, pt2;
        pt1.setX(0); pt1.setY(mCenter.y * rcTarget.height());
        pt2.setX(rcTarget.width()); pt2.setY(pt1.y());
        painter->drawLine(pt1,pt2);
        pt1.setY(0); pt1.setX(mCenter.x * rcTarget.width());
        pt2.setY(rcTarget.height()); pt2.setX(pt1.x());
        painter->drawLine(pt1,pt2);


    }
    if(mShowFp) {
        vector <QPointF> fps;
        QPointF pt;
        for (unsigned int i=0; i<mFpsList.size(); i++){
            pt.setX(mFpsList[i].x* rcTarget.width());
            pt.setY(mFpsList[i].y* rcTarget.height());
            fps.push_back(pt);
        }
        QPen pen1;
        pen1.setWidth(3);
        pen1.setColor(Qt::red);
        painter->setPen(pen1);
        for (unsigned int i=0; i<fps.size(); i++) {
            painter->drawPoint(fps[i]);
        }


    }
}
/* ==========================================================
 * Homograph Image Viewer
 * ==========================================================
 */
HomoView::HomoView(QWidget *parent) : ImageWin(parent),
    mShowFp(false),mShowGrideLines(false),
    mpSourceImage(NULL)
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

static void applyAll(int camId, nfPByte pSrc, int width, int inStride,  int height,
                       nfPByte pTar, int outWidth, int OutHeight, int outStride)
{
    AreaSettings* pAs = & gpTexProcess->mAreaSettings[camId];
    if(pAs->nFpAreaCounts == 18)
        nfCalculateHomoMatrix18(pAs->fps, pAs->fpt, pAs->homo);
    else if (pAs->nFpAreaCounts == 12)
        nfCalculateHomoMatrix12(pAs->fps, pAs->fpt, pAs->homo);
    else
        nfCalculateHomoMatrix4(pAs->fps, pAs->fpt, pAs->homo);


    float x,y,u,v;
    int nX, nY;

    for (int k=0; k< pAs->nFpAreaCounts; k++) {
        QRect region; //target region
        region.setLeft((int)(pAs->region[k].l * outWidth));
        region.setRight((int)(pAs->region[k].r * outWidth));
        region.setTop((int)(pAs->region[k].t * OutHeight));
        region.setBottom((int)(pAs->region[k].b * OutHeight));
        for(int i=region.top();i<region.bottom(); i++) {
            float t = (float)i/(float)OutHeight;
            for(int j=region.left(); j<region.right(); j++) {
                float s = (float)j/(float)outWidth; //normalized coordingnates

                if (nfDoHomoTransform(s,t,u,v, pAs->homo[k].h)) {
                    nfDoFec(u,v,x,y, &pAs->fec);
                    if ( x>=0 && x<1 && y>=0 && y< 1){
                        nX = (int) (x * (float) width+0.5);
                        nY = (int) (y * (float) height+0.5);

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

    nfPByte pOutBuffer = (nfPByte)malloc(HOMO_IMAGE_WIDTH*HOMO_IMAGE_HEIGHT*4);
    memset(pOutBuffer, 0, HOMO_IMAGE_WIDTH*HOMO_IMAGE_HEIGHT*4);
    applyAll(mCamId, mpSourceImage->buffer, mpSourceImage->width, mpSourceImage->stride,
            mpSourceImage->height, pOutBuffer, HOMO_IMAGE_WIDTH, HOMO_IMAGE_HEIGHT, HOMO_IMAGE_WIDTH*4);

    mImage = QImage(pOutBuffer,HOMO_IMAGE_WIDTH, HOMO_IMAGE_HEIGHT, QImage::Format_RGBA8888);
    mImageLabel->setPixmap(QPixmap::fromImage(mImage));

    mImageLabel->update();
}

void HomoView::processMessage(unsigned int command, long data)
{
    switch (command) {
    case MESSAGE_VIEW_SET_CAMERAID:
        mCamId = (int ) data;
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
        QPen pen1(Qt::blue);
        pen1.setWidth(1);
        painter->setPen(pen1);
        QRectF r;
        for (unsigned int i=0; i<mRegionList.size(); i++){
            r.setLeft(mRegionList[i].l* rcTarget.width());
            r.setTop(mRegionList[i].t* rcTarget.width());
            r.setRight(mRegionList[i].r* rcTarget.width());
            r.setBottom(mRegionList[i].b* rcTarget.width());

            painter->drawRect(r);
        }
    }
    if(mShowFp) {
        vector <QPointF> fps;
        QPointF pt;
        for (unsigned int i=0; i<mFpsList.size(); i++){
            pt.setX(mFpsList[i].x* rcTarget.width());
            pt.setY(mFpsList[i].y* rcTarget.height());
            fps.push_back(pt);
        }
        QPen pen1;
        pen1.setWidth(3);
        pen1.setColor(Qt::red);
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

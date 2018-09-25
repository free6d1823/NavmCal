#include "fpview.h"
#include "common.h"
#include "./imglab/ImgProcess.h"
#include <QApplication>
#include <QMessageBox>
#define OUT_IMAGE_WIDTH 1000
#define OUT_IMAGE_HEIGHT    1000
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
        for (int i=0; i<mFpsList.size(); i++){
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

    nfPByte pOutBuffer = (nfPByte)malloc(OUT_IMAGE_WIDTH*OUT_IMAGE_HEIGHT*4);
    applyFec(mpSourceImage->buffer, mpSourceImage->width, mpSourceImage->stride,
            mpSourceImage->height, pOutBuffer, OUT_IMAGE_WIDTH, OUT_IMAGE_HEIGHT, OUT_IMAGE_WIDTH*4);

    mImage = QImage(pOutBuffer,OUT_IMAGE_WIDTH, OUT_IMAGE_HEIGHT, QImage::Format_RGBA8888);
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
HomoView::HomoView(QWidget *parent) : ImageWin(parent)
{
    mCamId = 0;
    mViewType = 3;
}
HomoView::~HomoView()
{

}
void HomoView::processMessage(unsigned int command, long data)
{
    switch (command) {
    case MESSAGE_VIEW_SET_CAMERAID:
        mCamId = (int ) data;
        break;
    default:
        break;
    }
}
/* ==========================================================
 * All stitched Image Viewer
 * ==========================================================
 */
AllView::AllView(QWidget *parent) : ImageWin(parent)
{

    mViewType = 4;
}
AllView::~AllView()
{

}

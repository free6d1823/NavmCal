#include "fpview.h"
#include "common.h"
#include "./imglab/ImgProcess.h"
#include "mainwindow.h"

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
 * FEC Image Viewer
 * ==========================================================
 */
FecView::FecView(QWidget *parent) : ImageWin(parent),mShowFp(false),
    mShowGrideLines(false), mpSourceImage(nullptr)
{
    mCamId = 0;
    mViewType = 2;
    mDrag = false;
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
    case MESSAGE_VIEW_SCALE_1000IMAGE:
        scaleImage(((double)data)/1000);
        break;
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
    case MESSAGE_VIEW_CLEAR_GRIDELINES:
        mCurGrideLine.setLength(0.0);
        mImageLabel->update();
        break;
    default:
        break;
    }
}
void FecView::loadFps()
{
    if(gpTexProcess && mCamId >=0 && mCamId <MAX_CAMERAS){
        if (gpTexProcess->getDataState(mCamId) < DATA_STATE_FPF){
            gpTexProcess->normalizeFpf(mCamId);
        }

        gpTexProcess->calculateFps(mCamId);
        mFpsList.clear();
        AreaSettings* pAs = & gpTexProcess->mAreaSettings[mCamId];
        for (int i=0; i<pAs->nFpCounts; i++){
            mFpsList.push_back(pAs->fps[i]);
        }
    }
}
void FecView::onPostDraw(QPainter* painter)
{
    QRect rcTarget = mImageLabel->rect();
    if(mShowGrideLines) {
        QPen pen1;
        pen1.setWidth(GRIDELINE_WIDTH);
        pen1.setColor(GRIDELINE_COLOR);
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
        pen1.setWidth(FP_WIDTH);
        pen1.setColor(FP_COLOR);
        painter->setPen(pen1);
        for (unsigned int i=0; i<fps.size(); i++) {
            painter->drawPoint(fps[i]);
        }

        //draw gride lines
        if (mCurGrideLine.length() > 0.0) {
            QPen pen;
            pen.setWidth(TOOL_WIDTH);
            pen.setColor(TOOL_COLOR);
            painter->setPen(pen);
            QPointF pt1;
            QPointF pt2;
            pt1.setX(mCurGrideLine.p1().x() * rcTarget.width());
            pt1.setY(mCurGrideLine.p1().y()* rcTarget.height());
            pt2.setX(mCurGrideLine.p2().x() * rcTarget.width());
            pt2.setY(mCurGrideLine.p2().y()* rcTarget.height());
            painter->drawLine(pt1, pt2);
        }
    }
}
void FecView::mouseMoveEvent(QMouseEvent *event)
{

    if (mDrag) {
        QPointF pt = event->pos();
        QRect rcImage =  mImageLabel->rect();
        pt.setX((pt.x() - rcImage.left()+getScrollPosition().x())/rcImage.width());
        pt.setY((pt.y() - rcImage.top()+getScrollPosition().y())/rcImage.height());

        mCurGrideLine.setP2(pt);
        mImageLabel->update();
    }
}
void FecView::mousePressEvent(QMouseEvent *event)
{
    QPointF pt = event->pos();
    QRect rcImage =  mImageLabel->rect();
    pt.setX((pt.x() - rcImage.left()+getScrollPosition().x())/rcImage.width());
    pt.setY((pt.y() - rcImage.top()+getScrollPosition().y())/rcImage.height());
    mCurGrideLine.setP1(pt);
    mCurGrideLine.setP2(pt);
    mDrag = true;
    mImageLabel->update();
}
void FecView::mouseReleaseEvent(QMouseEvent *event)
{
    QPointF pt = event->pos();
    QRect rcImage =  mImageLabel->rect();
    pt.setX((pt.x() - rcImage.left()+getScrollPosition().x())/rcImage.width());
    pt.setY((pt.y() - rcImage.top()+getScrollPosition().y())/rcImage.height());
    mCurGrideLine.setP2(pt);
    mDrag = false;
    mImageLabel->update();
}

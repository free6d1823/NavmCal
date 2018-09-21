#include "fpview.h"
#include "common.h"
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
 * Feature Points Image Viewer
 * ==========================================================
 */

FpView::FpView(QWidget *parent) : ImageWin(parent)
{
    mShowFp = false;
    mFpsList.clear();
    mViewType = 1;
    mCamId = 0;
}
FpView::~FpView()
{

}
void FpView::processMessage(unsigned int command, long data)
{
    switch (command) {
    case MESSAGE_VIEW_SET_CAMERAID:
        mCamId = (int ) data;
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
void FpView::doAutoDetection()
{

}

void FpView::loadFps()
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
void FpView::onPostDraw(QPainter* painter)
{
    QRect rcTarget = mImageLabel->rect();
    if(mShowFp) {
        vector <QPointF> fps;
        QPointF pt;
        for (int i=0; i<FP_COUNTS; i++){
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
FecView::FecView(QWidget *parent) : ImageWin(parent)
{

    mViewType = 2;
}
FecView::~FecView()
{

}


/* ==========================================================
 * Homograph Image Viewer
 * ==========================================================
 */
HomoView::HomoView(QWidget *parent) : ImageWin(parent)
{

    mViewType = 3;
}
HomoView::~HomoView()
{

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

#include "fpview.h"
#include "common.h"
#include "./imglab/ImgProcess.h"


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
    mMode = Control1::EM_NONE;
    mDrag = false;
    mSelectedIndex = -1;

}
SingleView::~SingleView()
{

}
void SingleView::processMessage(unsigned int command, long data)
{
    switch (command) {
    case MESSAGE_VIEW_SCALE_1000IMAGE:
        scaleImage(((double)data)/1000);
        break;
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
    case MESSAGE_VIEW_SET_AUTO_MODE:
        doAutoDetection((int) data);
        break;
    case MESSAGE_VIEW_SET_ROI_MODE:
        setEditMode(Control1::EM_ROI);
        break;
    case MESSAGE_VIEW_SET_MANUAL_MODE:
        setEditMode(Control1::EM_MANUAL);
        break;
    case MESSAGE_VIEW_SET_LINK_MODE:
        setEditMode(Control1::EM_LINK);
        break;
    case MESSAGE_VIEW_DO_ACCEPT:
        break;
    case MESSAGE_VIEW_DO_RESET:
        mSelectedIndex = -1;
        mFpCandidates[mCamId].clear();
        break;
    default:
        break;
    }
}
void SingleView::doFpLink()
{
    mMode = Control1::EM_LINK;

    mSelectedIndex = -1;
    mImageLabel->update();
}
void SingleView::doAutoDetection(int threshold)
{
    mMode = Control1::EM_AUTO;
    mSelectedIndex = -1;

    //temperal demo purpose
    mFpCandidates[mCamId].clear();
    for (int i=0; i< mFpsList.size(); i++){
        mFpCandidates[mCamId].push_back(mFpsList[i]);
    }
    mImageLabel->update();
}

void SingleView::setEditMode(Control1::EditMode mode)
{
    mMode = mode;
    mSelectedIndex = -1;
    mImageLabel->update();
}

void SingleView::loadFps()
{
    if(gpTexProcess && mCamId >=0 && mCamId <MAX_CAMERAS){
        //normalize
        if (gpTexProcess->getDataState(mCamId) < DATA_STATE_FPF){
            gpTexProcess->normalizeFpf(mCamId);
        }
        //
        mFpsList.clear();
        AreaSettings* pAs = & gpTexProcess->mAreaSettings[mCamId];
        for (int i=0; i<pAs->nFpCounts; i++){
            mFpsList.push_back(pAs->fpf[i]);
        }
    }
}
int  SingleView::isHitFp(QPointF pt)
{
    int sel = -1;
    for (int i=0; i < mFpCandidates[mCamId].size(); i++){
        float x = pt.x() - mFpCandidates[mCamId][i].x;
        float y = pt.y() - mFpCandidates[mCamId][i].y;
        if (x < 0.005f && x > -0.005f && y < 0.005f && y > -0.005f){
            sel = i;
            break;
        }
    }
    return sel;
}
void SingleView::mouseMoveEvent(QMouseEvent *event)
{
    QPointF pt = event->pos();
    QRect rcImage =  mImageLabel->rect();
    pt.setX((pt.x() - rcImage.left()+getScrollPosition().x())/rcImage.width());
    pt.setY((pt.y() - rcImage.top()+getScrollPosition().y())/rcImage.height());
    switch (mMode) {
    case Control1::EM_ROI:
        if(mDrag) {
            if(mPtStart.x() < pt.x()){
                mRectRoi[mCamId].setLeft(mPtStart.x());
                mRectRoi[mCamId].setRight(pt.x());
            } else {
                mRectRoi[mCamId].setLeft(pt.x());
                mRectRoi[mCamId].setRight(mPtStart.x());
            }
            if(mPtStart.y() < pt.y()){
                mRectRoi[mCamId].setTop(mPtStart.y());
                mRectRoi[mCamId].setBottom(pt.y());
            } else {
                mRectRoi[mCamId].setBottom(mPtStart.y());
                mRectRoi[mCamId].setTop(pt.y());

            }
            mImageLabel->update();
        }
        break;
    case Control1::EM_MANUAL://move selected FP
        if(mSelectedIndex>=0){
            mFpCandidates[mCamId][mSelectedIndex].x = pt.x();
            mFpCandidates[mCamId][mSelectedIndex].y = pt.y();

        }
        mImageLabel->update();
        break;
    }

}
void SingleView::mousePressEvent(QMouseEvent *event)
{
    QPointF pt = event->pos();
    QRect rcImage =  mImageLabel->rect();
    mPtStart.setX((pt.x() - rcImage.left()+getScrollPosition().x())/rcImage.width());
    mPtStart.setY((pt.y() - rcImage.top()+getScrollPosition().y())/rcImage.height());
    switch (mMode) {
    case Control1::EM_ROI:
        if(!mDrag) {
            mRectRoi[mCamId].setTopLeft(mPtStart);
            mRectRoi[mCamId].setBottomRight(mPtStart);
            mDrag = true;
            mImageLabel->update();
        }
        break;
    case Control1::EM_MANUAL:
        mSelectedIndex = isHitFp(mPtStart);

        if (event->button() & Qt::RightButton) {
            if(mSelectedIndex >= 0){
                mFpCandidates[mCamId].erase(mFpCandidates[mCamId].begin()+mSelectedIndex);
            }
            mSelectedIndex = -1;
            mImageLabel->update();

        }else if (event->button() & Qt::LeftButton){
            mDrag = true;
            if (mSelectedIndex < 0) {
                nfFloat2D a;
                a.x = mPtStart.x();
                a.y = mPtStart.y();
                mSelectedIndex = mFpCandidates[mCamId].size();
                mFpCandidates[mCamId].push_back(a);
            }
            mImageLabel->update();
        }
        break;
    }
}
void SingleView::mouseDoubleClickEvent(QMouseEvent *event)
{

}
void SingleView::mouseReleaseEvent(QMouseEvent *event)
{
    QPointF pt = event->pos();
    QRect rcImage =  mImageLabel->rect();
    pt.setX((pt.x() - rcImage.left()+getScrollPosition().x())/rcImage.width());
    pt.setY((pt.y() - rcImage.top()+getScrollPosition().y())/rcImage.height());
    if (mDrag) {
        mDrag = false;
        switch (mMode) {
        case Control1::EM_ROI:
            if(mPtStart.x() < pt.x()){
                mRectRoi[mCamId].setLeft(mPtStart.x());
                mRectRoi[mCamId].setRight(pt.x());
            } else {
                mRectRoi[mCamId].setLeft(pt.x());
                mRectRoi[mCamId].setRight(mPtStart.x());
            }
            if(mPtStart.y() < pt.y()){
                mRectRoi[mCamId].setTop(mPtStart.y());
                mRectRoi[mCamId].setBottom(pt.y());
            } else {
                mRectRoi[mCamId].setBottom(mPtStart.y());
                mRectRoi[mCamId].setTop(pt.y());
            }
            break;
        case Control1::EM_MANUAL:

         break;
     }
     mImageLabel->update();

    }
}
void SingleView::onPostDraw(QPainter* painter)
{
    QRect rcTarget = mImageLabel->rect();
    if(mShowFp) {
        vector <QPointF> fps;
        QPointF pt;
/*        for (unsigned int i=0; i<mFpsList.size(); i++){
            pt.setX(mFpsList[i].x* rcTarget.width());
            pt.setY(mFpsList[i].y* rcTarget.height());
            fps.push_back(pt);
        }
*/
        for (unsigned int i=0; i<mFpCandidates[mCamId].size(); i++){
            pt.setX(mFpCandidates[mCamId][i].x* rcTarget.width());
            pt.setY(mFpCandidates[mCamId][i].y* rcTarget.height());
            fps.push_back(pt);
        }

        QPen pen2;
        pen2.setWidth(FP_WIDTH+2);
        pen2.setColor(SELECTED_COLOR);
        QPen pen1;
        pen1.setWidth(FP_WIDTH);
        pen1.setColor(FP_COLOR);
        for (unsigned int i=0; i<fps.size(); i++) {
            if (i== mSelectedIndex){
                painter->setPen(pen2);

            }else{
                painter->setPen(pen1);
            }
            painter->drawPoint(fps[i]);
        }
        pen1.setWidth(ROI_WIDTH);
        pen1.setColor(ROI_COLOT);
        painter->setPen(pen1);
        if (!mRectRoi[mCamId].isEmpty()){
            QRectF rc;
            rc.setLeft(mRectRoi[mCamId].left()* rcTarget.width());
            rc.setRight(mRectRoi[mCamId].right()* rcTarget.width());
            rc.setTop(mRectRoi[mCamId].top()* rcTarget.height());
            rc.setBottom(mRectRoi[mCamId].bottom()* rcTarget.height());
            painter->drawRect(rc);
        }
    }
}

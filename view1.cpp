#include "fpview.h"
#include "common.h"
#include "./imglab/ImgProcess.h"
#include "assert.h"
#include <QMessageBox>
#include <algorithm>

/* ==========================================================
 * Single Camera & Feature Points Image Viewer
 * ==========================================================
 */

SingleView::SingleView(QWidget *parent) : ImageWin(parent)
{
    mShowFp = false;
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
    case MESSAGE_VIEW_DO_AUTO_DETECT_FP:
        doAutoDetection((int) data);
        break;
    case MESSAGE_VIEW_SET_ROI_MODE:
        setEditMode(Control1::EM_ROI);
        break;
    case MESSAGE_VIEW_SET_MANUAL_MODE:
        setEditMode(Control1::EM_SET_FP);
        break;

    case MESSAGE_VIEW_SET_LINK_MODE:
        mNextLinkIndex = 0;
        setEditMode(Control1::EM_LINK);
        break;
    case MESSAGE_VIEW_DO_AUTO_LINK:
        mNextLinkIndex = 0;
        if(mFpCandidates[mCamId].size() == (unsigned int) gpTexProcess->mAreaSettings[mCamId].nFpCounts)
            doFpLink();
        else {
            QString text="Please add adequate feature points. This camera has added %1 feature points, but need %2 totally.";
            QMessageBox::warning(this, tr("Setup Feature Points"),
                    text.arg(mFpCandidates[mCamId].size()).arg(gpTexProcess->mAreaSettings[mCamId].nFpCounts),
                    QMessageBox::Cancel);
        }
        break;
    case MESSAGE_VIEW_DO_ACCEPT:
        //save mFpCandidates[mCamId] to ini
        saveFps();
        break;
    case MESSAGE_VIEW_DO_RESET:
        mSelectedIndex = -1;
        mFpCandidates[mCamId].clear();
        loadFps();
        break;
    default:
        break;
    }
}
#define MIN_DY1  0.1f   //center
#define MIN_DY2  0.05f  //edge

/* find top-left point index */
unsigned int findFirstFp(vector<nfFloat2D> list)
{
    //find top-two shortest points
    unsigned int i;
    unsigned int m=0,n=0;
    nfFloat2D a;
    a=list[0];
    float rmin = sqrt(a.x*a.x+ a.y*a.y);
    for(i=1; i<list.size(); i++) {
        float r = sqrt(list[i].x*list[i].x+ list[i].y*list[i].y);
        if (r < rmin ){
            m = i; rmin = r;
        }
    }
    a = list[0];
    rmin = sqrt(a.x*a.x+ a.y*a.y);
    for(i=1; i<list.size(); i++) {
        if (i==m)
            continue;
        float r = sqrt(list[i].x*list[i].x+ list[i].y*list[i].y);
        if (r < rmin ){
            n = i; rmin = r;
        }
    }
    //get the smaller x one as our candidate
    if (list[m].x < list[n].x)
        return m;
    return n;

}
/* find point left of pt, return -1 if not found or return the index */
int findRightFp(nfFloat2D pt, vector<nfFloat2D> list)
{
    unsigned int i;
    int m=-1, n= -1;
    //find two right nearest points
    float dxmin = 1;
    float bound = MIN_DY1;
    for(i=0; i<list.size(); i++) {
        float dx = list[i].x - pt.x;
        float dy = list[i].y - pt.y;
        if(dy<0) dy*= -1;
        if (list[i].x > 0.85 || list[i].x<0.15)
            bound = MIN_DY2;
        if (dx <= 0 || dy > bound){
            continue;
        }
        if (dx < dxmin ){
            m = (int)i;
            dxmin = dx;
        }
    }
    if (m == -1)
        return m;
    return m;
    dxmin = 1;
    for(i=0; i<list.size(); i++) {
        if (i == m)
            continue;
        float dx = list[i].x - pt.x;
        float dy = list[i].y - pt.y;
        if(dy<0) dy*= -1;

        if (list[i].x > 0.75 || list[i].x<0.25)
            bound = MIN_DY2;
        if (dx <= 0 || dy > bound){
            continue;
        }
        if (dx < dxmin ){
            n = (int)i;
            dxmin = dx;
        }
    }
    if (n == -1)
        return m;
    //select the smaller y diff
    float dy1 = list[m].y - pt.y; if (dy1<0)dy1 *= -1;
    float dy2 = list[n].y - pt.y; if (dy2<0)dy2 *= -1;
    return (dy1 < dy2)?m:n;
}

void SingleView::doFpLink()
{
    if (mMode != Control1::EM_LINK)
        return;
    mSelectedIndex = -1;
    if(mRectRoi[mCamId].isEmpty()){
        mRectRoi[mCamId] = QRectF(0,0,1,1);
    }
    //
    unsigned int i;
    vector <nfFloat2D> list;
    vector <nfFloat2D> dest;
    for (i=0; i< mFpCandidates[mCamId].size(); i++){
        list.push_back(mFpCandidates[mCamId][i]);
    }
    int k = findFirstFp(list);
    nfFloat2D f = list[k];
    dest.push_back(f);
    printf("row 0 item %d - (%f,%f)\n", dest.size(), f.x, f.y);
    list.erase(list.begin()+k);

    while(list.size() >0){
        k = findRightFp(f, list);
        if (k >= 0) {
            f = list[k];
            dest.push_back(f);
    printf("item %d - (%f,%f)\n", dest.size(), f.x, f.y);
            list.erase(list.begin()+k);
        }else {
            k = findFirstFp(list);
            f = list[k];
            dest.push_back(f);
    printf("nother row - item %d - (%f,%f)\n", dest.size(), f.x, f.y);
            list.erase(list.begin()+k);
        }
    }
    //
    mFpCandidates[mCamId].clear();
    for (i=0; i< mFpCandidates[mCamId].size(); i++){
        mFpCandidates[mCamId].push_back(dest[i]);
    }
    mImageLabel->update();
}

void SingleView::doAutoDetection(int threshold)
{
    if( mMode != Control1::EM_SET_FP)
        return;
    mSelectedIndex = -1;
    //temperal demo purpose

    loadFps();
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
        mFpCandidates[mCamId].clear();
        AreaSettings* pAs = & gpTexProcess->mAreaSettings[mCamId];
        for (int i=0; i<pAs->nFpCounts; i++){
            mFpCandidates[mCamId].push_back(pAs->fpf[i]);
        }
    }
}
void SingleView::saveFps()
{
    if(gpTexProcess && mCamId >=0 && mCamId <MAX_CAMERAS){
        AreaSettings* pAs = & gpTexProcess->mAreaSettings[mCamId];
        for (int i=0; i<mFpCandidates[mCamId].size(); i++){
            pAs->fpf[i] = mFpCandidates[mCamId][i];
        }
        gpTexProcess->normalizeFpf(mCamId);//to ensure FPF is normalized.
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
    case Control1::EM_SET_FP://move selected FP
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
    case Control1::EM_SET_FP:
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
    case Control1::EM_LINK:
        mSelectedIndex = isHitFp(mPtStart);
        if (mSelectedIndex >= 0){
            //remove mSelectedIndex-item and insert at mSelectedIndex
            nfFloat2D item = mFpCandidates[mCamId][mSelectedIndex];
            mFpCandidates[mCamId].insert(mFpCandidates[mCamId].begin()+mNextLinkIndex, item);
            if(mSelectedIndex < mNextLinkIndex){
                mFpCandidates[mCamId].erase(mFpCandidates[mCamId].begin()+mSelectedIndex);
                mNextLinkIndex --;
            }
            else {
                mFpCandidates[mCamId].erase(mFpCandidates[mCamId].begin()+mSelectedIndex+1);
            }
            mSelectedIndex = mNextLinkIndex;
            mImageLabel->update();
            mNextLinkIndex++;
            if(mNextLinkIndex >= mFpCandidates[mCamId].size())
                mNextLinkIndex = 0;
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
        case Control1::EM_SET_FP:

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
        //if (mMode == Control1::EM_LINK) {
        {
            QString nx = "%1";
            QPen pen3;
            pen3.setColor(FP_TEXT_COLOR);
            pen3.setWidth(1);
            painter->setPen(pen3);
            for (unsigned int i=0; i<mFpCandidates[mCamId].size(); i++){
                        pt.setX(mFpCandidates[mCamId][i].x* rcTarget.width()+5);
                        pt.setY(mFpCandidates[mCamId][i].y* rcTarget.height()-5);
                        painter->drawText(pt, nx.arg(i));
            }
        }
        QPen pen2;
        pen2.setWidth((int)((2) * mZoomFactor));
        pen2.setColor(SELECTED_COLOR);
        QPen pen1;
        pen1.setWidth((int)(FP_WIDTH*mZoomFactor));
        pen1.setColor(FP_COLOR);
        for (unsigned int i=0; i<fps.size(); i++) {
            if (i== mSelectedIndex){
                QRectF rc;
                double edge = FP_WIDTH* mZoomFactor;
                painter->setPen(pen2);
                rc.setTop(fps[i].y()-edge);
                rc.setBottom(fps[i].y()+edge);
                rc.setLeft(fps[i].x()-edge);
                rc.setRight(fps[i].x()+edge);
                painter->drawRect(rc);
            }else{
                painter->setPen(pen1);
                painter->drawPoint(fps[i]);
            }

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

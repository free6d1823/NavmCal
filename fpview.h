#ifndef FPVIEW_H
#define FPVIEW_H

#include "imagewin.h"
#include "common.h"
#include <vector>
#include "controlpanel.h"

using namespace std;


#define FEC_IMAGE_WIDTH 1000
#define FEC_IMAGE_HEIGHT    1000
#define HOMO_IMAGE_WIDTH 1000
#define HOMO_IMAGE_HEIGHT 1000
#define FP_COLOR    (Qt::red)
#define FP_WIDTH 7
#define FP_TEXT_COLOR (QColor(0x00,0x88,0xff))
#define GRIDELINE_WIDTH 1
#define GRIDELINE_COLOR Qt::blue
#define ROI_COLOT   (Qt::yellow)
#define ROI_WIDTH   3
#define LINK_COLOR  (Qt::darkblue)
#define LINK_WIDTH  2
#define SELECTED_COLOR QColor(255,128,0)
#define TOOL_COLOR   QColor(255,255,0)
#define TOOL_WIDTH   2

#define REGION_COLOR  (Qt::blue)
#define TRACK_WIDTH 2
#define TRACK_COLOR1  (Qt::red)
#define TRACK_COLOR2  (Qt::yellow)
#define TRACK_COLOR3  (Qt::green)

class SourceView : public ImageWin
{
    Q_OBJECT
public:
    explicit SourceView(QWidget *parent = nullptr);
    ~SourceView();
signals:

public slots:
};

class SingleView : public ImageWin
{
    Q_OBJECT
public:
    explicit SingleView(QWidget *parent = nullptr);
    ~SingleView();
    virtual void processMessage(unsigned int command, long data);
    virtual void onPostDraw(QPainter* painter);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
signals:

public slots:
private:
    int  isHitFp(QPointF pt);
    void loadFps();
    void saveFps();
    void doAutoDetection(int threshold);
    void doFpLink();
    void setEditMode(Control1::EditMode mode);
    bool mShowFp;
    int mCamId;

    Control1::EditMode mMode;
    QRectF mRectRoi[MAX_CAMERAS];
    QPointF mPtStart;
    vector <nfFloat2D> mFpCandidates[MAX_CAMERAS];
    int mSelectedIndex; /* selected FP index */
    vector <nfRectF> mRegionList[MAX_CAMERAS];

    int mNextLinkIndex; /* current focus link FP index */
    bool mDrag;
};
class FecView : public ImageWin
{
    Q_OBJECT
public:
    explicit FecView(QWidget *parent = nullptr);
    ~FecView();
    virtual void processMessage(unsigned int command, long data);
    virtual void onPostDraw(QPainter* painter);
    /*<! handle image be FecView itself */
    virtual void setImage(nfImage* pSource);
    void mouseMoveEvent(QMouseEvent *event);
    void mousePressEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
signals:

public slots:
private:
    void applyFec(nfPByte pSrc, int width, int inStride,  int height,
               nfPByte pTar, int outWidth, int OutHeight, int outStride);
    void loadFps();
    /*<! apply FEC parameters and update ImageView */
    void udateImage();
    int mCamId;
    bool mShowFp;
    bool mShowGrideLines;
    nfImage* mpSourceImage;
    vector <nfFloat2D> mFpsList;
    nfFloat2D mCenter;
    /*gride lines */
    QLineF mCurGrideLine;
    bool mDrag;
};
class HomoView : public ImageWin
{
    Q_OBJECT
public:
    explicit HomoView(QWidget *parent = nullptr);
    ~HomoView();
    virtual void processMessage(unsigned int command, long data);
    virtual void onPostDraw(QPainter* painter);
    /*<! handle image be FecView itself */
    virtual void setImage(nfImage* pSource);
signals:

public slots:
private:
    void loadFps();
    void loadRegions();
    /*<! apply FEC parameters and update ImageView */
    void udateImage();
    int mCamId;
    bool mShowFp;
    bool mShowGrideLines;
    nfImage* mpSourceImage;
    vector <nfFloat2D> mFpsList;
    vector <nfRectF> mRegionList;

};
class AllView : public ImageWin
{
    Q_OBJECT
public:
    explicit AllView(QWidget *parent = nullptr);
    ~AllView();
    virtual void processMessage(unsigned int command, long data);
    virtual void onPostDraw(QPainter* painter);
    QPolygonF findTrack(QRectF car, double angle);
    QPolygonF findTrack2(QRectF car, double dist1, double dist2);
signals:

private:
    bool mShowCar;
    void udateImage();
    bool mShowCam[4];
    double mSteerWheelAngle;
};
#endif // FPVIEW_H

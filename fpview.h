#ifndef FPVIEW_H
#define FPVIEW_H

#include "imagewin.h"
#include "common.h"
#include <vector>
using namespace std;

class SourceView : public ImageWin
{
    Q_OBJECT
public:
    explicit SourceView(QWidget *parent = 0);
    ~SourceView();
signals:

public slots:
};

class SingleView : public ImageWin
{
    Q_OBJECT
public:
    explicit SingleView(QWidget *parent = 0);
    ~SingleView();
    virtual void processMessage(unsigned int command, long data);
    virtual void onPostDraw(QPainter* painter);
signals:

public slots:
private:
    void loadFps();
    void doAutoDetection();
    bool mShowFp;
    int mCamId;
    vector <nfFloat2D> mFpsList;
};
class FecView : public ImageWin
{
    Q_OBJECT
public:
    explicit FecView(QWidget *parent = 0);
    ~FecView();
    virtual void processMessage(unsigned int command, long data);
    virtual void onPostDraw(QPainter* painter);
    /*<! handle image be FecView itself */
    virtual void setImage(nfImage* pSource);
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

};
class HomoView : public ImageWin
{
    Q_OBJECT
public:
    explicit HomoView(QWidget *parent = 0);
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
    explicit AllView(QWidget *parent = 0);
    ~AllView();
    virtual void processMessage(unsigned int command, long data);
    virtual void onPostDraw(QPainter* painter);
    QPolygonF findTrack(QRectF car, float angle);
signals:

private:
    bool mShowCar;
    void udateImage();
    bool mShowCam[4];
    float mSteerWheelAngle;
};
#endif // FPVIEW_H

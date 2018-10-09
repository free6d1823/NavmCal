#ifndef IMAGEVWIN_H
#define IMAGEVWIN_H

#include <QtWidgets/qtwidgetsglobal.h>
#include <QtWidgets/qscrollarea.h>

#include <QMouseEvent>
#include <QPaintEvent>
#include <QPainter>
#include <QLabel>

#define RULER_BREADTH 20

class QDRuler : public QWidget
{
Q_OBJECT
Q_ENUMS(RulerType)
Q_PROPERTY(qreal origin READ origin WRITE setOrigin)
Q_PROPERTY(qreal rulerUnit READ rulerUnit WRITE setRulerUnit)
Q_PROPERTY(qreal rulerZoom READ rulerZoom WRITE setRulerZoom)
public:
  enum RulerType { Horizontal, Vertical };
QDRuler(QDRuler::RulerType rulerType, QWidget* parent);

QSize minimumSizeHint() const
{
  return QSize(RULER_BREADTH,RULER_BREADTH);
}

QDRuler::RulerType rulerType() const
{
  return mRulerType;
}

qreal origin() const
{
  return mOrigin;
}

qreal rulerUnit() const
{
  return mRulerUnit;
}

qreal rulerZoom() const
{
  return mRulerZoom;
}

public slots:

void setOrigin(const qreal origin);
void setRulerUnit(const qreal rulerUnit);
void setRulerZoom(const qreal rulerZoom)
{
  if (mRulerZoom != rulerZoom)
  {
    mRulerZoom = rulerZoom;
    update();
  }
}

void setCursorPos(const QPoint cursorPos)
{
//  mCursorPos = this->mapFromGlobal(cursorPos);
//  mCursorPos += QPoint(RULER_BREADTH,RULER_BREADTH);
    mCursorPos = cursorPos;
  update();
}

void setMouseTrack(const bool track)
{
  if (mMouseTracking != track)
  {
    mMouseTracking = track;
    update();
  }
}

protected:
void mouseMoveEvent(QMouseEvent* event);
void paintEvent(QPaintEvent* /*event*/);
private:
void drawAScaleMeter(QPainter* painter, QRectF rulerRect, qreal scaleMeter, qreal startPositoin);

void drawFromOriginTo(QPainter* painter, QRectF rulerRect, qreal startMark, qreal endMark, int startTickNo, qreal step, qreal startPosition, qreal unScaleStep);
void drawMousePosTick(QPainter* painter);
private:
  RulerType mRulerType;
  qreal mOrigin;
  qreal mRulerUnit;
  qreal mRulerZoom;
  QPoint mCursorPos;
  bool mMouseTracking;
  bool mDrawText;
};


class nfImage;
class ImageWin;
typedef void (*ImageView_PostDraw)(ImageWin* owner, QPainter* painter);
class ImageView : public QLabel
{
    Q_OBJECT
public:
    explicit ImageView(QWidget *parent = 0);
    void setPostDrawCallback(ImageView_PostDraw pFn, ImageWin* owner){
        m_fpPostDraw = pFn;
        m_pOwner = owner;
    }
    //~ImageView();
protected:
    ImageView_PostDraw  m_fpPostDraw;
    ImageWin* m_pOwner;
    void paintEvent(QPaintEvent* event);
};

class ImageWin : public QScrollArea
{
    Q_OBJECT
//    Q_PROPERTY(bool widgetResizable READ widgetResizable WRITE setWidgetResizable)
//    Q_PROPERTY(Qt::Alignment alignment READ alignment WRITE setAlignment)

public:
static     ImageWin* createImageView(int id, QWidget *parent = 0);
static void PostDrwCallback(ImageWin* owner, QPainter* painter){
                owner->onPostDraw(painter);}
    explicit ImageWin(QWidget *parent = 0);
    ~ImageWin();
    double getZoomFactor();
    QPoint getScrollPosition();
    void scrollContentsBy(int dx, int dy);
    void mouseMoveEvent(QMouseEvent* event);

    virtual void setImage(nfImage* pSource);
    virtual void adjustSize( );
    virtual void scaleImage(double factor);
    virtual void processMessage(unsigned int /* command*/, long /*pData*/){}
    virtual void onPostDraw(QPainter* /*painter*/){}
 //   QImage* getImage(){ return &mImage;}
    void showRulers(bool bShow);
    bool isRulersShown();
protected:
    int mViewType;
    ImageView *mImageLabel;
    QImage  mImage;
    QDRuler* mHorzRuler;
    QDRuler* mVertRuler;
    QWidget* mRulerCorner;
    double mZoomFactor;

};

#endif // IMAGEVWIN_H

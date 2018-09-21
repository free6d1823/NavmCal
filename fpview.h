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

class FpView : public ImageWin
{
    Q_OBJECT
public:
    explicit FpView(QWidget *parent = 0);
    ~FpView();
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

signals:

public slots:
};
class HomoView : public ImageWin
{
    Q_OBJECT
public:
    explicit HomoView(QWidget *parent = 0);
    ~HomoView();
signals:

public slots:
};
class AllView : public ImageWin
{
    Q_OBJECT
public:
    explicit AllView(QWidget *parent = 0);
    ~AllView();
signals:

public slots:
};
#endif // FPVIEW_H

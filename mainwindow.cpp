#include "mainwindow.h"
#include "ui_mainwindow.h"
#include <QLabel>
#include <QScrollBar>
#include <QToolBar>
#include <QMessageBox>
#include <QDir>
#include <QFileDialog>
#include <QScreen>
#include <QImageReader>
#include <QImageWriter>
#include <QStandardPaths>
#include <QByteArray>
#include <QList>
#include <QClipboard>
#include <QMimeData>
#include <QScrollArea>
#include "imagewin.h"
#include <QDockWidget>
#include <QListWidget>
#include <QPlainTextEdit>
#include "control1.h"

#define CLIP(X) ( (X) > 255 ? 255 : (X) < 0 ? 0 : X)


// YUV -> RGB
#define C(Y) ( (Y) - 16  )
#define D(U) ( (U) - 128 )
#define E(V) ( (V) - 128 )

#define YUV2R(Y, U, V) CLIP(( 298 * C(Y)              + 409 * E(V) + 128) >> 8)
#define YUV2G(Y, U, V) CLIP(( 298 * C(Y) - 100 * D(U) - 208 * E(V) + 128) >> 8)
#define YUV2B(Y, U, V) CLIP(( 298 * C(Y) + 516 * D(U)              + 128) >> 8)
//////
/// \brief YuyvToRgb32 YUV420 to RGBA 32
/// \param pYuv
/// \param width
/// \param stride
/// \param height
/// \param pRgb     output RGB32 buffer
/// \param uFirst   true if pYuv is YUYV, false if YVYU
///
static void YuyvToRgb32(unsigned char* pYuv, int width, int stride, int height, unsigned char* pRgb, bool uFirst)
{
    //YVYU - format
    int nBps = width*4;
    unsigned char* pY1 = pYuv;

    unsigned char* pV;
    unsigned char* pU;

    if (uFirst) {
        pU = pY1+1; pV = pU+2;
    } else {
        pV = pY1+1; pU = pV+2;
    }


    unsigned char* pLine1 = pRgb;

    unsigned char y1,u,v;
    for (int i=0; i<height; i++)
    {
        for (int j=0; j<width; j+=2)
        {
            y1 = pY1[2*j];
            u = pU[2*j];
            v = pV[2*j];
            pLine1[j*4] = YUV2B(y1, u, v);//b
            pLine1[j*4+1] = YUV2G(y1, u, v);//g
            pLine1[j*4+2] = YUV2R(y1, u, v);//r
            pLine1[j*4+3] = 0xff;

            y1 = pY1[2*j+2];
            pLine1[j*4+4] = YUV2B(y1, u, v);//b
            pLine1[j*4+5] = YUV2G(y1, u, v);//g
            pLine1[j*4+6] = YUV2R(y1, u, v);//r
            pLine1[j*4+7] = 0xff;
        }
        pY1 += stride;
        pV += stride;
        pU += stride;
        pLine1 += nBps;

    }
}

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),ui(new Ui::MainWindow),
    mImageView(new ImageWin),
    mZoomFactor(1)
{
    ui->setupUi(this);


    createMenuAndToolbar();
    createUi();
    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);

}

MainWindow::~MainWindow()
{
    delete ui;
}
bool MainWindow::loadFile(const QString &fileName)
{
    QImageReader reader(fileName);
    reader.setAutoDetectImageFormat(true);
    const QImage newImage = reader.read();
    if (newImage.isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot load %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName), reader.errorString()));
        return false;
    }

    setImage(newImage);

    setWindowFilePath(fileName);

    const QString message = tr("Opened \"%1\", %2x%3, Depth: %4")
        .arg(QDir::toNativeSeparators(fileName)).arg(mImageView->getImage()->width()).arg(mImageView->getImage()->height()).arg(mImageView->getImage()->depth());
    statusBar()->showMessage(message);
    return true;
}
bool MainWindow::loadFileYuv(const QString & filename, bool isPlanMode)
{

    QFile fp(filename);
     if(!fp.open(QIODevice::ReadOnly))
             return false;
     //check filename and dimension

    char value[32];
    const char* pname = filename.toUtf8().data();
    const char* p1 = strchr(pname, '_');
    const char* p2 = strchr(pname, '.');
    int length = p2-p1-1;
    if (p1 == NULL || p2 == NULL || length <3 || length>= (int) sizeof(value)-1){
        fp.close();

        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("File name must be in the format of abc_widthxheight.yuv!"));
        return false;
    }

    memcpy(value, p1+1, length);
    value[length] = 0;
    p1 = strchr(value, 'x');
    if (!p1) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("File name must be in the form of abc_widthxheight.yuv!"));
        return false;
    }
    *(char*) p1 = 0;

    int width = atoi(value);
    int height = atoi(p1+1);

     unsigned char* m_pRgb32 = (unsigned char*) malloc(width*4*height);
     if (!m_pRgb32) return false;
     void* pYuv = malloc(width*2*height);
     if (!pYuv) {
         free(m_pRgb32);
         return false;
     }
     QImage* newImage = NULL;
     if ( fp.read((char* )pYuv, width*2*height) >0){
         YuyvToRgb32((unsigned char*)pYuv, width, width*2, height,
                                 m_pRgb32, true);
         free(pYuv);
         newImage = new QImage(m_pRgb32,
                     width, height, QImage::Format_RGBA8888);

     }
     fp.close();
     if(newImage) {
         setImage(*newImage);
         setWindowFilePath(filename);
         QString file1 = QDir::toNativeSeparators(filename);
         QString message = tr("Opened \"%1\", %2x%3, Depth: 32")
             .arg(file1).arg(width).arg(height);
         statusBar()->showMessage(message);

     }
     else {
              QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                       tr("Failed to read file!"));
          }
     return (newImage!=NULL);
}

void MainWindow::setImage(const QImage &newImage)
{
    mZoomFactor = 1.0;
    mImageView->setImage(newImage);
    mFitToWindowAct->setEnabled(true);
    updateActions();

    if (!mFitToWindowAct->isChecked())
        mImageView->adjustSize();
}
bool MainWindow::saveFile(const QString &fileName)
{
    QImageWriter writer(fileName);

    if (!writer.write(*mImageView->getImage())) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot write %1: %2")
                                 .arg(QDir::toNativeSeparators(fileName)), writer.errorString());
        return false;
    }
    const QString message = tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName));
    statusBar()->showMessage(message);
    return true;
}

static void initializeImageFileDialog(QFileDialog &dialog, QFileDialog::AcceptMode acceptMode)
{
    static bool firstDialog = true;

    if (firstDialog) {
        firstDialog = false;
        const QStringList picturesLocations = QStandardPaths::standardLocations(QStandardPaths::PicturesLocation);
        dialog.setDirectory(picturesLocations.isEmpty() ? QDir::currentPath() : picturesLocations.last());
    }
    QStringList filters;
    filters << "YUYV packet mode (*.yuv)"
            << "Image files (*.png *.xpm *.jpg)";
    dialog.setNameFilters(filters);;
   if (acceptMode == QFileDialog::AcceptSave)
        dialog.setDefaultSuffix("jpg");
}
void MainWindow::onFileOpen()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("Open Imahe"), QDir::currentPath(),
          tr("YUYV packet mode (*.yuv);;"
              "Image files (*.png *.xpm *.jpg)") );

    if (!filename.isNull()){
        if(filename.contains(".yuv"))
            loadFileYuv(filename);
        else if(filename.contains(".yuyv"))
            loadFileYuv(filename, false);
        else
            loadFile(filename);
    }

}

void MainWindow::onFileSaveAs()
{
    QFileDialog dialog(this, tr("Save File As"));
    initializeImageFileDialog(dialog, QFileDialog::AcceptSave);

    while (dialog.exec() == QDialog::Accepted && !saveFile(dialog.selectedFiles().first())) {}

}
void MainWindow::onFlowStep1()
{
    mStep1Act->setChecked(true);
    mStep2Act->setChecked(false);
    mStep3Act->setChecked(false);
    mStep4Act->setChecked(false);
    mDockView[0]->setVisible(true);
    mDockView[1]->setVisible(false);
    mDockView[2]->setVisible(false);
    mDockView[3]->setVisible(false);

}
void MainWindow::onFlowStep2()
{
    mStep1Act->setChecked(false);
    mStep2Act->setChecked(true);
    mStep3Act->setChecked(false);
    mStep4Act->setChecked(false);
    mDockView[0]->setVisible(false);
    mDockView[1]->setVisible(true);
    mDockView[2]->setVisible(false);
    mDockView[3]->setVisible(false);
}
void MainWindow::onFlowStep3()
{
    mStep1Act->setChecked(false);
    mStep2Act->setChecked(false);
    mStep3Act->setChecked(true);
    mStep4Act->setChecked(false);
    mDockView[0]->setVisible(false);
    mDockView[1]->setVisible(false);
    mDockView[2]->setVisible(true);
    mDockView[3]->setVisible(false);
}
void MainWindow::onFlowStep4()
{
    mStep1Act->setChecked(false);
    mStep2Act->setChecked(false);
    mStep3Act->setChecked(false);
    mStep4Act->setChecked(true);

    mDockView[0]->setVisible(false);
    mDockView[1]->setVisible(false);
    mDockView[2]->setVisible(false);
    mDockView[3]->setVisible(true);
}

void MainWindow::onViewZoomin()
{
    scaleImage(1.25);
}

void MainWindow::onViewZoomout()
{
    scaleImage(0.8);
}

void MainWindow::onViewNormalSize()
{
    mImageView->adjustSize();
    mZoomFactor = 1.0;
}

void MainWindow::onViewFitToWindow()
{
    bool fitToWindow = mFitToWindowAct->isChecked();
    mImageView->setWidgetResizable(fitToWindow);
    if (!fitToWindow)
        onViewNormalSize();
    updateActions();
}
void MainWindow::onViewShowRuler()
{
    mImageView->showRulers(!mImageView->isRulersShown());
}

void MainWindow::onHelpAbout()
{
    QMessageBox::about(this, tr("About Application"),
             tr("The <b>Application</b>  Copyrights 2018."));
}

void MainWindow::closeEvent(QCloseEvent *event)
{
    event->accept();
}

void MainWindow::createUi()
{
    setCentralWidget(mImageView);

    mDockView[0] = new QDockWidget(tr("Detect Feature Points"), this);
    mDockView[0]->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    Control1* step1Widget = new Control1(mDockView[0]);
    mDockView[0]->setWidget(step1Widget);
    addDockWidget(Qt::LeftDockWidgetArea, mDockView[0]);
    mDockView[0]->setVisible(true);

    mDockView[1] = new QDockWidget(tr("Fisheye Correction"), this);
    mDockView[1]->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    QLabel * step2Widget = new QLabel(mDockView[1]);
    step2Widget->setText("Step 2: adjust lens intrinsic parameters and fisheye correction arameters");
    mDockView[1]->setWidget(step2Widget);
    addDockWidget(Qt::LeftDockWidgetArea, mDockView[1]);
    mDockView[1]->setVisible(false);

    mDockView[2] = new QDockWidget(tr("Homograph"), this);
    mDockView[2]->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    QListWidget* step3Widget = new QListWidget(mDockView[2]);
    step3Widget->addItems(QStringList()
            << "Check homographic transformation");
    mDockView[2]->setWidget(step3Widget);
    addDockWidget(Qt::LeftDockWidgetArea, mDockView[2]);
    mDockView[2]->setVisible(false);

    mDockView[3] = new QDockWidget(tr("Stitching"), this);
    mDockView[3]->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    QListWidget* step4Widget = new QListWidget(mDockView[3]);
    step4Widget->addItems(QStringList()
            << "Final image");
    mDockView[3]->setWidget(step4Widget);
    addDockWidget(Qt::LeftDockWidgetArea, mDockView[3]);
    mDockView[3]->setVisible(false);

}

void MainWindow::createMenuAndToolbar()
{
    QToolBar *fileToolBar = ui->mainToolBar;
    QMenu* fileMenu = ui->menuBar->addMenu(tr("&File"));
    const QIcon openIcon = QIcon(":/images/open.png");
    QAction *openAct = fileMenu->addAction(openIcon, tr("&Open..."), this,
                    SLOT(onFileOpen()), QKeySequence::Open);
    openAct->setStatusTip(tr("Open an image file."));
    const QIcon saveIcon = QIcon(":/images/save.png");
    mSaveAsAct = fileMenu->addAction(saveIcon, tr("&Save..."), this,
                                     SLOT(onFileSaveAs()));
    mSaveAsAct->setStatusTip(tr("Save file."));
    mSaveAsAct->setEnabled(false);
    fileMenu->addSeparator();
    const QIcon closeIcon =  QIcon(":/images/exit.png");
    QAction* closeAct = fileMenu->addAction(closeIcon,tr("E&xit"), this,
            SLOT(close()), QKeySequence::Quit);
    closeAct->setStatusTip(tr("Exit this program."));
    fileToolBar->addAction(openAct);
    fileToolBar->addAction(mSaveAsAct);
    fileToolBar->addSeparator();
    /*******************************************/
    QMenu *flowMenu = ui->menuBar->addMenu(tr("&Flow"));
    const QIcon step1Icon =  QIcon(":/images/step1.png");
    mStep1Act = flowMenu->addAction(step1Icon, tr("Step &1"), this,
                         SLOT(onFlowStep1()),tr("Ctrl+1"));
    mStep1Act->setEnabled(false);
    mStep1Act->setCheckable(true);
    const QIcon step2Icon =  QIcon(":/images/step2.png");
    mStep2Act = flowMenu->addAction(step2Icon, tr("Step &2"), this,
                         SLOT(onFlowStep2()),tr("Ctrl+2"));
    mStep2Act->setEnabled(false);
    mStep2Act->setCheckable(true);
    const QIcon step3Icon =  QIcon(":/images/step3.png");
    mStep3Act = flowMenu->addAction(step3Icon, tr("Step &3"), this,
                         SLOT(onFlowStep3()),tr("Ctrl+3"));
    mStep3Act->setEnabled(false);
    mStep3Act->setCheckable(true);
    const QIcon step4Icon =  QIcon(":/images/step4.png");
    mStep4Act = flowMenu->addAction(step4Icon, tr("Step &4"), this,
                         SLOT(onFlowStep4()),tr("Ctrl+4"));
    mStep4Act->setEnabled(false);
    mStep4Act->setCheckable(true);
    fileToolBar->addAction(mStep1Act);
    fileToolBar->addAction(mStep2Act);
    fileToolBar->addAction(mStep3Act);
    fileToolBar->addAction(mStep4Act);
    fileToolBar->addSeparator();
    /*******************************************/
    QMenu* viewMenu = ui->menuBar->addMenu(tr("&View"));
    const QIcon zoominIcon =  QIcon(":/images/zoomin.png");
    mZoomInAct = viewMenu->addAction(zoominIcon,tr("Zoom &In"), this,
                                    SLOT(onViewZoomin()), QKeySequence::ZoomIn );
    mZoomInAct->setEnabled(false);
    mZoomInAct->setStatusTip(tr("Magfying image."));

    const QIcon zoomoutIcon = QIcon(":/images/zoomout.png");
    mZoomOutAct = viewMenu->addAction(zoomoutIcon,tr("Zoom &Out"), this,
                     SLOT(onViewZoomout()), QKeySequence::ZoomOut );
    mZoomOutAct->setStatusTip(tr("zoom out image."));
    mZoomOutAct->setEnabled(false);
    const QIcon normalSizeIcon = QIcon(":/images/normalsize.png");
    mNormalSizeAct = viewMenu->addAction(normalSizeIcon, tr("Normal &Size"), this,
                     SLOT(onViewNormalSize()), tr("Ctrl+S") );
    viewMenu->addSeparator();
    mNormalSizeAct->setStatusTip(tr("set normal size image."));
    mNormalSizeAct->setEnabled(false);
     const QIcon fitIcon = QIcon(":/images/fittowindow.png");
    mFitToWindowAct = viewMenu->addAction(fitIcon, tr("Fit to Window"), this,
                     SLOT(onViewFitToWindow()), tr("Ctrl+S") );
    mFitToWindowAct->setStatusTip(tr("set normal size image."));
    mFitToWindowAct->setEnabled(false);
    mFitToWindowAct->setCheckable(true);
    mFitToWindowAct->setShortcut(tr("Ctrl+F"));

    const QIcon rulerIcon = QIcon(":/images/ruler.png");
    QAction* rulerAct = viewMenu->addAction(rulerIcon, tr("Show &Ruler"), this,
                     SLOT(onViewShowRuler()), tr("Ctrl+R") );
    viewMenu->addSeparator();
    rulerAct->setStatusTip(tr("Toggle rulers."));
    rulerAct->setEnabled(true);

    fileToolBar->addAction(mZoomInAct);
    fileToolBar->addAction(mZoomOutAct);
    fileToolBar->addAction(mNormalSizeAct);
    fileToolBar->addSeparator() ;
    fileToolBar->addAction(mFitToWindowAct);
    fileToolBar->addAction(rulerAct);
    fileToolBar->addSeparator() ;

    QMenu* helpMenu = ui->menuBar->addMenu(tr("&Help"));
    const QIcon aboutIcon = QIcon(":/images/about.png");
    QAction* aboutAct = helpMenu->addAction(aboutIcon, tr("A&bout"), this, SLOT(onHelpAbout()));
    aboutAct->setStatusTip(tr("Copyrights."));
    fileToolBar->addAction(aboutAct);


}
void MainWindow::updateActions()
{
    mSaveAsAct->setEnabled(!mImageView->getImage()->isNull());
    mStep1Act->setEnabled(!mImageView->getImage()->isNull());
    mStep2Act->setEnabled(!mImageView->getImage()->isNull());
    mStep3Act->setEnabled(!mImageView->getImage()->isNull());
    mStep4Act->setEnabled(!mImageView->getImage()->isNull());
    mZoomInAct->setEnabled(!mFitToWindowAct->isChecked());
    mZoomOutAct->setEnabled(!mFitToWindowAct->isChecked());
    mNormalSizeAct->setEnabled(!mFitToWindowAct->isChecked());
}
void MainWindow::scaleImage(double factor)
{
//    Q_ASSERT(mImageLabel->pixmap());
    mZoomFactor *= factor;
 //   mImageLabel->resize(mZoomFactor * mImageLabel->pixmap()->size());
    mImageView->scaleImage(mZoomFactor);
    adjustScrollBar(mImageView->horizontalScrollBar(), factor);
    adjustScrollBar(mImageView->verticalScrollBar(), factor);

    mZoomInAct->setEnabled(mZoomFactor < 10.0);
    mZoomOutAct->setEnabled(mZoomFactor > 0.1);
}
void MainWindow::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}


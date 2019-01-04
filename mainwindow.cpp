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
#include "controlpanel.h"
#include "common.h"
#include "./imglab/ImgProcess.h"
#include "fpview.h"

TexProcess* gpTexProcess=NULL;
MainWindow* gpMainWin = NULL;
nfImage* gpInputImage = NULL;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),ui(new Ui::MainWindow),    
    mZoomFactor(1)
{
    ui->setupUi(this);
    mCurImgViewId = -1;
    for (int i=0; i< ControlPanel::STEP_MAX; i++)
        mImageView[i] = NULL;
    gpInputImage = NULL; /* input 4-cam image */
    createMenuAndToolbar();
    createUi();
    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);
    gpTexProcess = NULL;
    gpMainWin = this;
    doEnableFlowStep(0);
}

MainWindow::~MainWindow()
{
    delete ui;
    if (gpTexProcess){
        delete gpTexProcess;
        gpTexProcess = NULL;
    }
}
/* called by Controlx->start() */
void MainWindow::changeView(int id)
{
    if (id != mCurImgViewId){
        mImageView[id] = ImageWin::createImageView(id);
        Q_ASSERT(mImageView[id]->isWindow());
        setCentralWidget(mImageView[id]);//old one will be killed by Qt
        mCurImgViewId = id;
        mImageView[mCurImgViewId]->setVisible(true);
    }
}

bool MainWindow::loadFile(const QString &fileName)
{
    if (!gpTexProcess) {
        gpTexProcess = new TexProcess();
    }
    if (!gpTexProcess->loadIniFile(fileName.toStdString().c_str()))
    {
        delete gpTexProcess;
        gpTexProcess = NULL;
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Failed to load project file %1!")
                                 .arg(QDir::toNativeSeparators(fileName)));
        return false;
    }
    nfImage* pImg = gpTexProcess->getSourceImage();
    if (pImg == NULL){
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Failed to load project image specified in project file %1!")
                                 .arg(QDir::toNativeSeparators(fileName)));
        return false;

    }
    if (gpInputImage)
    {   //QImage owns the buffer data, so we don't need to free the buffer while delete the object
        nfImage::dettach(&gpInputImage);
    }
    gpInputImage =  pImg;

    setWindowFilePath(fileName);
    if (gpInputImage) {
        const QString message = tr("Load file \"%1\", image %2x%3")
            .arg(QDir::toNativeSeparators(fileName)).arg(gpInputImage->width).arg(gpInputImage->height);
        statusBar()->showMessage(message);
    }
    return true;
}

void MainWindow::setImage(nfImage*  pImage)
{
    if( mImageView[mCurImgViewId]) {
        mImageView[mCurImgViewId]->setImage(pImage);
        mImageView[mCurImgViewId]->update();
        mFitToWindowAct->setEnabled(true);
        updateActions();

        if (!mFitToWindowAct->isChecked())
             mImageView[mCurImgViewId]->adjustSize();
    }
}
bool MainWindow::saveFile(const QString &fileName)
{
    if(!gpTexProcess){

    }
    if(!gpTexProcess->saveIniFile(fileName.toStdString().c_str())) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot write %1")
                                 .arg(QDir::toNativeSeparators(fileName)));
        return false;
    }
    const QString message = tr("Wrote \"%1\"").arg(QDir::toNativeSeparators(fileName));
    statusBar()->showMessage(message);
    return true;
}


void MainWindow::onFileOpen()
{
    QString filename = QFileDialog::getOpenFileName(this,tr("Open Project"), QDir::currentPath(),
          tr("Project setting file (*.ini)") );

    if (!filename.isNull()){
        loadFile(filename);
        //reset steps to step0
        doEnableFlowStep(0);
    }

}

void MainWindow::onFileSaveAs()
{
    QString filename = QFileDialog::getSaveFileName(this,tr("Open Project"), QDir::currentPath(),
          tr("Project setting file (*.ini)") );

    if (!filename.isNull()){
            saveFile(filename);
    }

}
void MainWindow::doEnableFlowStep(int id)
{
    for (int i=0; i< ControlPanel::STEP_MAX; i++){
        mStepsAct[i]->setChecked(i==id);
        mDockView[i]->setVisible(i==id);
    }
    mControlPanel[id]->start();
}
void MainWindow::onFlowStep0(){doEnableFlowStep(0);}
void MainWindow::onFlowStep1(){doEnableFlowStep(1);}
void MainWindow::onFlowStep2(){doEnableFlowStep(2);}
void MainWindow::onFlowStep3(){doEnableFlowStep(3);}
void MainWindow::onFlowStep4(){doEnableFlowStep(4);}

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
     mImageView[mCurImgViewId]->adjustSize();
    mZoomFactor = 1.0;
}

void MainWindow::onViewFitToWindow()
{
    bool fitToWindow = mFitToWindowAct->isChecked();
     mImageView[mCurImgViewId]->setWidgetResizable(fitToWindow);
    if (!fitToWindow)
        onViewNormalSize();
    updateActions();
}
void MainWindow::onViewShowRuler()
{
     mImageView[mCurImgViewId]->showRulers(! mImageView[mCurImgViewId]->isRulersShown());
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

    mDockView[0] = new QDockWidget(tr("Welcome"), this);
    mDockView[0]->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    mControlPanel[0] = ControlPanel::create(ControlPanel::STEP_0, mDockView[0]);
    mDockView[0]->setWidget(mControlPanel[0]);
    addDockWidget(Qt::LeftDockWidgetArea, mDockView[0]);
    mDockView[0]->setVisible(true);

    for (int i=1; i< ControlPanel::STEP_MAX; i++) {
        mDockView[i] = new QDockWidget(tr("%1").arg(i), this);
        mDockView[i]->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
        mControlPanel[i] = ControlPanel::create((ControlPanel::TYPE)i, mDockView[i]);
        mDockView[i]->setWidget(mControlPanel[i]);
        addDockWidget(Qt::LeftDockWidgetArea, mDockView[i]);
        mDockView[i]->setVisible(false);
    }

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
    mStepsAct[1] = flowMenu->addAction(step1Icon, tr("Step &1"), this,
                         SLOT(onFlowStep1()),tr("Ctrl+1"));
    mStepsAct[1] ->setEnabled(false);
    mStepsAct[1] ->setCheckable(true);

    const QIcon step2Icon =  QIcon(":/images/step2.png");
    mStepsAct[2]  = flowMenu->addAction(step2Icon, tr("Step &2"), this,
                         SLOT(onFlowStep2()),tr("Ctrl+2"));
    mStepsAct[2]->setEnabled(false);
    mStepsAct[2]->setCheckable(true);
    const QIcon step3Icon =  QIcon(":/images/step3.png");
    mStepsAct[3] = flowMenu->addAction(step3Icon, tr("Step &3"), this,
                         SLOT(onFlowStep3()),tr("Ctrl+3"));
    mStepsAct[3]->setEnabled(false);
    mStepsAct[3]->setCheckable(true);
    const QIcon step4Icon =  QIcon(":/images/step4.png");
    mStepsAct[4] = flowMenu->addAction(step4Icon, tr("Step &4"), this,
                         SLOT(onFlowStep4()),tr("Ctrl+4"));
    mStepsAct[4]->setEnabled(false);
    mStepsAct[4]->setCheckable(true);

    mStepsAct[0] = flowMenu->addAction(tr("Resume"), this,
                         SLOT(onFlowStep0()),tr("Ctrl+0"));
    mStepsAct[0] ->setEnabled(true);
    mStepsAct[0] ->setCheckable(true);

    fileToolBar->addAction(mStepsAct[1]);
    fileToolBar->addAction(mStepsAct[2]);
    fileToolBar->addAction(mStepsAct[3]);
    fileToolBar->addAction(mStepsAct[4]);
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
    mSaveAsAct->setEnabled(gpTexProcess);
    for (int i=1; i< ControlPanel::STEP_MAX; i++)
        mStepsAct[i]->setEnabled(gpTexProcess!= NULL);

    mZoomInAct->setEnabled(!mFitToWindowAct->isChecked());
    mZoomOutAct->setEnabled(!mFitToWindowAct->isChecked());
    mNormalSizeAct->setEnabled(!mFitToWindowAct->isChecked());
}
void MainWindow::scaleImage(double factor)
{
    mZoomFactor *= factor;
    mImageView[mCurImgViewId]->scaleImage(mZoomFactor);
    adjustScrollBar( mImageView[mCurImgViewId]->horizontalScrollBar(), factor);
    adjustScrollBar( mImageView[mCurImgViewId]->verticalScrollBar(), factor);

    mZoomInAct->setEnabled(mZoomFactor < 10.0);
    mZoomOutAct->setEnabled(mZoomFactor > 0.1);
}
void MainWindow::adjustScrollBar(QScrollBar *scrollBar, double factor)
{
    scrollBar->setValue(int(factor * scrollBar->value()
                            + ((factor - 1) * scrollBar->pageStep()/2)));
}

void MainWindow::sendMessage(unsigned int command, long  data){
    mImageView[mCurImgViewId]->processMessage(command, data);
}

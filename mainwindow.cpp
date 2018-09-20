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
#include "control0.h"
#include "control1.h"
#include "control2.h"
#include "common.h"
#include "./imglab/ImgProcess.h"
TexProcess* gpTexProcess=NULL;

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),ui(new Ui::MainWindow),
    mImageView(new ImageWin),
    mZoomFactor(1)
{
    ui->setupUi(this);


    createMenuAndToolbar();
    createUi();
    resize(QGuiApplication::primaryScreen()->availableSize() * 3 / 5);
    gpTexProcess = NULL;
}

MainWindow::~MainWindow()
{
    delete ui;
    if (gpTexProcess){
        delete gpTexProcess;
        gpTexProcess = NULL;
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
    QImage* newImage = new QImage(pImg->buffer,
            pImg->width, pImg->height, QImage::Format_RGBA8888);
    nfImage::dettach(&pImg);

    if (newImage->isNull()) {
        QMessageBox::information(this, QGuiApplication::applicationDisplayName(),
                                 tr("Cannot create image"));
        return false;
    }

    setImage(*newImage);

    setWindowFilePath(fileName);

    const QString message = tr("Load file \"%1\", image %2x%3")
        .arg(QDir::toNativeSeparators(fileName)).arg(newImage->width()).arg(newImage->height());
    statusBar()->showMessage(message);
    return true;
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
void MainWindow::onFlowStep1()
{
    mStep1Act->setChecked(true);
    mStep2Act->setChecked(false);
    mStep3Act->setChecked(false);
    mStep4Act->setChecked(false);
    mDockView[0]->setVisible(false);
    mDockView[1]->setVisible(true);
    mDockView[2]->setVisible(false);
    mDockView[3]->setVisible(false);
    mDockView[4]->setVisible(false);

}
void MainWindow::onFlowStep2()
{
    mStep1Act->setChecked(false);
    mStep2Act->setChecked(true);
    mStep3Act->setChecked(false);
    mStep4Act->setChecked(false);
    mDockView[0]->setVisible(false);
    mDockView[1]->setVisible(false);
    mDockView[2]->setVisible(true);
    mDockView[3]->setVisible(false);
    mDockView[4]->setVisible(false);
}
void MainWindow::onFlowStep3()
{
    mStep1Act->setChecked(false);
    mStep2Act->setChecked(false);
    mStep3Act->setChecked(true);
    mStep4Act->setChecked(false);
    mDockView[0]->setVisible(false);
    mDockView[1]->setVisible(false);
    mDockView[2]->setVisible(false);
    mDockView[3]->setVisible(true);
    mDockView[4]->setVisible(false);
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
    mDockView[3]->setVisible(false);
    mDockView[4]->setVisible(true);
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

    mDockView[0] = new QDockWidget(tr("Welcome"), this);
    mDockView[0]->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    Control0* step0Widget = new Control0(mDockView[0]);
    mDockView[0]->setWidget(step0Widget);
    addDockWidget(Qt::LeftDockWidgetArea, mDockView[0]);
    mDockView[0]->setVisible(true);

    mDockView[1] = new QDockWidget(tr("Feature Points"), this);
    mDockView[1]->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    Control1 * step1Widget = new Control1(mDockView[1]);
    mDockView[1]->setWidget(step1Widget);
    addDockWidget(Qt::LeftDockWidgetArea, mDockView[1]);
    mDockView[1]->setVisible(false);

    mDockView[2] = new QDockWidget(tr("FEC"), this);
    mDockView[2]->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    Control2* step2Widget = new Control2(mDockView[2]);
    mDockView[2]->setWidget(step2Widget);
    addDockWidget(Qt::LeftDockWidgetArea, mDockView[2]);
    mDockView[2]->setVisible(false);

    mDockView[3] = new QDockWidget(tr("Homograph"), this);
    mDockView[3]->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    QListWidget* step3Widget = new QListWidget(mDockView[3]);
    step3Widget->addItems(QStringList()
            << "Final image");
    mDockView[3]->setWidget(step3Widget);
    addDockWidget(Qt::LeftDockWidgetArea, mDockView[3]);
    mDockView[3]->setVisible(false);

    mDockView[4] = new QDockWidget(tr("Stitching"), this);
    mDockView[4]->setAllowedAreas(Qt::LeftDockWidgetArea | Qt::RightDockWidgetArea);
    QListWidget* step4Widget = new QListWidget(mDockView[3]);
    step4Widget->addItems(QStringList()
            << "Final image");
    mDockView[4]->setWidget(step4Widget);
    addDockWidget(Qt::LeftDockWidgetArea, mDockView[4]);
    mDockView[4]->setVisible(false);

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
    mSaveAsAct->setEnabled(gpTexProcess);
    mStep1Act->setEnabled(gpTexProcess);
    mStep2Act->setEnabled(gpTexProcess);
    mStep3Act->setEnabled(gpTexProcess);
    mStep4Act->setEnabled(gpTexProcess);
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


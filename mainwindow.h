#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
#include "controlpanel.h"
class QAction;

class QMenu;
class ImageWin;
class QScrollArea;
class QScrollBar;

namespace Ui {
class MainWindow;
}


class nfImage;
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool loadFile(const QString &);
    void doEnableFlowStep(int id);
    void changeView(int id);
    void setImage(nfImage*  pImage);
    void sendMessage(unsigned int command, long data);
protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
     void onFileOpen();
     void onFileSaveAs();
     void onFlowStep0();
     void onFlowStep1();
     void onFlowStep2();
     void onFlowStep3();
     void onFlowStep4();
     void onViewZoomin();
     void onViewZoomout();
     void onViewNormalSize();
     void onViewFitToWindow();
     void onViewShowRuler();
     void onHelpAbout();
private:
    void createMenuAndToolbar();
    void createUi();
    void updateActions();
    bool saveFile(const QString &fileName);

    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    Ui::MainWindow *ui;
    QDockWidget* mDockView[ControlPanel::STEP_MAX];
    ControlPanel* mControlPanel[ControlPanel::STEP_MAX];
    QAction *mStepsAct[ControlPanel::STEP_MAX];
    ImageWin *mImageView[ControlPanel::STEP_MAX];
    int mCurImgViewId;

    double mZoomFactor;
    QAction *mSaveAsAct;
    QAction *mZoomInAct;
    QAction *mZoomOutAct;
    QAction *mNormalSizeAct;
    QAction *mFitToWindowAct;

};

#endif // MAINWINDOW_H

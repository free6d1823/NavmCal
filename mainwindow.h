#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QCloseEvent>
class QAction;

class QMenu;
class ImageWin;
class QScrollArea;
class QScrollBar;

namespace Ui {
class MainWindow;
}
#define MAX_LEFT_PANELS 5
class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();
    bool loadFile(const QString &);

protected:
    void closeEvent(QCloseEvent *event) override;

private slots:
     void onFileOpen();
     void onFileSaveAs();
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
    void setImage(const QImage &newImage);
    void scaleImage(double factor);
    void adjustScrollBar(QScrollBar *scrollBar, double factor);

    Ui::MainWindow *ui;
    QDockWidget* mDockView[MAX_LEFT_PANELS];
    ImageWin *mImageView;
    double mZoomFactor;
    QAction *mSaveAsAct;
    QAction *mZoomInAct;
    QAction *mZoomOutAct;
    QAction *mNormalSizeAct;
    QAction *mFitToWindowAct;
    QAction *mStep1Act;
    QAction *mStep2Act;
    QAction *mStep3Act;
    QAction *mStep4Act;

};

#endif // MAINWINDOW_H

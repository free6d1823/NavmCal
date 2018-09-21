#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>


class ControlPanel : public QWidget
{
    Q_OBJECT
public:
    enum TYPE {
        STEP_0 = 0,
        STEP_1 = 1,
        STEP_2 = 2,
        STEP_3 = 3,
        STEP_4 = 4,
        STEP_MAX
    };
static ControlPanel* create(TYPE id, QWidget *parent = 0);
    explicit ControlPanel(TYPE id, QWidget *parent = 0);
    ///
    /// \brief start to prepare things when this step is started
    ///
    virtual void start(){};
    ///
    /// \brief stop to do things before this step is fnished
    ///
    virtual void stop(){};
signals:

public slots:
private:
    void createUi();
protected:
    TYPE mId;

};
///////////////////////////////////////
namespace Ui {
class Control0;
}

class Control0 : public ControlPanel
{
    Q_OBJECT

public:
    explicit Control0(TYPE id,QWidget *parent = 0);
    ~Control0();
    ///
    /// \brief start to prepare things when this step is started
    ///
    virtual void start();
    ///
    /// \brief stop to do things before this step is fnished
    ///
    virtual void stop();
private:
    Ui::Control0 *ui;
};
///////////////////////////////////////

namespace Ui {
class Control1;
}

class Control1 : public ControlPanel
{
    Q_OBJECT

public:
    explicit Control1(TYPE id, QWidget *parent = 0);
    ~Control1();
    ///
    /// \brief start to prepare things when this step is started
    ///
    virtual void start();
    ///
    /// \brief stop to do things before this step is fnished
    ///
    virtual void stop();
public slots:
    void onCamera0();
    void onCamera1();
    void onCamera2();
    void onCamera3();
    void onShowFp(bool show);
    void onAutoDetection();
private:
    void loadCamera(int cam);
    void createUi();
    Ui::Control1 *ui;
};
//////////////////////////////////////////
namespace Ui {
class Control2;
}

class Control2 : public ControlPanel
{
    Q_OBJECT

public:
    explicit Control2(TYPE id, QWidget *parent = 0);
    ~Control2();
    ///
    /// \brief start to prepare things when this step is started
    ///
    virtual void start();
    ///
    /// \brief stop to do things before this step is fnished
    ///
    virtual void stop();
private:
    Ui::Control2 *ui;
};
///////////////////////////////////////////////////////////

namespace Ui {
class Control3;
}

class Control3 : public ControlPanel
{
    Q_OBJECT

public:
    explicit Control3(TYPE id, QWidget *parent = 0);
    ~Control3();
    ///
    /// \brief start to prepare things when this step is started
    ///
    virtual void start();
    ///
    /// \brief stop to do things before this step is fnished
    ///
    virtual void stop();
private:
    Ui::Control3 *ui;
};
///////////////////////////////////////////////////////////

namespace Ui {
class Control4;
}

class Control4 : public ControlPanel
{
    Q_OBJECT

public:
    explicit Control4(TYPE id, QWidget *parent = 0);
    ~Control4();
    ///
    /// \brief start to prepare things when this step is started
    ///
    virtual void start();
    ///
    /// \brief stop to do things before this step is fnished
    ///
    virtual void stop();
private:
    Ui::Control4 *ui;
};
#endif // CONTROLPANEL_H

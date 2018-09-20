#ifndef CONTROLPANEL_H
#define CONTROLPANEL_H

#include <QWidget>

class ControlPanel : public QWidget
{
    Q_OBJECT
public:
    enum TYPE {
        STEP_0 = 0,
        STEP_1,
        STEP_2,
        STEP_3,
        STEP_4,
        STEP_MAX
    };

    explicit ControlPanel(TYPE id, QWidget *parent = 0);

signals:

public slots:
private:
    void createUi();
    TYPE mId;
};

#endif // CONTROLPANEL_H

#ifndef CONTROL1_H
#define CONTROL1_H

#include <QWidget>

namespace Ui {
class Control1;
}

class Control1 : public QWidget
{
    Q_OBJECT

public:
    explicit Control1(QWidget *parent = 0);
    ~Control1();

private:
    Ui::Control1 *ui;
};

#endif // CONTROL1_H

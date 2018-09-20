#ifndef CONTROL2_H
#define CONTROL2_H

#include <QWidget>

namespace Ui {
class Control2;
}

class Control2 : public QWidget
{
    Q_OBJECT

public:
    explicit Control2(QWidget *parent = 0);
    ~Control2();

private:
    Ui::Control2 *ui;
};

#endif // CONTROL2_H

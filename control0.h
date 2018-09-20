#ifndef CONTROL0_H
#define CONTROL0_H

#include <QWidget>

namespace Ui {
class Control0;
}

class Control0 : public QWidget
{
    Q_OBJECT

public:
    explicit Control0(QWidget *parent = 0);
    ~Control0();

private:
    Ui::Control0 *ui;
};

#endif // CONTROL0_H

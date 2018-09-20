#include "controlpanel.h"
#include <QVBoxLayout>
#include <QLabel>
ControlPanel::ControlPanel(TYPE id, QWidget *parent) : QWidget(parent), mId(id)
{

}
void ControlPanel::createUi()
{
    QVBoxLayout *boxLayout = new QVBoxLayout(this);
    setLayout(boxLayout);
    QLabel* label1 = new QLabel(tr("nAVM Calibration"));
    label1->setAlignment(Qt::Al);
    boxLayout->addWidget(label1);
    QLabel* label2 = new QLabel(tr("Please open a project file then follow the following steps"));
    label2->setWordWrap(true);


}

#include "controlpanel.h"
#include <QVBoxLayout>
#include <QLabel>

ControlPanel* ControlPanel::create(TYPE id, QWidget *parent)
{
    ControlPanel* pPanel = NULL;
    switch (id) {
    case STEP_1:
        pPanel = (ControlPanel*)new Control1(id, parent);
        break;
    case STEP_2:
        pPanel = (ControlPanel*)new Control2(id, parent);
        break;
    case STEP_3:
        pPanel = (ControlPanel*)new Control3(id, parent);
        break;
    case STEP_4:
        pPanel = (ControlPanel*)new Control4(id, parent);
        break;
    case STEP_0:
    default:
        pPanel = (ControlPanel*)new Control0(id, parent);
        break;
    }
    return pPanel;
}

ControlPanel::ControlPanel(TYPE id, QWidget *parent) : QWidget(parent), mId(id)
{

}
void ControlPanel::createUi()
{
}

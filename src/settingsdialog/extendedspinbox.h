#ifndef EXTENTEDSPINBOX_H
#define EXTENTEDSPINBOX_H
#include <QSpinBox>

class ExtendedSpinbox : public QSpinBox
{
    Q_OBJECT

    int MinimumShift_, MaximumShift_;
public:
    ExtendedSpinbox (QWidget *parent = 0);
    void SetMinimumShift (int);
    void SetMaximumShift (int);
public slots:
    void changeMinimum (int);
    void changeMaximum (int);
};

#endif


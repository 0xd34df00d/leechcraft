#ifndef RADIOGROUP_H
#define RADIOGROUP_H
#include <QWidget>

class QButtonGroup;
class QRadioButton;

class RadioGroup : public QWidget
{
 Q_OBJECT

 QString Value_;
 QButtonGroup *Group_;
public:
 RadioGroup (QWidget *parent = 0);
 void AddButton (QRadioButton*, bool def = false);
 QString GetValue () const;
private slots:
 void handleToggled (bool);
signals:
 void valueChanged ();
};

#endif


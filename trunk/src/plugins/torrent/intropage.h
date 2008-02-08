#ifndef INTROPAGE_H
#define INTROPAGE_H
#include <QWizardPage>
class QLabel;

class IntroPage : public QWizardPage
{
 Q_OBJECT

 QLabel *Label_;
public:
 IntroPage (QWidget *parent = 0);
};

#endif


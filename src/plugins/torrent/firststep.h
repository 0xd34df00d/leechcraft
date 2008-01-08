#ifndef FIRSTSTEP_H
#define FIRSTSTEP_H
#include <QWizardPage>
#include "ui_newtorrentfirststep.h"

class FirstStep : public QWizardPage, private Ui::NewTorrentFirstStep
{
	Q_OBJECT
public:
	FirstStep (QWidget *parent = 0);
};

#endif


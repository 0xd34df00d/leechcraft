#ifndef SECONDSTEP_H
#define SECONDSTEP_H
#include <QWizardPage>
#include "ui_newtorrentsecondstep.h"

class SecondStep : public QWizardPage, private Ui::NewTorrentSecondStep
{
 Q_OBJECT
public:
 SecondStep (QWidget *parent = 0);
 QStringList GetPaths () const;
private slots:
 void on_AddPath__released ();
 void on_RemoveSelected__released ();
 void on_Clear__released ();
};

#endif


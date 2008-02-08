#ifndef JOBADDERDIALOG_H
#define JOBADDERDIALOG_H
#include <QDialog>
#include "ui_jobadderdialog.h"

class QPushButton;
class QLineEdit;
class QLabel;
class JobParams;
class QCheckBox;

class JobAdderDialog : public QDialog, private Ui::JobAdderDialog
{
 Q_OBJECT
public:
 JobAdderDialog (QWidget *parent = 0);
 void SetURL (const QString&);
public slots:
 virtual void done (int r);
private slots:
 void on_BrowseButton__released ();
 void on_URL__textChanged ();
signals:
 void gotParams (JobParams*);
};

#endif


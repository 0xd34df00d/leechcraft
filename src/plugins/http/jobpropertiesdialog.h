#ifndef JOBPROPERTIESDIALOG_H
#define JOBPROPERTIESDIALOG_H
#include <QDialog>
#include "ui_jobadderdialog.h"

class JobRepresentation;
class JobParams;

class JobPropertiesDialog : public QDialog, private Ui::JobAdderDialog
{
 Q_OBJECT
public:
 JobPropertiesDialog (JobRepresentation*, QWidget *parent = 0);
 JobParams* GetParams () const;
};

#endif


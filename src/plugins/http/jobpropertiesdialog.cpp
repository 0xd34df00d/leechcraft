#include "jobpropertiesdialog.h"
#include "jobrepresentation.h"
#include "jobparams.h"

JobPropertiesDialog::JobPropertiesDialog (JobRepresentation *rep, QWidget *parent)
: QDialog (parent)
{
    setupUi (this);
    Autostart_->hide ();

    URL_->setText (rep->URL_);
    LocalName_->setText (rep->LocalName_);
}

JobParams* JobPropertiesDialog::GetParams () const
{
    JobParams *result = new JobParams;
    result->URL_ = URL_->text ();
    result->LocalName_ = LocalName_->text ();
    return result;
}


#include "fileexistsdialog.h"

FileExistsDialog::FileExistsDialog (QWidget *parent)
: QDialog (parent)
{
	setupUi (this);
	setWindowModality (Qt::ApplicationModal);
}

FileExistsDialog::Result FileExistsDialog::GetSelected () const
{
	if (RadioCancel->isChecked ())
		return Abort;
	if (RadioContinue->isChecked ())
		return Continue;
	if (RadioUnique->isChecked ())
		return Unique;
	if (RadioScratch->isChecked ())
		return Scratch;
}


#include "aboutdialog.h"
#include "config.h"

namespace LeechCraft
{
	AboutDialog::AboutDialog (QWidget *parent)
	: QDialog (parent)
	{
		Ui_.setupUi (this);
		Ui_.ProgramName_->setText (tr ("LeechCraft %1")
				.arg (LEECHCRAFT_VERSION));
	}
};


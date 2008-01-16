#include <QtGui/QtGui>
#include "changelogdialog.h"

ChangelogDialog::ChangelogDialog (QWidget *parent)
: QDialog (parent)
{
	setupUi (this);

	QFile f (":/change.log");
	f.open (QIODevice::ReadOnly);

	ChangelogShower_->setCurrentFont (QFont ("Courier New"));
	ChangelogShower_->setFontPointSize (10);
	ChangelogShower_->setPlainText (f.readAll ());
}


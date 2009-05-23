#include <QFileDialog>
#include "movetorrentfiles.h"

MoveTorrentFiles::MoveTorrentFiles (const QString& old, QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
	Ui_.OldLocation_->setText (old);
	Ui_.NewLocation_->setText (old);
}

QString MoveTorrentFiles::GetNewLocation () const
{
	return Ui_.NewLocation_->text ();
}

void MoveTorrentFiles::on_Browse__released ()
{
	QString dir = QFileDialog::getExistingDirectory (this, tr ("New location"),
			Ui_.NewLocation_->text ());
	if (dir.isEmpty () || dir == Ui_.NewLocation_->text ())
		return;
	Ui_.NewLocation_->setText (dir);
}


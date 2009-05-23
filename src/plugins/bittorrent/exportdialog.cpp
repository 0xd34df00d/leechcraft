#include "exportdialog.h"
#include <QFileDialog>
#include <QDir>

ExportDialog::ExportDialog (QWidget *parent)
: QDialog (parent)
{
	Ui_.setupUi (this);
}

ExportDialog::~ExportDialog ()
{
}

QString ExportDialog::GetLocation () const
{
	return Ui_.SaveLine_->text ();
}

bool ExportDialog::GetSettings () const
{
	return Ui_.SettingsBox_->checkState () == Qt::Checked;
}

bool ExportDialog::GetActive () const
{
	return Ui_.TorrentsBox_->checkState () == Qt::Checked;
}

void ExportDialog::on_BrowseButton__released ()
{
	QString filename = QFileDialog::getSaveFileName (this,
			tr ("Save file"),
			QDir::homePath () + "/export.lcte",
			tr ("BitTorrent Exchange (*.lcte)"));
	if (filename.isEmpty ())
		return;

	Ui_.SaveLine_->setText (filename);
}


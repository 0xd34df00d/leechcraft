#include "exportopml.h"
#include <QFileInfo>
#include <QFileDialog>
#include <QDir>

ExportOPML::ExportOPML ()
{
	Ui_.setupUi (this);
}

ExportOPML::~ExportOPML ()
{
}

QString ExportOPML::GetDestination () const
{
	return Ui_.File_->text ();
}

void ExportOPML::on_File__textEdited (const QString& newFilename)
{
}

void ExportOPML::on_Browse__released ()
{
	QString filename = QFileDialog::getSaveFileName (this,
			tr ("Select OPML save file"),
			QDir::homePath (),
			tr ("OPML files (*.opml);;"
				"XML files (*.xml);;"
				"All files (*.*)"));
	if (filename.isEmpty ())
		return;

	Ui_.File_->setText (filename);
}


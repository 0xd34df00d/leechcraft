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

QString ExportOPML::GetTitle () const
{
	return Ui_.Title_->text ();
}

QString ExportOPML::GetOwner () const
{
	return Ui_.Owner_->text ();
}

QString ExportOPML::GetOwnerEmail () const
{
	return Ui_.OwnerEmail_->text ();
}

std::vector<bool> ExportOPML::GetSelectedFeeds () const
{
	std::vector<bool> result (Ui_.Channels_->topLevelItemCount ());

	for (int i = 0, items = Ui_.Channels_->topLevelItemCount ();
			i < items; ++i)
		result [i] = (Ui_.Channels_->topLevelItem (i)->
				data (0, Qt::CheckStateRole) == Qt::Checked);

	return result;
}

void ExportOPML::SetFeeds (const channels_shorts_t& channels)
{
	for (channels_shorts_t::const_iterator i = channels.begin (),
			end = channels.end (); i != end; ++i)
	{
		QStringList strings;
		strings << i->Title_ << i->ParentURL_;

		QTreeWidgetItem *item =
			new QTreeWidgetItem (Ui_.Channels_, strings);
		item->setData (0, Qt::CheckStateRole, Qt::Checked);
	}
}

void ExportOPML::on_File__textEdited (const QString&)
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


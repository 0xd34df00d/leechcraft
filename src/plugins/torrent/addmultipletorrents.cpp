#include <QFileDialog>
#include "addmultipletorrents.h"
#include "settingsmanager.h"

AddMultipleTorrents::AddMultipleTorrents (QWidget *parent)
: QDialog (parent)
{
	setupUi (this);
	OpenDirectory_->setText (SettingsManager::Instance ()->GetLastTorrentDirectory ());
	SaveDirectory_->setText (SettingsManager::Instance ()->GetLastSaveDirectory ());
}

QString AddMultipleTorrents::GetOpenDirectory () const
{
	return OpenDirectory_->text ();
}

QString AddMultipleTorrents::GetSaveDirectory () const
{
	return SaveDirectory_->text ();
}

void AddMultipleTorrents::on_BrowseOpen__released ()
{
	QString dir = QFileDialog::getExistingDirectory (this, tr ("Select directory with torrents"), SettingsManager::Instance ()->GetLastTorrentDirectory ());
	if (dir.isEmpty ())
		return;

	SettingsManager::Instance ()->SetLastTorrentDirectory (dir);
	OpenDirectory_->setText (dir);
}

void AddMultipleTorrents::on_BrowseSave__released ()
{
	QString dir = QFileDialog::getExistingDirectory (this, tr ("Select save directory"), SettingsManager::Instance ()->GetLastSaveDirectory ());
	if (dir.isEmpty ())
		return;

	SettingsManager::Instance ()->SetLastSaveDirectory (dir);
	SaveDirectory_->setText (dir);
}


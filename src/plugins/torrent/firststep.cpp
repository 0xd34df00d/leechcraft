#include <QFileDialog>
#include "firststep.h"
#include "settingsmanager.h"

FirstStep::FirstStep (QWidget *parent)
: QWizardPage (parent)
{
	setupUi (this);
	registerField ("OutputDirectory", OutputDirectory_);
	registerField ("TorrentName*", TorrentName_);
	registerField ("AnnounceURL*", AnnounceURL_);
	registerField ("Date", Date_);
	registerField ("Comment", Comment_);
	registerField ("RootPath", RootPath_);
	Date_->setDateTime (QDateTime::currentDateTime ());
	OutputDirectory_->setText (SettingsManager::Instance ()->GetLastMakeTorrentDirectory ());
	RootPath_->setText (SettingsManager::Instance ()->GetLastAddDirectory ());
}

void FirstStep::on_BrowseOutput__released ()
{
	QString directory = QFileDialog::getExistingDirectory (this,
			tr ("Select where to place torrent file"),
			OutputDirectory_->text ());
	if (directory.isEmpty ())
		return;

	OutputDirectory_->setText (directory);
	SettingsManager::Instance ()->SetLastMakeTorrentDirectory (directory);
}

void FirstStep::on_BrowseRoot__released ()
{
	QString directory = QFileDialog::getExistingDirectory (this,
			tr ("Select root path"),
			SettingsManager::Instance ()->GetLastAddDirectory ());
	if (directory.isEmpty ())
		return;
	RootPath_->setText (directory);
	SettingsManager::Instance ()->SetLastAddDirectory (directory);
}


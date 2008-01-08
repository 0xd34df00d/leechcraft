#include "firststep.h"
#include "settingsmanager.h"

FirstStep::FirstStep (QWidget *parent)
: QWizardPage (parent)
{
	setupUi (this);
	registerField ("OutputDirectory", OutputDirectory_);
	registerField ("TorrentName", TorrentName_);
	registerField ("AnnounceURL", AnnounceURL_);
	registerField ("Date", Date_);
	registerField ("Comment", Comment_);
	Date_->setDateTime (QDateTime::currentDateTime ());
	OutputDirectory_->setText (SettingsManager::Instance ()->GetLastMakeTorrentDirectory ());
}


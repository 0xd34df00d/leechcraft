#include <QFileDialog>
#include "firststep.h"
#include "xmlsettingsmanager.h"

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
    OutputDirectory_->setText (XmlSettingsManager::Instance ()->property ("LastMakeTorrentDirectory").toString ());
    RootPath_->setText (XmlSettingsManager::Instance ()->property ("LastAddDirectory").toString ());
}

void FirstStep::on_BrowseOutput__released ()
{
    QString directory = QFileDialog::getExistingDirectory (this,
            tr ("Select where to place torrent file"),
            OutputDirectory_->text ());
    if (directory.isEmpty ())
        return;

    OutputDirectory_->setText (directory);
    XmlSettingsManager::Instance ()->setProperty ("LastMakeTorrentDirectory", directory);
}

void FirstStep::on_BrowseRoot__released ()
{
    QString directory = QFileDialog::getExistingDirectory (this,
            tr ("Select root path"),
            RootPath_->text ());
    if (directory.isEmpty ())
        return;

    RootPath_->setText (directory);
    XmlSettingsManager::Instance ()->setProperty ("LastAddDirectory", directory);
}


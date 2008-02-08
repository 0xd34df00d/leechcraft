#include <QFileDialog>
#include "addmultipletorrents.h"
#include "xmlsettingsmanager.h"

AddMultipleTorrents::AddMultipleTorrents (QWidget *parent)
: QDialog (parent)
{
    setupUi (this);
    OpenDirectory_->setText (XmlSettingsManager::Instance ()->property ("LastTorrentDirectory").toString ());
    SaveDirectory_->setText (XmlSettingsManager::Instance ()->property ("LastSaveDirectory").toString ());
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
    QString dir = QFileDialog::getExistingDirectory (this, tr ("Select directory with torrents"), OpenDirectory_->text ());
    if (dir.isEmpty ())
        return;

    XmlSettingsManager::Instance ()->setProperty ("LastTorrentDirectory", dir);
    OpenDirectory_->setText (dir);
}

void AddMultipleTorrents::on_BrowseSave__released ()
{
    QString dir = QFileDialog::getExistingDirectory (this, tr ("Select save directory"), SaveDirectory_->text ());
    if (dir.isEmpty ())
        return;

    XmlSettingsManager::Instance ()->setProperty ("LastSaveDirectory", dir);
    SaveDirectory_->setText (dir);
}


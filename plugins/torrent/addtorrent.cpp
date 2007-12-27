#include <QHeaderView>
#include <QFileDialog>
#include <plugininterface/proxy.h>
#include "addtorrent.h"
#include "settingsmanager.h"
#include "core.h"

AddTorrent::AddTorrent (QWidget *parent)
: QDialog (parent)
{
	setupUi (this);
	FileWidget_->header ()->setStretchLastSection (true);
}

void AddTorrent::on_TorrentBrowse__released ()
{
	QString filename = QFileDialog::getOpenFileName (this, tr ("Select torrent file"), SettingsManager::Instance ()->GetLastTorrentDirectory (), tr ("Torrents (*.torrent);;All files (*.*)"));
	if (filename.isEmpty ())
		return;

	SettingsManager::Instance ()->SetLastTorrentDirectory (QFileInfo (filename).absoluteDir ().dirName ());
	TorrentFile_->setText (filename);
	libtorrent::torrent_info info = Core::Instance ()->GetTorrentInfo (filename);
	TrackerURL_->setText (QString::fromStdString (info.trackers ().at (0).url));
	Size_->setText (Proxy::Instance ()->MakePrettySize (info.total_size ()));
	QString creator = QString::fromStdString (info.creator ()),
			comment = QString::fromStdString (info.comment ());
	QString date = QString::fromStdString (to_simple_string (info.creation_date ().get ()));
	if (!creator.isEmpty () && !creator.isNull ())
		Creator_->setText (creator);
	if (!comment.isEmpty () && !comment.isNull ())
		Comment_->setText (comment);
	if (!date.isEmpty () && !date.isNull ())
		Date_->setText (date);
}

void AddTorrent::on_DestinationBrowse__released ()
{
	QString dir = QFileDialog::getExistingDirectory (this, tr ("Select save directory"), SettingsManager::Instance ()->GetLastSaveDirectory ());
	if (dir.isEmpty ())
		return;

	SettingsManager::Instance ()->SetLastSaveDirectory (QDir (dir).absolutePath ());
	Destination_->setText (dir);
}


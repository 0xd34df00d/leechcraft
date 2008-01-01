#include <QHeaderView>
#include <QFileDialog>
#include <plugininterface/proxy.h>
#include <boost/date_time.hpp>
#include "addtorrent.h"
#include "settingsmanager.h"
#include "core.h"

AddTorrent::AddTorrent (QWidget *parent)
: QDialog (parent)
{
	setupUi (this);
	FileWidget_->header ()->setStretchLastSection (true);
	OK_->setEnabled (false);
	connect (this, SIGNAL (on_TorrentFile__textChanged ()), this, SLOT (setOkEnabled ()));
	connect (this, SIGNAL (on_Destination__textChanged ()), this, SLOT (setOkEnabled ()));

	QString dir = SettingsManager::Instance ()->GetLastSaveDirectory ();
	Destination_->setText (dir);
}

void AddTorrent::Reinit ()
{
	while (FileWidget_->topLevelItemCount ())
		delete FileWidget_->takeTopLevelItem (0);
	TorrentFile_->setText ("");
	TrackerURL_->setText (tr ("<unknown>"));
	Size_->setText (tr ("<unknown>"));
	Creator_->setText (tr ("<unknown>"));
	Comment_->setText (tr ("<unknown>"));
	Date_->setText (tr ("<unknown>"));
}

QString AddTorrent::GetFilename () const
{
	return TorrentFile_->text ();
}

QString AddTorrent::GetSavePath () const
{
	return Destination_->text ();
}

void AddTorrent::setOkEnabled ()
{
	OK_->setEnabled (QFileInfo (TorrentFile_->text ()).isReadable () && QFileInfo (Destination_->text ()).exists ());
}

void AddTorrent::on_TorrentBrowse__released ()
{
	QString filename = QFileDialog::getOpenFileName (this, tr ("Select torrent file"), SettingsManager::Instance ()->GetLastTorrentDirectory (), tr ("Torrents (*.torrent);;All files (*.*)"));
	if (filename.isEmpty ())
		return;

	Reinit ();

	SettingsManager::Instance ()->SetLastTorrentDirectory (QFileInfo (filename).absolutePath ());
	TorrentFile_->setText (filename);

	libtorrent::torrent_info info = Core::Instance ()->GetTorrentInfo (filename);
	if (!info.is_valid ())
		return;
	TrackerURL_->setText (QString::fromStdString (info.trackers ().at (0).url));
	Size_->setText (Proxy::Instance ()->MakePrettySize (info.total_size ()));
	QString creator = QString::fromStdString (info.creator ()),
			comment = QString::fromStdString (info.comment ());
	QString date = QString::fromStdString (boost::posix_time::to_simple_string (info.creation_date ().get ()));
	if (!creator.isEmpty () && !creator.isNull ())
		Creator_->setText (creator);
	if (!comment.isEmpty () && !comment.isNull ())
		Comment_->setText (comment);
	if (!date.isEmpty () && !date.isNull ())
		Date_->setText (date);
	for (libtorrent::torrent_info::file_iterator i = info.begin_files (); i != info.end_files (); ++i)
	{
		QTreeWidgetItem *item = new QTreeWidgetItem (FileWidget_);
		item->setText (0, Proxy::Instance ()->MakePrettySize (i->size));
		item->setText (1, QString::fromStdString (i->path.string ()));
	}
}

void AddTorrent::on_DestinationBrowse__released ()
{
	QString dir = QFileDialog::getExistingDirectory (this, tr ("Select save directory"), SettingsManager::Instance ()->GetLastSaveDirectory ());
	if (dir.isEmpty ())
		return;

	SettingsManager::Instance ()->SetLastSaveDirectory (dir);
	Destination_->setText (dir);
}


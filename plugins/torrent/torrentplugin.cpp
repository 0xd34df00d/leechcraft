#include <QtGui>
#include <plugininterface/proxy.h>
#include "torrentplugin.h"
#include "core.h"
#include "addtorrent.h"
#include "settingsmanager.h"

void TorrentPlugin::Init ()
{
	setupUi (this);
	IsShown_ = false;
	SettingsDialog_ = new SettingsDialog (this);
	SettingsDialog_->RegisterObject (SettingsManager::Instance ());
	AddTorrentDialog_ = new AddTorrent (this);
	connect (Core::Instance (), SIGNAL (error (QString)), this, SLOT (showError (QString)));
	connect (Core::Instance (), SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)), this, SLOT (updateTorrentStats ()));
	Core::Instance ()->DoDelayedInit ();
	TorrentView_->setModel (Core::Instance ());
	CurrentRow_ = -1;
}

QString TorrentPlugin::GetName () const
{
	return windowTitle ();
}

QString TorrentPlugin::GetInfo () const
{
	return tr ("BitTorrent client using rb-libtorrent.");
}

QString TorrentPlugin::GetStatusbarMessage () const
{
	return QString ("");
}

IInfo& TorrentPlugin::SetID (IInfo::ID_t id)
{
	ID_ = id;
	return *this;
}

IInfo::ID_t TorrentPlugin::GetID () const
{
	return ID_;
}

QStringList TorrentPlugin::Provides () const
{
	return QStringList ("bittorrent") << "resume";
}

QStringList TorrentPlugin::Needs () const
{
	return QStringList ();
}

QStringList TorrentPlugin::Uses () const
{
	return QStringList ();
}

void TorrentPlugin::SetProvider (QObject*, const QString&)
{
}

void TorrentPlugin::Release ()
{
	Core::Instance ()->Release ();
	SettingsManager::Instance ()->Release ();
}

QIcon TorrentPlugin::GetIcon () const
{
	return windowIcon ();
}

void TorrentPlugin::SetParent (QWidget *w)
{
	setParent (w);
}

void TorrentPlugin::ShowWindow ()
{
	IsShown_ = 1 - IsShown_;
	IsShown_ ? show () : hide ();
}

void TorrentPlugin::ShowBalloonTip ()
{
}

qint64 TorrentPlugin::GetDownloadSpeed () const
{
	return 0;
}

qint64 TorrentPlugin::GetUploadSpeed () const
{
	return 0;
}

void TorrentPlugin::StartAll ()
{
}

void TorrentPlugin::StopAll ()
{
}

void TorrentPlugin::StartAt (ulong)
{
}

void TorrentPlugin::StopAt (ulong)
{
}

void TorrentPlugin::DeleteAt (ulong)
{
}

void TorrentPlugin::closeEvent (QCloseEvent*)
{
	IsShown_ = false;
}

void TorrentPlugin::on_OpenTorrent__triggered ()
{
	if (AddTorrentDialog_->exec () == QDialog::Rejected)
		return;

	QString filename = AddTorrentDialog_->GetFilename (),
			path = AddTorrentDialog_->GetSavePath ();
	Core::Instance ()->AddFile (filename, path);
}

void TorrentPlugin::on_RemoveTorrent__triggered ()
{
	if (CurrentRow_ == TorrentView_->currentIndex ().row ())
		CurrentRow_ = -1;
	Core::Instance ()->RemoveTorrent (TorrentView_->currentIndex ().row ());
}

void TorrentPlugin::on_Resume__triggered ()
{
	Core::Instance ()->ResumeTorrent (TorrentView_->currentIndex ().row ());
}

void TorrentPlugin::on_Stop__triggered ()
{
	Core::Instance ()->PauseTorrent (TorrentView_->currentIndex ().row ());
}

void TorrentPlugin::on_Preferences__triggered ()
{
	SettingsDialog_->show ();
	SettingsDialog_->setWindowTitle (windowTitle () + tr (": Preferences"));
}

void TorrentPlugin::on_TorrentView__clicked (const QModelIndex& index)
{
	CurrentRow_ = index.row ();
	updateTorrentStats ();
}

void TorrentPlugin::on_TorrentView__pressed (const QModelIndex& index)
{
	CurrentRow_ = index.row ();
	updateTorrentStats ();
}

void TorrentPlugin::setActionsEnabled ()
{
}

void TorrentPlugin::showError (QString e)
{
	qWarning () << e;
	QMessageBox::warning (this, tr ("Error!"), e);
}

void TorrentPlugin::updateTorrentStats ()
{
	if (CurrentRow_ == -1)
	{
		LabelState_->setText ("<>");
		LabelTracker_->setText ("<>");
		LabelProgress_->setText ("<>");
		LabelDHTNodesCount_->setText ("<>");
		LabelDownloaded_->setText ("<>");
		LabelTotalSize_->setText ("<>");
		LabelFailed_->setText ("<>");
		LabelConnectedSeeds_->setText ("<>");
		LabelConnectedPeers_->setText ("<>");
		LabelNextAnnounce_->setText ("<>");
		LabelAnnounceInterval_->setText ("<>");
		LabelTotalPieces_->setText ("<>");
		LabelDownloadedPieces_->setText ("<>");
		LabelPieceSize_->setText ("<>");
		LabelDownloadRate_->setText ("<>");
		LabelUploadRate_->setText ("<>");
	}
	else
	{
		TorrentInfo i = Core::Instance ()->GetTorrentStats (CurrentRow_);
		LabelState_->setText (i.State_);
		LabelTracker_->setText (i.Tracker_);
		LabelProgress_->setText (QString::number (i.Progress_ * 100) + "%");
		LabelDHTNodesCount_->setText (QString::number (i.DHTNodesCount_));
		LabelDownloaded_->setText (Proxy::Instance ()->MakePrettySize (i.Downloaded_));
		LabelTotalSize_->setText (Proxy::Instance ()->MakePrettySize (i.TotalSize_));
		LabelFailed_->setText (Proxy::Instance ()->MakePrettySize (i.FailedSize_));
		LabelConnectedPeers_->setText (QString::number (i.ConnectedPeers_));
		LabelConnectedSeeds_->setText (QString::number (i.ConnectedSeeds_));
		LabelNextAnnounce_->setText (i.NextAnnounce_.toString ());
		LabelAnnounceInterval_->setText (i.AnnounceInterval_.toString ());
		LabelTotalPieces_->setText (QString::number (i.TotalPieces_));
		LabelDownloadedPieces_->setText (QString::number (i.DownloadedPieces_));
		LabelPieceSize_->setText (Proxy::Instance ()->MakePrettySize (i.PieceSize_));
		LabelDownloadRate_->setText (Proxy::Instance ()->MakePrettySize (i.DownloadRate_) + tr ("/s"));
		LabelUploadRate_->setText (Proxy::Instance ()->MakePrettySize (i.UploadRate_) + tr ("/s"));
	}
}

Q_EXPORT_PLUGIN2 (leechcraft_torrent, TorrentPlugin);


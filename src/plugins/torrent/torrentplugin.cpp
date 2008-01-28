#include <QtGui/QtGui>
#include <plugininterface/proxy.h>
#include "torrentplugin.h"
#include "core.h"
#include "addtorrent.h"
#include "addmultipletorrents.h"
#include "newtorrentwizard.h"
#include "xmlsettingsmanager.h"

void TorrentPlugin::Init ()
{
    QTranslator *transl = new QTranslator (this);
    QString localeName = QLocale::system ().name ();
    transl->load (QString (":/leechcraft_torrent_") + localeName);
    qApp->installTranslator (transl);

	setupUi (this);
	IsShown_ = false;
	XmlSettingsDialog_ = new XmlSettingsDialog (this);
	XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (), ":/torrentsettings.xml");
	AddTorrentDialog_ = new AddTorrent (this);
	connect (Core::Instance (), SIGNAL (error (QString)), this, SLOT (showError (QString)));
	connect (Core::Instance (), SIGNAL (torrentFinished (const QString&)), this, SIGNAL (downloadFinished (const QString&)));
	connect (Core::Instance (), SIGNAL (fileFinished (const QString&)), this, SIGNAL (fileDownloaded (const QString&)));
	connect (Core::Instance (), SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)), this, SLOT (updateTorrentStats ()));
	connect (Stats_, SIGNAL (currentChanged (int)), this, SLOT (updateTorrentStats ()));
	TorrentView_->setModel (Core::Instance ());
	Core::Instance ()->DoDelayedInit ();
	
	OverallStatsUpdateTimer_ = new QTimer (this);
	connect (OverallStatsUpdateTimer_, SIGNAL (timeout ()), this, SLOT (updateOverallStats ()));
	OverallStatsUpdateTimer_->start (500);

	QFontMetrics fm = fontMetrics ();
	QHeaderView *header = TorrentView_->header ();
	header->resizeSection (Core::ColumnName, fm.width ("thisisanaveragewareztorrentname,right?maybeyes.torrent"));
	header->resizeSection (Core::ColumnDownloaded, fm.width ("_1234.0 KB_"));
	header->resizeSection (Core::ColumnUploaded, fm.width ("_1234.0 KB_"));
	header->resizeSection (Core::ColumnRating, fm.width ("_12.345_"));
	header->resizeSection (Core::ColumnSize, fm.width ("_1234.0 KB_"));
	header->resizeSection (Core::ColumnProgress, fm.width ("___100%___"));
	header->resizeSection (Core::ColumnState, fm.width ("__Downloading__"));
	header->resizeSection (Core::ColumnSP, fm.width ("_123/123_"));
	header->resizeSection (Core::ColumnUSpeed, fm.width ("_1234.0 KB/s_"));
	header->resizeSection (Core::ColumnDSpeed, fm.width ("_1234.0 KB/s_"));
	header->resizeSection (Core::ColumnRemaining, fm.width ("10d 00:00:00"));

	Plugins_->addAction (OpenTorrent_);
	Plugins_->addAction (OpenMultipleTorrents_);
	Plugins_->addAction (CreateTorrent_);
	Plugins_->addSeparator ();
	Plugins_->addAction (Preferences_);
}

QString TorrentPlugin::GetName () const
{
	return windowTitle ();
}

QString TorrentPlugin::GetInfo () const
{
	return tr ("Full-featured BitTorrent client.");
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

void TorrentPlugin::PushMainWindowExternals (const MainWindowExternals& ex)
{
	Plugins_ = ex.RootMenu_->addMenu (tr ("&BitTorrent"));
}

void TorrentPlugin::Release ()
{
	Core::Instance ()->Release ();
	XmlSettingsManager::Instance ()->Release ();
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

bool TorrentPlugin::CouldDownload (const QString& string) const
{
	QFile file (string);
	if (!file.exists () || !file.open (QIODevice::ReadOnly))
		return false;

	return Core::Instance ()->IsValidTorrent (file.readAll ());
}

void TorrentPlugin::AddJob (const QString& name)
{
	AddTorrentDialog_->Reinit ();
	AddTorrentDialog_->SetFilename (name);

	if (AddTorrentDialog_->exec () == QDialog::Rejected)
		return;

	QString filename = AddTorrentDialog_->GetFilename (),
			path = AddTorrentDialog_->GetSavePath ();
	QVector<bool> files = AddTorrentDialog_->GetSelectedFiles ();
	Core::Instance ()->AddFile (filename, path, files);
}

void TorrentPlugin::closeEvent (QCloseEvent*)
{
	IsShown_ = false;
}

void TorrentPlugin::handleHidePlugins ()
{
	IsShown_ = false;
	hide ();
}

void TorrentPlugin::on_OpenTorrent__triggered ()
{
	AddTorrentDialog_->Reinit ();
	if (AddTorrentDialog_->exec () == QDialog::Rejected)
		return;

	QString filename = AddTorrentDialog_->GetFilename (),
			path = AddTorrentDialog_->GetSavePath ();
	QVector<bool> files = AddTorrentDialog_->GetSelectedFiles ();
	Core::Instance ()->AddFile (filename, path, files);
}

void TorrentPlugin::on_OpenMultipleTorrents__triggered ()
{
	AddMultipleTorrents dialog;
	if (dialog.exec () == QDialog::Rejected)
		return;

	QString savePath = dialog.GetSaveDirectory (),
			openPath = dialog.GetOpenDirectory ();
	QDir dir (openPath);
	QStringList names = dir.entryList (QStringList ("*.torrent"));
	for (int i = 0; i < names.size (); ++i)
	{
		QString name = openPath;
		if (!name.endsWith ('/'))
			name += '/';
		name += names.at (i);
		Core::Instance ()->AddFile (name, savePath);
	}
}

void TorrentPlugin::on_CreateTorrent__triggered ()
{
	NewTorrentWizard *wizard = new NewTorrentWizard (this);
	if (wizard->exec () == QDialog::Accepted)
		Core::Instance ()->MakeTorrent (wizard->GetParams ());
}

void TorrentPlugin::on_RemoveTorrent__triggered ()
{
	if (QMessageBox::question (this, tr ("Question"), tr ("Do you really want to delete the torrent?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
		return;

	int row = TorrentView_->currentIndex ().row ();
	if (row == -1)
		return;

	Core::Instance ()->RemoveTorrent (row);
	updateTorrentStats ();
}

void TorrentPlugin::on_Resume__triggered ()
{
	Core::Instance ()->ResumeTorrent (TorrentView_->currentIndex ().row ());
}

void TorrentPlugin::on_Stop__triggered ()
{
	Core::Instance ()->PauseTorrent (TorrentView_->currentIndex ().row ());
}

void TorrentPlugin::on_ForceReannounce__triggered ()
{
	Core::Instance ()->ForceReannounce (TorrentView_->currentIndex ().row ());
}

void TorrentPlugin::on_Preferences__triggered ()
{
	XmlSettingsDialog_->show ();
	XmlSettingsDialog_->setWindowTitle (windowTitle () + tr (": Preferences"));
}

void TorrentPlugin::on_TorrentView__clicked (const QModelIndex&)
{
	updateTorrentStats ();
}

void TorrentPlugin::on_TorrentView__pressed (const QModelIndex&)
{
	updateTorrentStats ();
}

void TorrentPlugin::on_OverallDownloadRateController__valueChanged (int val)
{
	Core::Instance ()->SetOverallDownloadRate (val);
}

void TorrentPlugin::on_OverallUploadRateController__valueChanged (int val)
{
	Core::Instance ()->SetOverallUploadRate (val);
}

void TorrentPlugin::on_DesiredRating__valueChanged (double val)
{
	Core::Instance ()->SetDesiredRating (val);
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
	QModelIndex index = TorrentView_->currentIndex ();
	switch (Stats_->currentIndex ())
	{
		case 0:
			break;
		case 1:
			updateOverallStats ();
			break;
		case 2:
			if (!index.isValid ())
			{
				LabelState_->setText ("<>");
				LabelTracker_->setText ("<>");
				LabelProgress_->setText ("<>");
				LabelDHTNodesCount_->setText ("<>");
				LabelDownloaded_->setText ("<>");
				LabelUploaded_->setText ("<>");
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
				LabelTorrentRating_->setText ("<>");
			}
			else
			{
				TorrentInfo i = Core::Instance ()->GetTorrentStats (index.row ());
				LabelState_->setText (i.State_);
				LabelTracker_->setText (i.Tracker_);
				LabelProgress_->setText (QString::number (i.Progress_ * 100) + "%");
				LabelDHTNodesCount_->setText (QString::number (i.DHTNodesCount_));
				LabelDownloaded_->setText (Proxy::Instance ()->MakePrettySize (i.Downloaded_));
				LabelUploaded_->setText (Proxy::Instance ()->MakePrettySize (i.Uploaded_));
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
				LabelTorrentRating_->setText (QString::number (i.Uploaded_ / static_cast<double> (i.Downloaded_), 'g', 4));
			}
			break;
		case 3:
			FilesWidget_->clear ();
			if (index.isValid ())
			{
				QList<FileInfo> files = Core::Instance ()->GetTorrentFiles (index.row ());
				for (int i = 0; i < files.size (); ++i)
				{
					QTreeWidgetItem *item = new QTreeWidgetItem (FilesWidget_);
					item->setText (0, files.at (i).Name_);
					item->setText (1, Proxy::Instance ()->MakePrettySize (files.at (i).Size_));
					item->setText (2, QString::number (files.at (i).Priority_));
					item->setText (3, QString::number (files.at (i).Progress_ * 100) + "%");
				}
			}
			break;
		case 4:
			PeersWidget_->clear ();
			if (index.isValid ())
			{
				QList<PeerInfo> peers = Core::Instance ()->GetPeers (index.row ());
				for (int i = 0; i < peers.size (); ++i)
				{
					QTreeWidgetItem *item = new QTreeWidgetItem (PeersWidget_);
					item->setText (0, peers.at (i).IP_);
					item->setText (1, peers.at (i).Seed_ ? tr ("true") : ("false"));
					item->setText (2, Proxy::Instance ()->MakePrettySize (peers.at (i).DSpeed_) + tr ("/s"));
					item->setText (3, Proxy::Instance ()->MakePrettySize (peers.at (i).USpeed_) + tr ("/s"));
					item->setText (4, Proxy::Instance ()->MakePrettySize (peers.at (i).Downloaded_));
					item->setText (5, Proxy::Instance ()->MakePrettySize (peers.at (i).Uploaded_));
					item->setText (6, peers.at (i).Client_);
					item->setText (7, peers.at (i).Country_);
				}
			}
			break;
	}
}

void TorrentPlugin::updateOverallStats ()
{
	OverallStats stats = Core::Instance ()->GetOverallStats ();
	LabelTotalDownloadRate_->setText (Proxy::Instance ()->MakePrettySize (static_cast<int> (stats.DownloadRate_)) + tr ("/s"));
	LabelTotalUploadRate_->setText (Proxy::Instance ()->MakePrettySize (static_cast<int> (stats.UploadRate_)) + tr ("/s"));
	LabelTotalDownloaded_->setText (Proxy::Instance ()->MakePrettySize (stats.SessionDownload_));
	LabelTotalUploaded_->setText (Proxy::Instance ()->MakePrettySize (stats.SessionUpload_));
	LabelTotalConnections_->setText (QString::number (stats.NumConnections_));
	LabelUploadConnections_->setText (QString::number (stats.NumUploads_));
	LabelTotalPeers_->setText (QString::number (stats.NumPeers_));
	LabelTotalDHTNodes_->setText (QString::number (stats.NumDHTNodes_));
	LabelDHTTorrents_->setText (QString::number (stats.NumDHTTorrents_));
	LabelListenPort_->setText (QString::number (stats.ListenPort_));
	LabelSessionRating_->setText (QString::number (stats.SessionUpload_ / static_cast<double> (stats.SessionDownload_), 'g', 4));
}

Q_EXPORT_PLUGIN2 (leechcraft_torrent, TorrentPlugin);


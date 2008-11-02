#define WIN32_LEAN_AND_MEAN
#include "torrentplugin.h"
#include <QMessageBox>
#include <QTemporaryFile>
#include <QtDebug>
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QTabWidget>
#include <QTranslator>
#include <QTimer>
#include <QToolBar>
#include <QSortFilterProxyModel>
#include <libtorrent/session.hpp>
#include <plugininterface/proxy.h>
#include <plugininterface/tagscompleter.h>
#include <plugininterface/tagscompletionmodel.h>
#include <plugininterface/util.h>
#include "core.h"
#include "addtorrent.h"
#include "addmultipletorrents.h"
#include "newtorrentwizard.h"
#include "xmlsettingsmanager.h"
#include "piecesmodel.h"
#include "peersmodel.h"
#include "torrentfilesmodel.h"
#include "filesviewdelegate.h"
#include "movetorrentfiles.h"
#include "representationmodel.h"
#include "trackerschanger.h"
#include "historymodel.h"

#ifdef AddJob
#undef AddJob
#endif

void TorrentPlugin::Init ()
{
	Translator_.reset (LeechCraft::Util::InstallTranslator ("torrent"));
    SetupCore ();
    SetupTorrentView ();
	SetupActions ();
    SetupStuff ();

    setActionsEnabled ();

    Ui_.LogShower_->setPlainText ("BitTorrent initialized");
}

TorrentPlugin::~TorrentPlugin ()
{
}

QString TorrentPlugin::GetName () const
{
    return "BitTorrent";
}

QString TorrentPlugin::GetInfo () const
{
    return tr ("Full-featured BitTorrent client.");
}

QStringList TorrentPlugin::Provides () const
{
    return QStringList ("bittorrent") << "resume" << "remoteable";
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
    XmlSettingsManager::Instance ()->Release ();
}

QIcon TorrentPlugin::GetIcon () const
{
    return QIcon (":/resources/images/torrent_bittorrent.png"); 
}

qint64 TorrentPlugin::GetDownloadSpeed () const
{
    OverallStats stats = Core::Instance ()->GetOverallStats ();
    return stats.DownloadRate_;
}

qint64 TorrentPlugin::GetUploadSpeed () const
{
    OverallStats stats = Core::Instance ()->GetOverallStats ();
    return stats.UploadRate_;
}

void TorrentPlugin::StartAll ()
{
    int numTorrents = Core::Instance ()->columnCount (QModelIndex ());
    for (int i = 0; i < numTorrents; ++i)
        Core::Instance ()->ResumeTorrent (i);
    setActionsEnabled ();
}

void TorrentPlugin::StopAll ()
{
    int numTorrents = Core::Instance ()->columnCount (QModelIndex ());
    for (int i = 0; i < numTorrents; ++i)
        Core::Instance ()->PauseTorrent (i);
}

bool TorrentPlugin::CouldDownload (const QString& string, LeechCraft::TaskParameters) const
{
    QFile file (string);
    if (!file.exists () || !file.open (QIODevice::ReadOnly))
        return false;

    return Core::Instance ()->IsValidTorrent (file.readAll ());
}

int TorrentPlugin::AddJob (const LeechCraft::DownloadParams& dp, LeechCraft::TaskParameters parameters)
{
    AddTorrentDialog_->Reinit ();
    AddTorrentDialog_->SetFilename (dp.Resource_);

	QString path;
	QStringList tags;
	QVector<bool> files;
	QString fname;
	if (parameters & LeechCraft::FromAutomatic)
	{
		fname = dp.Resource_;
		path = AddTorrentDialog_->GetDefaultSavePath ();
		tags = AddTorrentDialog_->GetDefaultTags ();
	}
	else
	{
		if (AddTorrentDialog_->exec () == QDialog::Rejected)
			return -1;

		fname = AddTorrentDialog_->GetFilename (),
		path = AddTorrentDialog_->GetSavePath ();
		files = AddTorrentDialog_->GetSelectedFiles ();
		tags = AddTorrentDialog_->GetTags ();
		if (AddTorrentDialog_->GetAddType () == Core::Started)
			parameters |= LeechCraft::Autostart;
		else
			parameters &= ~LeechCraft::Autostart;
	}
	int result = Core::Instance ()->AddFile (fname, path, tags, files, parameters);
    setActionsEnabled ();
	return result;
}

QAbstractItemModel* TorrentPlugin::GetRepresentation () const
{
    return FilterModel_.get ();
}

LeechCraft::HistoryModel* TorrentPlugin::GetHistory () const
{
	return Core::Instance ()->GetHistoryModel ();
}

QWidget* TorrentPlugin::GetControls () const
{
	return Toolbar_.get ();
}

QWidget* TorrentPlugin::GetAdditionalInfo () const
{
	return TabWidget_.get ();
}

void TorrentPlugin::ItemSelected (const QModelIndex& item)
{
	QModelIndex mapped = FilterModel_->mapToSource (item);
	Core::Instance ()->SetCurrentTorrent (mapped.row ());
	Ui_.TorrentTags_->setText (Core::Instance ()->GetTagsForIndex ().join (" "));
	if (mapped.isValid ())
	{
		TorrentSelectionChanged_ = true;
		updateTorrentStats ();
	}

	setActionsEnabled ();
}

void TorrentPlugin::ImportSettings (const QByteArray& settings)
{
	XmlSettingsDialog_->MergeXml (settings);
}

void TorrentPlugin::ImportData (const QByteArray& data)
{
	Core::Instance ()->ImportData (data);
}

QByteArray TorrentPlugin::ExportSettings () const
{
	return XmlSettingsDialog_->GetXml ().toUtf8 ();
}

QByteArray TorrentPlugin::ExportData () const
{
	return Core::Instance ()->ExportData ();
}

QStringList TorrentPlugin::GetTags (int torrent) const
{
	return Core::Instance ()->GetTagsForIndex (torrent);
}

QStringList TorrentPlugin::GetHistoryTags (int historyRow) const
{
	QModelIndex index = Core::Instance ()->
		GetHistoryModel ()->index (historyRow, 0);
	return Core::Instance ()->GetHistoryModel ()->
		data (index, HistoryModel::TagsRole).toStringList ();
}

void TorrentPlugin::SetTags (int torrent, const QStringList& tags)
{
	Core::Instance ()->UpdateTags (tags, torrent);
}

void TorrentPlugin::on_OpenTorrent__triggered ()
{
    AddTorrentDialog_->Reinit ();
    if (AddTorrentDialog_->exec () == QDialog::Rejected)
        return;

    QString filename = AddTorrentDialog_->GetFilename (),
            path = AddTorrentDialog_->GetSavePath ();
    QVector<bool> files = AddTorrentDialog_->GetSelectedFiles ();
    QStringList tags = AddTorrentDialog_->GetTags ();
	LeechCraft::TaskParameters tp;
	if (AddTorrentDialog_->GetAddType () == Core::Started)
		tp |= LeechCraft::Autostart;
    Core::Instance ()->AddFile (filename, path, tags, files, tp);
    setActionsEnabled ();
}

void TorrentPlugin::on_OpenMultipleTorrents__triggered ()
{
    AddMultipleTorrents dialog;
	std::auto_ptr<TagsCompleter> completer (new TagsCompleter (dialog.GetEdit (), this));
    completer->setModel (Core::Instance ()->GetTagsCompletionModel ());
	dialog.GetEdit ()->AddSelector ();

    if (dialog.exec () == QDialog::Rejected)
        return;

	LeechCraft::TaskParameters tp;
	if (dialog.GetAddType () == Core::Started)
		tp |= LeechCraft::Autostart;

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
        Core::Instance ()->AddFile (name, savePath, dialog.GetTags ());
    }
    setActionsEnabled ();
}

void TorrentPlugin::on_CreateTorrent__triggered ()
{
	std::auto_ptr<NewTorrentWizard> wizard (new NewTorrentWizard ());
    if (wizard->exec () == QDialog::Accepted)
        Core::Instance ()->MakeTorrent (wizard->GetParams ());
    setActionsEnabled ();
}

void TorrentPlugin::on_RemoveTorrent__triggered (int row)
{
    if (QMessageBox::question (0,
				tr ("Question"),
				tr ("Do you really want to delete the torrent?"),
				QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        return;

    Core::Instance ()->RemoveTorrent (row);
    TorrentSelectionChanged_ = true;
    setActionsEnabled ();
}

void TorrentPlugin::on_Resume__triggered (int row)
{
    Core::Instance ()->ResumeTorrent (row);
    setActionsEnabled ();
}

void TorrentPlugin::on_Stop__triggered (int row)
{
    Core::Instance ()->PauseTorrent (row);
    setActionsEnabled ();
}

void TorrentPlugin::on_MoveUp__triggered (const std::deque<int>& selections)
{
	Core::Instance ()->MoveUp (selections);
}

void TorrentPlugin::on_MoveDown__triggered (const std::deque<int>& selections)
{
	Core::Instance ()->MoveDown (selections);
}

void TorrentPlugin::on_MoveToTop__triggered (const std::deque<int>& selections)
{
	Core::Instance ()->MoveToTop (selections);
}

void TorrentPlugin::on_MoveToBottom__triggered (const std::deque<int>& selections)
{
	Core::Instance ()->MoveToBottom (selections);
}

void TorrentPlugin::on_ForceReannounce__triggered (int row)
{
    Core::Instance ()->ForceReannounce (row);
}

void TorrentPlugin::on_ForceRecheck__triggered (int row)
{
	Core::Instance ()->ForceRecheck (row);
}

void TorrentPlugin::on_Preferences__triggered ()
{
    XmlSettingsDialog_->show ();
    XmlSettingsDialog_->setWindowTitle ("BitTorrent" + tr (": Settings"));
}

void TorrentPlugin::on_ChangeTrackers__triggered ()
{
	QStringList trackers = Core::Instance ()->GetTrackers ();
	TrackersChanger changer;
	changer.SetTrackers (trackers);
	if (changer.exec () == QDialog::Accepted)
		Core::Instance ()->SetTrackers (changer.GetTrackers ());
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

void TorrentPlugin::on_TorrentDownloadRateController__valueChanged (int val)
{
    Core::Instance ()->SetTorrentDownloadRate (val);
}

void TorrentPlugin::on_TorrentUploadRateController__valueChanged (int val)
{
    Core::Instance ()->SetTorrentUploadRate (val);
}

void TorrentPlugin::on_TorrentDesiredRating__valueChanged (double val)
{
    Core::Instance ()->SetTorrentDesiredRating (val);
}

void TorrentPlugin::on_TorrentManaged__stateChanged (int state)
{
	Core::Instance ()->SetTorrentManaged (state == Qt::Checked);
}

void TorrentPlugin::on_TorrentSequentialDownload__stateChanged (int state)
{
	Core::Instance ()->SetTorrentSequentialDownload (state == Qt::Checked);
}

void TorrentPlugin::on_CaseSensitiveSearch__stateChanged (int state)
{
    FilterModel_->setFilterCaseSensitivity (state ? Qt::CaseSensitive : Qt::CaseInsensitive);
}

void TorrentPlugin::on_DownloadingTorrents__valueChanged (int newValue)
{
    Core::Instance ()->SetMaxDownloadingTorrents (newValue);
}

void TorrentPlugin::on_UploadingTorrents__valueChanged (int newValue)
{
    Core::Instance ()->SetMaxUploadingTorrents (newValue);
}

void TorrentPlugin::on_TorrentTags__editingFinished ()
{
    Core::Instance ()->UpdateTags (Ui_.TorrentTags_->text ().split (' ', QString::SkipEmptyParts));
}

void TorrentPlugin::on_MoveFiles__triggered (int)
{
    QString oldDir = Core::Instance ()->GetTorrentDirectory ();
    MoveTorrentFiles mtf (oldDir);
    if (mtf.exec () == QDialog::Rejected)
        return;
    QString newDir = mtf.GetNewLocation ();
    if (oldDir == newDir)
        return;

    if (Core::Instance ()->MoveTorrentFiles (newDir))
        QMessageBox::information (0, tr ("Information"),
                tr ("Started moving torrent's files from %1 to %2").
                arg (oldDir).
                arg (newDir));
    else
        QMessageBox::warning (0, tr ("Warning"),
                tr ("Failed to move torrent's files from %1 to %2").
                arg (oldDir).
                arg (newDir));
}

void TorrentPlugin::setActionsEnabled ()
{
	int torrent = Core::Instance ()->GetCurrentTorrent ();
	bool isValid = false;
	if (torrent != -1)
		isValid = Core::Instance ()->CheckValidity (torrent);
    RemoveTorrent_->setEnabled (isValid);
    Stop_->setEnabled (isValid);
    Resume_->setEnabled (isValid);
    ForceReannounce_->setEnabled (isValid);
}

void TorrentPlugin::showError (QString e)
{
    qWarning () << e;
    QMessageBox::warning (0, tr ("Error!"), e);
}

void TorrentPlugin::updateTorrentStats ()
{
	switch (TabWidget_->currentIndex ())
	{
		case 0:
			UpdateDashboard ();
			updateOverallStats ();
			break;
		case 1:
			UpdateTorrentControl ();
			break;
		case 2:
			UpdateTorrentPage ();
			break;
		case 3:
			UpdateFilesPage ();
			break;
		case 4:
			UpdatePeersPage ();
			break;
		case 5:
			UpdatePiecesPage ();
			break;
	}
	TorrentSelectionChanged_ = false;
}

void TorrentPlugin::updateOverallStats ()
{
    OverallStats stats = Core::Instance ()->GetOverallStats ();
    Ui_.LabelTotalDownloadRate_->setText (Proxy::Instance ()->MakePrettySize (static_cast<int> (stats.DownloadRate_)) + tr ("/s"));
    Ui_.LabelTotalUploadRate_->setText (Proxy::Instance ()->MakePrettySize (static_cast<int> (stats.UploadRate_)) + tr ("/s"));
    Ui_.LabelTotalDownloaded_->setText (Proxy::Instance ()->MakePrettySize (stats.SessionDownload_));
    Ui_.LabelTotalUploaded_->setText (Proxy::Instance ()->MakePrettySize (stats.SessionUpload_));
    Ui_.LabelTotalConnections_->setText (QString::number (stats.NumConnections_));
    Ui_.LabelUploadConnections_->setText (QString::number (stats.NumUploads_));
    Ui_.LabelTotalPeers_->setText (QString::number (stats.NumPeers_));
    Ui_.LabelTotalDHTNodes_->setText (QString ("(") +
			QString::number (stats.NumGlobalDHTNodes_) +
			QString (") ") +
			QString::number (stats.NumDHTNodes_));
    Ui_.LabelDHTTorrents_->setText (QString::number (stats.NumDHTTorrents_));
    Ui_.LabelListenPort_->setText (QString::number (stats.ListenPort_));
    Ui_.LabelSessionRating_->setText (QString::number (stats.SessionUpload_ / static_cast<double> (stats.SessionDownload_), 'g', 4));
	Ui_.LabelTotalFailedData_->setText (Proxy::Instance ()->MakePrettySize (stats.TotalFailedData_));
	Ui_.LabelTotalRedundantData_->setText (Proxy::Instance ()->MakePrettySize (stats.TotalRedundantData_));
	Ui_.LabelExternalAddress_->setText (Core::Instance ()->GetExternalAddress ());

	libtorrent::cache_status cs = Core::Instance ()->GetCacheStats ();
	Ui_.BlocksWritten_->setText (QString::number (cs.blocks_written));
	Ui_.Writes_->setText (QString::number (cs.writes));
	Ui_.WriteHitRatio_->setText (QString::number (static_cast<double> (cs.blocks_written - cs.writes ) /
				static_cast<double> (cs.blocks_written)));
	Ui_.CacheSize_->setText (QString::number (cs.cache_size));
	Ui_.TotalBlocksRead_->setText (QString::number (cs.blocks_read));
	Ui_.CachedBlockReads_->setText (QString::number (cs.blocks_read_hit));
	Ui_.ReadHitRatio_->setText (QString::number (static_cast<double> (cs.blocks_read_hit) /
				static_cast<double> (cs.blocks_read)));
	Ui_.ReadCacheSize_->setText (QString::number (cs.read_cache_size));
}

void TorrentPlugin::doLogMessage (const QString& msg)
{
    Ui_.LogShower_->append (msg.trimmed ());
}

void TorrentPlugin::addToHistory (const QString& name,
		const QString& where, quint64 size, const QDateTime& when,
		const QStringList& tags)
{
	HistoryModel::HistoryItem item = { name, where, size, when, tags };
	Core::Instance ()->GetHistoryModel ()->AddItem (item);
}

void TorrentPlugin::UpdateDashboard ()
{
    Ui_.OverallDownloadRateController_->setValue (Core::Instance ()->GetOverallDownloadRate ());
    Ui_.OverallUploadRateController_->setValue (Core::Instance ()->GetOverallUploadRate ());
    Ui_.DownloadingTorrents_->setValue (Core::Instance ()->GetMaxDownloadingTorrents ());
    Ui_.UploadingTorrents_->setValue (Core::Instance ()->GetMaxUploadingTorrents ());
    Ui_.DesiredRating_->setValue (Core::Instance ()->GetDesiredRating ());
}

void TorrentPlugin::UpdateTorrentControl ()
{
	Ui_.TorrentDownloadRateController_->setValue (Core::Instance ()->GetTorrentDownloadRate ());
	Ui_.TorrentUploadRateController_->setValue (Core::Instance ()->GetTorrentUploadRate ());
	Ui_.TorrentDesiredRating_->setValue (Core::Instance ()->GetTorrentDesiredRating ());
	Ui_.TorrentManaged_->setCheckState (Core::Instance ()->IsTorrentManaged () ? Qt::Checked : Qt::Unchecked);
	Ui_.TorrentSequentialDownload_->setCheckState (Core::Instance ()->IsTorrentSequentialDownload () ? Qt::Checked : Qt::Unchecked);

	TorrentInfo i = Core::Instance ()->GetTorrentStats ();
	Ui_.LabelDownloadRate_->setText (Proxy::Instance ()->MakePrettySize (i.DownloadRate_) + tr ("/s"));
	Ui_.LabelUploadRate_->setText (Proxy::Instance ()->MakePrettySize (i.UploadRate_) + tr ("/s"));
	Ui_.LabelNextAnnounce_->setText (i.NextAnnounce_.toString ());
	Ui_.LabelProgress_->setText (QString::number (i.Progress_ * 100) + "%");
	Ui_.LabelState_->setText (i.State_);
	Ui_.LabelDownloaded_->setText (Proxy::Instance ()->MakePrettySize (i.Downloaded_));
	Ui_.LabelUploaded_->setText (Proxy::Instance ()->MakePrettySize (i.Uploaded_));
	Ui_.LabelWantedDownloaded_->setText (Proxy::Instance ()->MakePrettySize (i.WantedDownload_));
	Ui_.LabelUploadedTotal_->setText (Proxy::Instance ()->MakePrettySize (i.UploadedTotal_));
	Ui_.LabelTorrentOverallRating_->setText (QString::number (i.UploadedTotal_ / static_cast<double> (i.Downloaded_), 'g', 4));
	Ui_.LabelActiveTime_->setText (Proxy::Instance ()->MakeTimeFromLong (i.ActiveTime_));
	Ui_.LabelSeedingTime_->setText (Proxy::Instance ()->MakeTimeFromLong (i.SeedingTime_));
	Ui_.LabelSeedRank_->setText (QString::number (i.SeedRank_));
	Ui_.LabelLastScrape_->setText (Proxy::Instance ()->MakeTimeFromLong (i.LastScrape_));
	Ui_.LabelTorrentRating_->setText (QString::number (i.Uploaded_ / static_cast<double> (i.Downloaded_), 'g', 4));
	Ui_.PiecesWidget_->setPieceMap (i.Pieces_);
}

void TorrentPlugin::UpdateTorrentPage ()
{
	TorrentInfo i = Core::Instance ()->GetTorrentStats ();
	Ui_.LabelTracker_->setText (i.Tracker_);
	Ui_.LabelDestination_->setText (i.Destination_);
	Ui_.LabelDHTNodesCount_->setText (QString::number (i.DHTNodesCount_));
	Ui_.LabelTotalSize_->setText (Proxy::Instance ()->MakePrettySize (i.TotalSize_));
	Ui_.LabelFailed_->setText (Proxy::Instance ()->MakePrettySize (i.FailedSize_));
	Ui_.LabelConnectedPeers_->setText (QString::number (i.ConnectedPeers_) + "/" +
			QString::number (i.ConnectedSeeds_));
	Ui_.LabelAnnounceInterval_->setText (i.AnnounceInterval_.toString ());
	Ui_.LabelTotalPieces_->setText (QString::number (i.TotalPieces_));
	Ui_.LabelDownloadedPieces_->setText (QString::number (i.DownloadedPieces_));
	Ui_.LabelPieceSize_->setText (Proxy::Instance ()->MakePrettySize (i.PieceSize_) + "; " +
			Proxy::Instance ()->MakePrettySize (i.BlockSize_));
	Ui_.LabelDistributedCopies_->setText (i.DistributedCopies_ == -1 ?
			tr ("Not tracking") : QString::number (i.DistributedCopies_));
	Ui_.LabelRedundantData_->setText (Proxy::Instance ()->MakePrettySize (i.RedundantBytes_));
	Ui_.LabelPeersInList_->setText (QString::number (i.PeersInList_) + "/" +
			QString::number (i.SeedsInList_));
	Ui_.LabelPeersInSwarm_->setText ((i.PeersInSwarm_ == -1 ?
				tr ("Unknown") : QString::number (i.PeersInSwarm_)) + "/" +
			(i.SeedsInSwarm_ == -1 ?
			  tr ("Unknown") : QString::number (i.SeedsInSwarm_)));
	Ui_.LabelConnectCandidates_->setText (QString::number (i.ConnectCandidates_));
	Ui_.LabelUpBandwidthQueue_->setText (QString::number (i.UpBandwidthQueue_) + "/" + 
			QString::number (i.DownBandwidthQueue_));
	Ui_.LabelProtocolOverhead_->setText (Proxy::Instance ()->MakePrettySize (i.DownloadOverhead_) + "/" +
			Proxy::Instance ()->MakePrettySize (i.UploadOverhead_));
}

void TorrentPlugin::UpdateFilesPage ()
{
    if (TorrentSelectionChanged_)
    {
        Core::Instance ()->ResetFiles ();
        Ui_.FilesView_->expandAll ();
    }
    else
	{
        Core::Instance ()->UpdateFiles ();
		Ui_.FilesView_->expandAll ();
	}
}

void TorrentPlugin::UpdatePeersPage ()
{
	Core::Instance ()->UpdatePeers ();
}

void TorrentPlugin::UpdatePiecesPage ()
{
	Core::Instance ()->UpdatePieces ();
}

void TorrentPlugin::SetupTabWidget ()
{
	TabWidget_.reset (new QTabWidget ());
	Ui_.setupUi (TabWidget_.get ());
	connect (Ui_.OverallDownloadRateController_,
			SIGNAL (valueChanged (int)),
			this,
			SLOT (on_OverallDownloadRateController__valueChanged (int)));
	connect (Ui_.OverallUploadRateController_,
			SIGNAL (valueChanged (int)),
			this,
			SLOT (on_OverallUploadRateController__valueChanged (int)));
	connect (Ui_.DesiredRating_,
			SIGNAL (valueChanged (double)),
			this,
			SLOT (on_DesiredRating__valueChanged (double)));
	connect (Ui_.TorrentDownloadRateController_,
			SIGNAL (valueChanged (int)),
			this,
			SLOT (on_TorrentDownloadRateController__valueChanged (int)));
	connect (Ui_.TorrentUploadRateController_,
			SIGNAL (valueChanged (int)),
			this,
			SLOT (on_TorrentUploadRateController__valueChanged (int)));
	connect (Ui_.TorrentDesiredRating_,
			SIGNAL (valueChanged (double)),
			this,
			SLOT (on_TorrentDesiredRating__valueChanged (double)));
	connect (Ui_.TorrentManaged_,
			SIGNAL (stateChanged (int)),
			this,
			SLOT (on_TorrentManaged__stateChanged (int)));
	connect (Ui_.TorrentSequentialDownload_,
			SIGNAL (stateChanged (int)),
			this,
			SLOT (on_TorrentSequentialDownload__stateChanged (int)));
	connect (Ui_.DownloadingTorrents_,
			SIGNAL (valueChanged (int)),
			this,
			SLOT (on_DownloadingTorrents__valueChanged (int)));
	connect (Ui_.UploadingTorrents_,
			SIGNAL (valueChanged (int)),
			this,
			SLOT (on_UploadingTorrents__valueChanged (int)));
	connect (Ui_.TorrentTags_,
			SIGNAL (editingFinished ()),
			this,
			SLOT (on_TorrentTags__editingFinished ()));
}

void TorrentPlugin::SetupCore ()
{
	SetupTabWidget ();
    TorrentSelectionChanged_ = true;
    LastPeersUpdate_.reset (new QTime);
    LastPeersUpdate_->start ();
    XmlSettingsDialog_.reset (new XmlSettingsDialog ());
    XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (), ":/torrentsettings.xml");
    AddTorrentDialog_.reset (new AddTorrent ());
	connect (Core::Instance (),
			SIGNAL (error (QString)),
			this,
			SLOT (showError (QString)));
	connect (Core::Instance (),
			SIGNAL (logMessage (const QString&)),
			this,
			SLOT (doLogMessage (const QString&)));
	connect (Core::Instance (),
			SIGNAL (torrentFinished (const QString&)),
			this,
			SIGNAL (downloadFinished (const QString&)));
	connect (Core::Instance (),
			SIGNAL (fileFinished (const QString&)),
			this,
			SIGNAL (fileDownloaded (const QString&)));
	connect (Core::Instance (),
			SIGNAL (dataChanged (const QModelIndex&,
					const QModelIndex&)),
			this,
			SLOT (updateTorrentStats ()));
	connect (Core::Instance (),
			SIGNAL (addToHistory (const QString&, const QString&,
					quint64, const QDateTime&, const QStringList&)),
			this,
			SLOT (addToHistory (const QString&, const QString&,
					quint64, const QDateTime&, const QStringList&)));
	connect (TabWidget_.get (),
			SIGNAL (currentChanged (int)),
			this,
			SLOT (updateTorrentStats ()));
	connect (Core::Instance (),
			SIGNAL (taskFinished (int)),
			this,
			SIGNAL (jobFinished (int)));
	connect (Core::Instance (),
			SIGNAL (taskRemoved (int)),
			this,
			SIGNAL (jobRemoved (int)));

    Core::Instance ()->DoDelayedInit ();
}

void TorrentPlugin::SetupTorrentView ()
{
    FilterModel_.reset (new RepresentationModel);
    FilterModel_->setSourceModel (Core::Instance ());
	connect (Core::Instance (),
			SIGNAL (dataChanged (const QModelIndex&,
					const QModelIndex&)),
			FilterModel_.get (),
			SLOT (invalidate ()));
}

void TorrentPlugin::SetupStuff ()
{
    TagsChangeCompleter_.reset (new TagsCompleter (Ui_.TorrentTags_));
    TagsAddDiaCompleter_.reset (new TagsCompleter (AddTorrentDialog_->GetEdit ()));
    TagsChangeCompleter_->setModel (Core::Instance ()->GetTagsCompletionModel ());
    TagsAddDiaCompleter_->setModel (Core::Instance ()->GetTagsCompletionModel ());
	AddTorrentDialog_->GetEdit ()->AddSelector ();

    Ui_.PiecesView_->setModel (Core::Instance ()->GetPiecesModel ());

    Ui_.FilesView_->setModel (Core::Instance ()->GetTorrentFilesModel ());
    Ui_.FilesView_->setItemDelegate (new FilesViewDelegate (this));

    QSortFilterProxyModel *peersSorter = new QSortFilterProxyModel (this);
    peersSorter->setDynamicSortFilter (true);
    peersSorter->setSourceModel (Core::Instance ()->GetPeersModel ());
    peersSorter->setSortRole (PeersModel::SortRole);
    Ui_.PeersView_->setModel (peersSorter);

	UpdateDashboard ();
    
    OverallStatsUpdateTimer_.reset (new QTimer (this));
    connect (OverallStatsUpdateTimer_.get (),
			SIGNAL (timeout ()),
			this,
			SLOT (updateTorrentStats ()));
    connect (OverallStatsUpdateTimer_.get (),
			SIGNAL (timeout ()),
			FilterModel_.get (),
			SLOT (invalidate ()));
    OverallStatsUpdateTimer_->start (500);
}

void TorrentPlugin::SetupActions ()
{
	Toolbar_.reset (new QToolBar ());
	Toolbar_->setMovable (false);
	Toolbar_->setFloatable (false);

	OpenTorrent_.reset (new QAction (tr ("Open torrent..."),
				Toolbar_.get ()));
	OpenTorrent_->setShortcut (Qt::Key_Insert);
	OpenTorrent_->setProperty ("ActionIcon", "torrent_addjob");
	connect (OpenTorrent_.get (),
			SIGNAL (triggered ()),
			this,
			SLOT (on_OpenTorrent__triggered ()));

	Preferences_.reset (new QAction (tr ("Settings..."),
				Toolbar_.get ()));
	Preferences_->setShortcut (tr ("Ctrl+Shift+P"));
	Preferences_->setProperty ("ActionIcon", "torrent_preferences");
	connect (Preferences_.get (),
			SIGNAL (triggered ()),
			this,
			SLOT (on_Preferences__triggered ()));

	ChangeTrackers_.reset (new QAction (tr ("Change trackers..."),
				Toolbar_.get ()));
	ChangeTrackers_->setShortcut (tr ("C"));
	ChangeTrackers_->setProperty ("ActionIcon", "torrent_changetrackers");
	connect (ChangeTrackers_.get (),
			SIGNAL (triggered ()),
			this,
			SLOT (on_ChangeTrackers__triggered ()));

	CreateTorrent_.reset (new QAction (tr ("Create torrent..."),
				Toolbar_.get ()));
	CreateTorrent_->setShortcut (tr ("N"));
	CreateTorrent_->setProperty ("ActionIcon", "torrent_create");
	connect (CreateTorrent_.get (),
			SIGNAL (triggered ()),
			this,
			SLOT (on_CreateTorrent__triggered ()));

	OpenMultipleTorrents_.reset (new QAction (tr ("Open multiple torrents..."),
			Toolbar_.get ()));
	OpenMultipleTorrents_->setProperty ("ActionIcon", "torrent_addmulti");
	connect (OpenMultipleTorrents_.get (),
			SIGNAL (triggered ()),
			this,
			SLOT (on_OpenMultipleTorrents__triggered ()));

	RemoveTorrent_.reset (new QAction (tr ("Remove"),
				Toolbar_.get ()));
	RemoveTorrent_->setShortcut (tr ("Del"));
	RemoveTorrent_->setProperty ("Slot", "on_RemoveTorrent__triggered");
	RemoveTorrent_->setProperty ("Object", QVariant::fromValue<QObject*> (this));
	RemoveTorrent_->setProperty ("ActionIcon", "torrent_deletejob");
	
	Resume_.reset (new QAction (tr ("Resume"),
				Toolbar_.get ()));
	Resume_->setShortcut (tr ("R"));
	Resume_->setProperty ("Slot", "on_Resume__triggered");
	Resume_->setProperty ("Object", QVariant::fromValue<QObject*> (this));
	Resume_->setProperty ("ActionIcon", "torrent_startjob");

	Stop_.reset (new QAction (tr ("Pause"),
				Toolbar_.get ()));
	Stop_->setShortcut (tr ("S"));
	Stop_->setProperty ("Slot", "on_Stop__triggered");
	Stop_->setProperty ("Object", QVariant::fromValue<QObject*> (this));
	Stop_->setProperty ("ActionIcon", "torrent_stopjob");

	MoveUp_.reset (new QAction (tr ("Move up"),
				Toolbar_.get ()));
	MoveUp_->setShortcut (Qt::CTRL + Qt::Key_Up);
	MoveUp_->setProperty ("Slot", "on_MoveUp__triggered");
	MoveUp_->setProperty ("WholeSelection", true);
	MoveUp_->setProperty ("Object", QVariant::fromValue<QObject*> (this));
	MoveUp_->setProperty ("ActionIcon", "torrent_moveup");

	MoveDown_.reset (new QAction (tr ("Move down"),
				Toolbar_.get ()));
	MoveDown_->setShortcut (Qt::CTRL + Qt::Key_Down);
	MoveDown_->setProperty ("Slot", "on_MoveDown__triggered");
	MoveDown_->setProperty ("WholeSelection", true);
	MoveDown_->setProperty ("Object", QVariant::fromValue<QObject*> (this));
	MoveDown_->setProperty ("ActionIcon", "torrent_movedown");

	MoveToTop_.reset (new QAction (tr ("Move to top"),
				Toolbar_.get ()));
	MoveToTop_->setShortcut (Qt::CTRL + Qt::SHIFT + Qt::Key_Up);
	MoveToTop_->setProperty ("Slot", "on_MoveToTop__triggered");
	MoveToTop_->setProperty ("WholeSelection", true);
	MoveToTop_->setProperty ("Object", QVariant::fromValue<QObject*> (this));
	MoveToTop_->setProperty ("ActionIcon", "torrent_movetop");

	MoveToBottom_.reset (new QAction (tr ("Move to bottom"),
				Toolbar_.get ()));
	MoveToBottom_->setShortcut (Qt::CTRL + Qt::SHIFT + Qt::Key_Down);
	MoveToBottom_->setProperty ("Slot", "on_MoveToBottom__triggered");
	MoveToBottom_->setProperty ("WholeSelection", true);
	MoveToBottom_->setProperty ("Object", QVariant::fromValue<QObject*> (this));
	MoveToBottom_->setProperty ("ActionIcon", "torrent_movebottom");

	ForceReannounce_.reset (new QAction (tr ("Reannounce"),
				Toolbar_.get ()));
	ForceReannounce_->setShortcut (tr ("F"));
	ForceReannounce_->setProperty ("Slot", "on_ForceReannounce__triggered");
	ForceReannounce_->setProperty ("Object", QVariant::fromValue<QObject*> (this));
	ForceReannounce_->setProperty ("ActionIcon", "torrent_forcereannounce");
	
	ForceRecheck_.reset (new QAction (tr ("Recheck"),
			Toolbar_.get ()));
	ForceRecheck_->setProperty ("Slot", "on_ForceRecheck__triggered");
	ForceRecheck_->setProperty ("Object", QVariant::fromValue<QObject*> (this));
	ForceRecheck_->setProperty ("ActionIcon", "torrent_forcerecheck");

	MoveFiles_.reset (new QAction (tr ("Move files..."),
				Toolbar_.get ()));
	MoveFiles_->setShortcut (tr ("M"));
	MoveFiles_->setProperty ("Slot", "on_MoveFiles__triggered");
	MoveFiles_->setProperty ("Object", QVariant::fromValue<QObject*> (this));
	MoveFiles_->setProperty ("ActionIcon", "torrent_movefiles");

	Toolbar_->addAction (CreateTorrent_.get ());
	Toolbar_->addSeparator ();
	Toolbar_->addAction (OpenTorrent_.get ());
	Toolbar_->addAction (RemoveTorrent_.get ());
	Toolbar_->addAction (OpenMultipleTorrents_.get ());
	Toolbar_->addSeparator ();
	Toolbar_->addAction (Resume_.get ());
	Toolbar_->addAction (Stop_.get ());
	Toolbar_->addSeparator ();
	Toolbar_->addAction (MoveUp_.get ());
	Toolbar_->addAction (MoveDown_.get ());
	Toolbar_->addAction (MoveToTop_.get ());
	Toolbar_->addAction (MoveToBottom_.get ());
	Toolbar_->addSeparator ();
	Toolbar_->addAction (ForceReannounce_.get ());
	Toolbar_->addAction (ForceRecheck_.get ());
	Toolbar_->addAction (MoveFiles_.get ());
	Toolbar_->addAction (ChangeTrackers_.get ());
	Toolbar_->addSeparator ();
	Toolbar_->addAction (Preferences_.get ());
}

Q_EXPORT_PLUGIN2 (leechcraft_torrent, TorrentPlugin);


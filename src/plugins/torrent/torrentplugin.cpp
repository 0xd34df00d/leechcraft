#include "torrentplugin.h"
#include <QMessageBox>
#include <QUrl>
#include <QTemporaryFile>
#include <QtDebug>
#include <QDir>
#include <QMenu>
#include <QAction>
#include <QTabWidget>
#include <QTranslator>
#include <QTextCodec>
#include <QTimer>
#include <QToolBar>
#include <QSortFilterProxyModel>
#include <QHeaderView>
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
#include "exportdialog.h"

#ifdef AddJob
#undef AddJob
#endif

using LeechCraft::Util::Proxy;
using LeechCraft::Util::TagsCompleter;
using LeechCraft::Util::TagsLineEdit;
using LeechCraft::ActionInfo;

void TorrentPlugin::Init (ICoreProxy_ptr)
{
	Translator_.reset (LeechCraft::Util::InstallTranslator ("torrent"));
    SetupCore ();
    SetupTorrentView ();
    SetupStuff ();

    setActionsEnabled ();
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
	XmlSettingsDialog_.reset ();
}

QIcon TorrentPlugin::GetIcon () const
{
    return QIcon (":/resources/images/torrent_bittorrent.png"); 
}

qint64 TorrentPlugin::GetDownloadSpeed () const
{
	return Core::Instance ()->GetOverallStats ().download_rate;
}

qint64 TorrentPlugin::GetUploadSpeed () const
{
	return Core::Instance ()->GetOverallStats ().upload_rate;
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

bool TorrentPlugin::CouldDownload (const LeechCraft::DownloadEntity& e) const
{
	return Core::Instance ()->CouldDownload (e);
}

int TorrentPlugin::AddJob (LeechCraft::DownloadEntity e)
{
	QString resource = QTextCodec::codecForName ("UTF-8")->
		toUnicode (e.Entity_);

	if (resource.startsWith ("magnet:"))
	{
		QStringList tags = XmlSettingsManager::Instance ()->
			property ("AutomaticTags").toString ()
			.split (' ', QString::SkipEmptyParts);

		QUrl url (resource);
		QList<QPair<QString, QString> > queryItems = url.queryItems ();
		for (QList<QPair<QString, QString> >::const_iterator i = queryItems.begin (),
				end = queryItems.end (); i != end; ++i)
			if (i->first == "kt")
				tags += i->second.split ('+', QString::SkipEmptyParts);

		return Core::Instance ()->AddMagnet (resource,
				e.Location_,
				tags,
				e.Parameters_);
	}
	else if (resource.startsWith ("file://"))
		resource = QUrl (resource).toLocalFile ();

	QString suggestedFname = resource;
	QFile file (suggestedFname);
    if ((!file.exists () ||
			!file.open (QIODevice::ReadOnly)) &&
			Core::Instance ()->IsValidTorrent (e.Entity_))
	{
		QTemporaryFile file ("lctemporarybittorrentfile.XXXXXX");
		if (!file.open  ())
			return -1;
		file.write (e.Entity_);
		suggestedFname = file.fileName ().toUtf8 ();
		file.setAutoRemove (false);
	}

    AddTorrentDialog_->Reinit ();
    AddTorrentDialog_->SetFilename (suggestedFname);
	if (!e.Location_.isEmpty ())
		AddTorrentDialog_->SetSavePath (e.Location_);

	QString path;
	QStringList tags;
	QVector<bool> files;
	QString fname;
	if (e.Parameters_ & LeechCraft::FromUserInitiated)
	{
		if (AddTorrentDialog_->exec () == QDialog::Rejected)
			return -1;

		fname = AddTorrentDialog_->GetFilename (),
		path = AddTorrentDialog_->GetSavePath ();
		files = AddTorrentDialog_->GetSelectedFiles ();
		tags = AddTorrentDialog_->GetTags ();
		if (AddTorrentDialog_->GetAddType () == Core::Started)
			e.Parameters_ &= ~LeechCraft::NoAutostart;
		else
			e.Parameters_ |= LeechCraft::NoAutostart;
	}
	else
	{
		fname = suggestedFname;
		path = e.Location_;
		tags = XmlSettingsManager::Instance ()->
			property ("AutomaticTags").toString ()
			.split (' ', QString::SkipEmptyParts);
	}
	int result = Core::Instance ()->AddFile (fname,
			path,
			tags,
			files,
			e.Parameters_);
    setActionsEnabled ();
	file.remove ();
	return result;
}

QAbstractItemModel* TorrentPlugin::GetRepresentation () const
{
    return FilterModel_.get ();
}

void TorrentPlugin::ItemSelected (const QModelIndex& item)
{
	QModelIndex mapped = item.isValid () ?
		FilterModel_->mapToSource (item) : QModelIndex ();
	Current_ = mapped;
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
}

QByteArray TorrentPlugin::ExportSettings () const
{
	return XmlSettingsDialog_->GetXml ().toUtf8 ();
}

QByteArray TorrentPlugin::ExportData () const
{
	return QByteArray ();
}

void TorrentPlugin::SetTags (int torrent, const QStringList& tags)
{
	Core::Instance ()->UpdateTags (tags, torrent);
}

boost::shared_ptr<LeechCraft::Util::XmlSettingsDialog> TorrentPlugin::GetSettingsDialog () const
{
	return XmlSettingsDialog_;
}

#define _LC_MERGE(a) EA##a

#define _LC_SINGLE(a) \
	case _LC_MERGE(a): \
		a->setShortcut (shortcut); \
		break;

#define _LC_TRAVERSER(z,i,array) \
	_LC_SINGLE (BOOST_PP_SEQ_ELEM(i, array))

#define _LC_EXPANDER(Names) \
	switch (name) \
	{ \
		BOOST_PP_REPEAT (BOOST_PP_SEQ_SIZE (Names), _LC_TRAVERSER, Names) \
	}
void TorrentPlugin::SetShortcut (int name,
		const QKeySequence& shortcut)
{
	_LC_EXPANDER ((OpenTorrent_)
			(ChangeTrackers_)
			(CreateTorrent_)
			(OpenMultipleTorrents_)
			(RemoveTorrent_)
			(Resume_)
			(Stop_)
			(MoveUp_)
			(MoveDown_)
			(MoveToTop_)
			(MoveToBottom_)
			(ForceReannounce_)
			(ForceRecheck_)
			(MoveFiles_)
			(Import_)
			(Export_));
}

#define _L(a) result [EA##a] = ActionInfo (a->text (), \
		a->shortcut (), a->icon ())
QMap<int, ActionInfo> TorrentPlugin::GetActionInfo () const
{
	QMap<int, ActionInfo> result;
	_L (OpenTorrent_);
	_L (ChangeTrackers_);
	_L (CreateTorrent_);
	_L (OpenMultipleTorrents_);
	_L (RemoveTorrent_);
	_L (Resume_);
	_L (Stop_);
	_L (MoveUp_);
	_L (MoveDown_);
	_L (MoveToTop_);
	_L (MoveToBottom_);
	_L (ForceReannounce_);
	_L (ForceRecheck_);
	_L (MoveFiles_);
	_L (Import_);
	_L (Export_);
	return result;
}
#undef _L

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
	if (AddTorrentDialog_->GetAddType () != Core::Started)
		tp |= LeechCraft::NoAutostart;
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
	if (dialog.GetAddType () != Core::Started)
		tp |= LeechCraft::NoAutostart;

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

void TorrentPlugin::on_TorrentSuperSeeding__stateChanged (int state)
{
	Core::Instance ()->SetTorrentSuperSeeding (state == Qt::Checked);
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

    if (!Core::Instance ()->MoveTorrentFiles (newDir))
        QMessageBox::warning (0,
				tr ("Warning"),
                tr ("Failed to move torrent's files from %1 to %2")
				.arg (oldDir)
				.arg (newDir));
}

void TorrentPlugin::on_Import__triggered ()
{
}

void TorrentPlugin::on_Export__triggered ()
{
	ExportDialog dia;
	if (dia.exec () == QDialog::Rejected)
		return;

	bool settings = dia.GetSettings ();
	bool active = dia.GetActive ();
	QString where = dia.GetLocation ();

	Core::Instance ()->Export (where, settings, active);
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
	if (!Current_.isValid ())
		return;

	switch (TabWidget_->currentIndex ())
	{
		case 0:
			UpdateDashboard ();
			UpdateOverallStats ();
			break;
		case 1:
			UpdateTorrentControl ();
			break;
		case 2:
			UpdateFilesPage ();
			break;
		case 3:
			UpdatePeersPage ();
			break;
		case 4:
			UpdatePiecesPage ();
			break;
	}
	TorrentSelectionChanged_ = false;
}

namespace
{
	struct Percenter
	{
		template<typename T>
		QString operator() (const T& t1, const T& t2) const
		{
			if (t2)
			{
				return QString (" (") +
					QString::number (static_cast<float> (t1) * 100 /
							static_cast<float> (t2), 'f', 1) +
					"%)";
			}
			else
				return QString ("");
		}
	};

	template<int i>
	struct Constructor
	{
		template<typename T>
		QString operator() (const T& t1, const T& t2) const
		{
			Percenter p;
			return Proxy::Instance ()->MakePrettySize (t1) +
				(i ? QObject::tr ("/s") : "") + p (t1, t2);
		}
	};
};

void TorrentPlugin::UpdateOverallStats ()
{
	libtorrent::session_status stats = Core::Instance ()->GetOverallStats ();

	Ui_.LabelTotalDownloadRate_->
		setText (Proxy::Instance ()->
				MakePrettySize (stats.download_rate) + tr ("/s"));
	Ui_.LabelTotalUploadRate_->
		setText (Proxy::Instance ()->
				MakePrettySize (stats.upload_rate) + tr ("/s"));

	Constructor<1> speed;

	Ui_.LabelOverheadDownloadRate_->
		setText (speed (stats.ip_overhead_download_rate, stats.download_rate));
	Ui_.LabelOverheadUploadRate_->
		setText (speed (stats.ip_overhead_upload_rate, stats.upload_rate));
	Ui_.LabelDHTDownloadRate_->
		setText (speed (stats.dht_download_rate, stats.download_rate));
	Ui_.LabelDHTUploadRate_->
		setText (speed (stats.dht_upload_rate, stats.upload_rate));
	Ui_.LabelTrackerDownloadRate_->
		setText (speed (stats.tracker_download_rate, stats.download_rate));
	Ui_.LabelTrackerUploadRate_->
		setText (speed (stats.tracker_upload_rate, stats.upload_rate));

	Ui_.LabelTotalDownloaded_->
		setText (Proxy::Instance ()->
				MakePrettySize (stats.total_download));
	Ui_.LabelTotalUploaded_->
		setText (Proxy::Instance ()->
				MakePrettySize (stats.total_upload));

	Constructor<0> simple;
	Ui_.LabelOverheadDownloaded_->
		setText (simple (stats.total_ip_overhead_download, stats.total_download));
	Ui_.LabelOverheadUploaded_->
		setText (simple (stats.total_ip_overhead_upload, stats.total_upload));
	Ui_.LabelDHTDownloaded_->
		setText (simple (stats.total_dht_download, stats.total_download));
	Ui_.LabelDHTUploaded_->
		setText (simple (stats.total_dht_upload, stats.total_upload));
	Ui_.LabelTrackerDownloaded_->
		setText (simple (stats.total_tracker_download, stats.total_download));
	Ui_.LabelTrackerUploaded_->
		setText (simple (stats.total_tracker_upload, stats.total_upload));

	Ui_.LabelTotalPeers_->setText (QString::number (stats.num_peers));
	Ui_.LabelTotalDHTNodes_->setText (QString ("(") +
			QString::number (stats.dht_global_nodes) +
			QString (") ") +
			QString::number (stats.dht_nodes));
	Ui_.LabelDHTTorrents_->
		setText (QString::number (stats.dht_torrents));
	Ui_.LabelListenPort_->
		setText (QString::number (Core::Instance ()->GetListenPort ()));
	if (stats.total_payload_download)
		Ui_.LabelSessionRating_->
			setText (QString::number (stats.total_payload_upload /
						static_cast<double> (stats.total_payload_download), 'g', 4));
	else
		Ui_.LabelSessionRating_->setText (QString::fromUtf8 ("\u221E"));
	Ui_.LabelTotalFailedData_->
		setText (Proxy::Instance ()->MakePrettySize (stats.total_failed_bytes));
	Ui_.LabelTotalRedundantData_->
		setText (Proxy::Instance ()->MakePrettySize (stats.total_redundant_bytes));
	Ui_.LabelExternalAddress_->
		setText (Core::Instance ()->GetExternalAddress ());

	libtorrent::cache_status cs = Core::Instance ()->GetCacheStats ();
	Ui_.BlocksWritten_->setText (QString::number (cs.blocks_written));
	Ui_.Writes_->setText (QString::number (cs.writes));
	Ui_.WriteHitRatio_->
		setText (QString::number (static_cast<double> (cs.blocks_written - cs.writes ) /
				static_cast<double> (cs.blocks_written)));
	Ui_.CacheSize_->setText (QString::number (cs.cache_size));
	Ui_.TotalBlocksRead_->setText (QString::number (cs.blocks_read));
	Ui_.CachedBlockReads_->setText (QString::number (cs.blocks_read_hit));
	Ui_.ReadHitRatio_->
		setText (QString::number (static_cast<double> (cs.blocks_read_hit) /
				static_cast<double> (cs.blocks_read)));
	Ui_.ReadCacheSize_->setText (QString::number (cs.read_cache_size));

	Core::pertrackerstats_t ptstats;
	Core::Instance ()->GetPerTracker (ptstats);
	Ui_.PerTrackerStats_->clear ();
	for (Core::pertrackerstats_t::const_iterator i = ptstats.begin (),
			end = ptstats.end (); i != end; ++i)
	{
		QStringList strings;
		strings	<< i->first
			<< Proxy::Instance ()->MakePrettySize (i->second.DownloadRate_) + tr ("/s")
			<< Proxy::Instance ()->MakePrettySize (i->second.UploadRate_) + tr ("/s");

		new QTreeWidgetItem (Ui_.PerTrackerStats_, strings);
	}
}

void TorrentPlugin::doLogMessage (const QString& msg)
{
	emit log (msg);
}

void TorrentPlugin::setTabWidgetSettings ()
{
	Ui_.BoxSessionStats_->setVisible (XmlSettingsManager::Instance ()->
			property ("ActiveSessionStats").toBool ());
	Ui_.BoxAdvancedSessionStats_->setVisible (XmlSettingsManager::Instance ()->
			property ("ActiveAdvancedSessionStats").toBool ());
	Ui_.BoxPerTrackerStats_->setVisible (XmlSettingsManager::Instance ()->
			property ("ActiveTrackerStats").toBool ());
	Ui_.BoxCacheStats_->setVisible (XmlSettingsManager::Instance ()->
			property ("ActiveCacheStats").toBool ());
	Ui_.BoxTorrentStatus_->setVisible (XmlSettingsManager::Instance ()->
			property ("ActiveTorrentStatus").toBool ());
	Ui_.BoxTorrentAdvancedStatus_->setVisible (XmlSettingsManager::Instance ()->
			property ("ActiveTorrentAdvancedStatus").toBool ());
	Ui_.BoxTorrentInfo_->setVisible (XmlSettingsManager::Instance ()->
			property ("ActiveTorrentInfo").toBool ());
	Ui_.BoxTorrentPeers_->setVisible (XmlSettingsManager::Instance ()->
			property ("ActiveTorrentPeers").toBool ());
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
	Ui_.TorrentDownloadRateController_->
		setValue (Core::Instance ()->GetTorrentDownloadRate ());
	Ui_.TorrentUploadRateController_->setValue (Core::Instance ()->
			GetTorrentUploadRate ());
	Ui_.TorrentDesiredRating_->setValue (Core::Instance ()->
			GetTorrentDesiredRating ());
	Ui_.TorrentManaged_->setCheckState (Core::Instance ()->
			IsTorrentManaged () ? Qt::Checked : Qt::Unchecked);
	Ui_.TorrentSequentialDownload_->setCheckState (Core::Instance ()->
			IsTorrentSequentialDownload () ? Qt::Checked : Qt::Unchecked);
	Ui_.TorrentSuperSeeding_->setCheckState (Core::Instance ()->
			IsTorrentSuperSeeding () ? Qt::Checked : Qt::Unchecked);

	std::auto_ptr<TorrentInfo> i;
	try
	{
		i = Core::Instance ()->GetTorrentStats ();
	}
	catch (...)
	{
		Ui_.TorrentControlTab_->setEnabled (false);
		return;
	}

	Ui_.TorrentControlTab_->setEnabled (true);
	Ui_.LabelState_->setText (i->State_);
	Ui_.LabelDownloadRate_->
		setText (Proxy::Instance ()->
				MakePrettySize (i->Status_.download_rate) + tr ("/s"));
	Ui_.LabelUploadRate_->
		setText (Proxy::Instance ()->
				MakePrettySize (i->Status_.upload_rate) + tr ("/s"));
	Ui_.LabelNextAnnounce_->
		setText (QTime (i->Status_.next_announce.hours (),
					i->Status_.next_announce.minutes (),
					i->Status_.next_announce.seconds ()).toString ());
	Ui_.LabelProgress_->
		setText (QString::number (i->Status_.progress * 100, 'f', 2) + "%");
	Ui_.LabelDownloaded_->
		setText (Proxy::Instance ()->MakePrettySize (i->Status_.total_download));
	Ui_.LabelUploaded_->
		setText (Proxy::Instance ()->MakePrettySize (i->Status_.total_upload));
	Ui_.LabelWantedDownloaded_->
		setText (Proxy::Instance ()->MakePrettySize (i->Status_.total_wanted_done));
	Ui_.LabelDownloadedTotal_->
		setText (Proxy::Instance ()->MakePrettySize (i->Status_.all_time_download));
	Ui_.LabelUploadedTotal_->
		setText (Proxy::Instance ()->MakePrettySize (i->Status_.all_time_upload));
	if (i->Status_.all_time_download)
		Ui_.LabelTorrentOverallRating_->
			setText (QString::number (i->Status_.all_time_upload /
						static_cast<double> (i->Status_.all_time_download), 'g', 4));
	else
		Ui_.LabelTorrentOverallRating_->
			setText (QString::fromUtf8 ("\u221E"));
	Ui_.LabelActiveTime_->
		setText (Proxy::Instance ()->MakeTimeFromLong (i->Status_.active_time));
	Ui_.LabelSeedingTime_->
		setText (Proxy::Instance ()->MakeTimeFromLong (i->Status_.seeding_time));
	Ui_.LabelSeedRank_->
		setText (QString::number (i->Status_.seed_rank));
	if (i->Status_.last_scrape >= 0)
		Ui_.LabelLastScrape_->
			setText (Proxy::Instance ()->MakeTimeFromLong (i->Status_.last_scrape));
	else
		Ui_.LabelLastScrape_->
			setText (tr ("Wasn't yet"));
	Ui_.LabelTotalSize_->
		setText (Proxy::Instance ()->MakePrettySize (i->Info_->total_size ()));
	Ui_.LabelWantedSize_->
		setText (Proxy::Instance ()->MakePrettySize (i->Status_.total_wanted));
	if (i->Status_.total_payload_download)
		Ui_.LabelTorrentRating_->
			setText (QString::number (i->Status_.total_payload_upload /
						static_cast<double> (i->Status_.total_payload_download), 'g', 4));
	else
		Ui_.LabelTorrentRating_->
			setText (QString::fromUtf8 ("\u221E"));
	Ui_.PiecesWidget_->setPieceMap (i->Status_.pieces);
	Ui_.LabelTracker_->
		setText (QString::fromStdString (i->Status_.current_tracker));
	Ui_.LabelDestination_->
		setText (i->Destination_);
	Ui_.LabelName_->
		setText (QString::fromUtf8 (i->Info_->name ().c_str ()));
	Ui_.LabelCreator_->
		setText (QString::fromUtf8 (i->Info_->creator ().c_str ()));
	Ui_.LabelComment_->
		setText (QString::fromUtf8 (i->Info_->comment ().c_str ()));
	Ui_.LabelPrivate_->
		setText (i->Info_->priv () ? tr ("Yes") : tr ("No"));
	Ui_.LabelDHTNodesCount_->
		setText (QString::number (i->Info_->nodes ().size ()));
	Ui_.LabelFailed_->
		setText (Proxy::Instance ()->MakePrettySize (i->Status_.total_failed_bytes));
	Ui_.LabelConnectedPeers_->
		setText (QString::number (i->Status_.num_peers));
	Ui_.LabelConnectedSeeds_->
		setText (QString::number (i->Status_.num_seeds));
	Ui_.LabelAnnounceInterval_->
		setText (QTime (i->Status_.announce_interval.hours (),
					i->Status_.announce_interval.minutes (),
					i->Status_.announce_interval.seconds ()).toString ());
	Ui_.LabelTotalPieces_->
		setText (QString::number (i->Info_->num_pieces ()));
	Ui_.LabelDownloadedPieces_->
		setText (QString::number (i->Status_.num_pieces));
	Ui_.LabelPieceSize_->
		setText (Proxy::Instance ()->MakePrettySize (i->Info_->piece_length ()));
	Ui_.LabelBlockSize_->
		setText (Proxy::Instance ()->MakePrettySize (i->Status_.block_size));
	Ui_.LabelDistributedCopies_->
		setText (i->Status_.distributed_copies == -1 ?
			tr ("Not tracking") : QString::number (i->Status_.distributed_copies));
	Ui_.LabelRedundantData_->
		setText (Proxy::Instance ()->MakePrettySize (i->Status_.total_redundant_bytes));
	Ui_.LabelPeersInList_->
		setText (QString::number (i->Status_.list_peers));
	Ui_.LabelSeedsInList_->
		setText (QString::number (i->Status_.list_seeds));
	Ui_.LabelPeersInSwarm_->
		setText ((i->Status_.num_incomplete == -1 ?
				tr ("Unknown") : QString::number (i->Status_.num_incomplete)));
	Ui_.LabelSeedsInSwarm_->
		setText ((i->Status_.num_complete == -1 ?
			  tr ("Unknown") : QString::number (i->Status_.num_complete)));
	Ui_.LabelConnectCandidates_->
		setText (QString::number (i->Status_.connect_candidates));
	Ui_.LabelUpBandwidthQueue_->
		setText (QString::number (i->Status_.up_bandwidth_queue));
	Ui_.LabelDownBandwidthQueue_->
		setText (QString::number (i->Status_.down_bandwidth_queue));
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
	connect (Ui_.TorrentSuperSeeding_,
			SIGNAL (stateChanged (int)),
			this,
			SLOT (on_TorrentSuperSeeding__stateChanged (int)));
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

	QList<QByteArray> tabWidgetSettings;
	tabWidgetSettings << "ActiveSessionStats"
		<< "ActiveAdvancedSessionStats"
		<< "ActiveTrackerStats"
		<< "ActiveCacheStats"
		<< "ActiveTorrentStatus"
		<< "ActiveTorrentAdvancedStatus"
		<< "ActiveTorrentInfo"
		<< "ActiveTorrentPeers";
    XmlSettingsManager::Instance ()->RegisterObject (tabWidgetSettings,
			this, "setTabWidgetSettings");

	setTabWidgetSettings ();
}

void TorrentPlugin::SetupCore ()
{
	SetupTabWidget ();
	SetupActions ();
    TorrentSelectionChanged_ = true;
    LastPeersUpdate_.reset (new QTime);
    LastPeersUpdate_->start ();
    XmlSettingsDialog_.reset (new LeechCraft::Util::XmlSettingsDialog ());
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
			SIGNAL (fileFinished (const LeechCraft::DownloadEntity&)),
			this,
			SIGNAL (gotEntity (const LeechCraft::DownloadEntity&)));
	connect (Core::Instance (),
			SIGNAL (dataChanged (const QModelIndex&,
					const QModelIndex&)),
			this,
			SLOT (updateTorrentStats ()));
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

	Core::Instance ()->SetWidgets (Toolbar_.get (), TabWidget_.get ());
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
	Ui_.TorrentTags_->AddSelector ();

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

	QFontMetrics fm = QApplication::fontMetrics ();
	QHeaderView *header = Ui_.PerTrackerStats_->header ();
	header->resizeSection (0,
			fm.width ("www.domain.name.org"));
	header->resizeSection (1,
			fm.width ("1234.5678 bytes/s"));
	header->resizeSection (2,
			fm.width ("1234.5678 bytes/s"));
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

	Import_.reset (new QAction (tr ("Import..."),
				Toolbar_.get ()));
	connect (Import_.get (),
			SIGNAL (triggered ()),
			this,
			SLOT (on_Import__triggered ()));
	Import_->setProperty ("ActionIcon", "torrent_import");

	Export_.reset (new QAction (tr ("Export..."),
				Toolbar_.get ()));
	connect (Export_.get (),
			SIGNAL (triggered ()),
			this,
			SLOT (on_Export__triggered ()));
	Export_->setProperty ("ActionIcon", "torrent_export");

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
	Toolbar_->addAction (Import_.get ());
	Toolbar_->addAction (Export_.get ());
}

Q_EXPORT_PLUGIN2 (leechcraft_torrent, TorrentPlugin);


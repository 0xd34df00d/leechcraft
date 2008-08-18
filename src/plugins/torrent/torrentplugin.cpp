#include <QtGui/QtGui>
#include <plugininterface/proxy.h>
#include <plugininterface/tagscompleter.h>
#include <plugininterface/tagscompletionmodel.h>
#include "torrentplugin.h"
#include "core.h"
#include "addtorrent.h"
#include "addmultipletorrents.h"
#include "newtorrentwizard.h"
#include "trackerschanger.h"
#include "xmlsettingsmanager.h"
#include "piecesmodel.h"
#include "peersmodel.h"
#include "channelsfiltermodel.h"
#include "torrentfilesmodel.h"
#include "filesviewdelegate.h"
#include "movetorrentfiles.h"

void TorrentPlugin::SetupCore ()
{
    setupUi (this);
    TorrentSelectionChanged_ = true;
    LastPeersUpdate_ = new QTime;
    LastPeersUpdate_->start ();
    IsShown_ = false;
    XmlSettingsDialog_ = new XmlSettingsDialog (this);
    XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (), ":/torrentsettings.xml");
    AddTorrentDialog_ = new AddTorrent (this);
	Core::Instance ()->SetWindow (this);
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
					quint64, QDateTime)),
			this,
			SLOT (addToHistory (const QString&, const QString&,
					quint64, QDateTime)));
	connect (Stats_,
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
	connect (TorrentView_->selectionModel (),
			SIGNAL (currentRowChanged (const QModelIndex&, const QModelIndex&)),
			this,
			SLOT (itemSelectionChanged (const QModelIndex&)));
	connect (Stats_,
			SIGNAL (currentChanged (int)),
			this,
			SLOT (tabChanged ()));

    Core::Instance ()->DoDelayedInit ();
}

void TorrentPlugin::SetupTorrentView ()
{
    FilterModel_ = new ChannelsFilterModel;
    FilterModel_->setSourceModel (Core::Instance ());
    FilterModel_->setFilterKeyColumn (0);
    TorrentView_->setModel (FilterModel_);
	connect (Core::Instance (),
			SIGNAL (dataChanged (const QModelIndex&,
					const QModelIndex&)),
			FilterModel_,
			SLOT (invalidate ()));
	connect (FixedStringSearch_,
			SIGNAL (textChanged (const QString&)),
			FilterModel_,
			SLOT (setNormalMode ()));
	connect (WildcardSearch_,
			SIGNAL (textChanged (const QString&)),
			FilterModel_,
			SLOT (setNormalMode ()));
	connect (RegexpSearch_,
			SIGNAL (textChanged (const QString&)),
			FilterModel_,
			SLOT (setNormalMode ()));
	connect (FixedStringSearch_,
			SIGNAL (textChanged (const QString&)),
			FilterModel_,
			SLOT (setFilterFixedString (const QString&)));
	connect (WildcardSearch_,
			SIGNAL (textChanged (const QString&)),
			FilterModel_,
			SLOT (setFilterWildcard (const QString&)));
	connect (RegexpSearch_,
			SIGNAL (textChanged (const QString&)),
			FilterModel_,
			SLOT (setFilterRegExp (const QString&)));
	connect (TagsSearch_,
			SIGNAL (textChanged (const QString&)),
			FilterModel_,
			SLOT (setTagsMode ()));
	connect (TagsSearch_,
			SIGNAL (textChanged (const QString&)),
			FilterModel_,
			SLOT (setFilterFixedString (const QString&)));
}

void TorrentPlugin::SetupStuff ()
{
    TagsChangeCompleter_ = new TagsCompleter (TorrentTags_, this);
    TagsSearchCompleter_ = new TagsCompleter (TagsSearch_, this);
    TagsAddDiaCompleter_ = new TagsCompleter (AddTorrentDialog_->GetEdit (), this);
    TagsChangeCompleter_->setModel (Core::Instance ()->GetTagsCompletionModel ());
    TagsSearchCompleter_->setModel (Core::Instance ()->GetTagsCompletionModel ());
    TagsAddDiaCompleter_->setModel (Core::Instance ()->GetTagsCompletionModel ());

    PiecesView_->setModel (Core::Instance ()->GetPiecesModel ());

    FilesView_->setModel (Core::Instance ()->GetTorrentFilesModel ());
    FilesView_->setItemDelegate (new FilesViewDelegate (this));

    QSortFilterProxyModel *peersSorter = new QSortFilterProxyModel (this);
    peersSorter->setDynamicSortFilter (true);
    peersSorter->setSourceModel (Core::Instance ()->GetPeersModel ());
    peersSorter->setSortRole (PeersModel::SortRole);
    PeersView_->setModel (peersSorter);

    IgnoreTimer_ = true;
    OverallDownloadRateController_->setValue (Core::Instance ()->GetOverallDownloadRate ());
    OverallUploadRateController_->setValue (Core::Instance ()->GetOverallUploadRate ());
    DownloadingTorrents_->setValue (Core::Instance ()->GetMaxDownloadingTorrents ());
    UploadingTorrents_->setValue (Core::Instance ()->GetMaxUploadingTorrents ());
    DesiredRating_->setValue (Core::Instance ()->GetDesiredRating ());
    
    OverallStatsUpdateTimer_ = new QTimer (this);
    connect (OverallStatsUpdateTimer_, SIGNAL (timeout ()), this, SLOT (updateOverallStats ()));
    OverallStatsUpdateTimer_->start (500);
}

void TorrentPlugin::SetupHeaders ()
{
    QFontMetrics fm = fontMetrics ();
    QHeaderView *header = TorrentView_->header ();
    header->resizeSection (Core::ColumnName, fm.width ("thisisanaveragewareztorrentname,right?maybeyes.torrent"));
    header->resizeSection (Core::ColumnDownloaded, fm.width ("_1234.0 KB_"));
    header->resizeSection (Core::ColumnUploaded, fm.width ("_1234.0 KB_"));
    header->resizeSection (Core::ColumnRating, fm.width ("_12.345_"));
    header->resizeSection (Core::ColumnSize, fm.width ("_1234.0 KB_"));
    header->resizeSection (Core::ColumnProgress, fm.width ("_100%_"));
    header->resizeSection (Core::ColumnState, fm.width ("__Downloading__"));
    header->resizeSection (Core::ColumnSP, fm.width ("_123/123_"));
    header->resizeSection (Core::ColumnUSpeed, fm.width ("_1234.0 KB/s_"));
    header->resizeSection (Core::ColumnDSpeed, fm.width ("_1234.0 KB/s_"));
    header->resizeSection (Core::ColumnRemaining, fm.width ("10d 00:00:00"));

    header = PeersView_->header ();
    header->resizeSection (0, fm.width ("000.000.000.000"));
    header->resizeSection (1, fm.width ("_1234.0 KB/s_"));
    header->resizeSection (2, fm.width ("_1234.0 KB/s_"));
    header->resizeSection (3, fm.width ("_1234.0 KB_"));
    header->resizeSection (4, fm.width ("_1234.0 KB_"));
    header->resizeSection (5, fm.width ("LeechCraft rulez"));
    header->resizeSection (6, fm.width ("888, 888 we don't have"));
    header->resizeSection (7, fm.width ("000"));
    header->resizeSection (8, fm.width ("00:00"));
    header->resizeSection (9, fm.width ("00:00"));
    header->resizeSection (10, fm.width ("99"));
    header->resizeSection (11, fm.width ("99"));
    header->resizeSection (12, fm.width ("Piece 100, block 50, 16384 of 16384 bytes"));

    header = FilesView_->header ();
    header->resizeSection (0, fm.width ("Thisisanaveragetorrentcontainedfilename,ormaybeevenbiggerthanthat!"));
    header->resizeSection (1, fm.width ("_999.9 MB_"));
    header->resizeSection (2, fm.width ("_8_"));
    header->resizeSection (3, fm.width ("_0.0246_"));
}

void TorrentPlugin::Init ()
{
    QTranslator *transl = new QTranslator (this);
    QString localeName = QString(::getenv ("LANG")).left (2);
    if (localeName.isNull () || localeName.isEmpty ())
        localeName = QLocale::system ().name ();
    transl->load (QString (":/leechcraft_torrent_") + localeName);
    qApp->installTranslator (transl);

    SetupCore ();
    SetupTorrentView ();
    SetupStuff ();
    SetupHeaders ();

    Plugins_->addAction (OpenTorrent_);
    Plugins_->addAction (OpenMultipleTorrents_);
    Plugins_->addAction (CreateTorrent_);
    Plugins_->addSeparator ();
    Plugins_->addAction (Preferences_);

    setActionsEnabled ();
    LogShower_->setPlainText ("BitTorrent initialized");
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName () + "_Torrent");
    int max = settings.beginReadArray ("History");
    for (int i = 0; i < max; ++i)
    {
        settings.setArrayIndex (i);
        QTreeWidgetItem *item = new QTreeWidgetItem (HistoryTree_);
        item->setText (0, settings.value ("Name").toString ());
        item->setText (1, settings.value ("Where").toString ());
        item->setText (2, settings.value ("TorrentSize").toString ());
        item->setText (3, settings.value ("Date").toString ());
    }
    settings.endArray ();
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

void TorrentPlugin::PushMainWindowExternals (const MainWindowExternals& ex)
{
    Plugins_ = ex.RootMenu_->addMenu (tr ("&BitTorrent"));
}

void TorrentPlugin::Release ()
{
    Core::Instance ()->Release ();
    XmlSettingsManager::Instance ()->Release ();
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName () + "_Torrent");
    settings.beginWriteArray ("History");
    for (int i = 0; i < HistoryTree_->topLevelItemCount (); ++i)
    {
        settings.setArrayIndex (i);
        settings.setValue ("Name", HistoryTree_->topLevelItem (i)->text (0));
        settings.setValue ("Where", HistoryTree_->topLevelItem (i)->text (1));
        settings.setValue ("TorrentSize", HistoryTree_->topLevelItem (i)->text (2));
        settings.setValue ("Date", HistoryTree_->topLevelItem (i)->text (3));
    }
    settings.endArray ();
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

QList<QVariantList> TorrentPlugin::GetAll () const
{
    QList<QVariantList> result;

    QVariantList t;
    for (int i = 0; i < Core::Instance ()->columnCount (QModelIndex ()); ++i)
        t << Core::Instance ()->headerData (i, Qt::Horizontal);
    result << t;

    for (int i = 0; i < Core::Instance ()->rowCount (); ++i)
    {
        QVariantList tmp;
        for (int j = 0; j < Core::Instance ()->columnCount (QModelIndex ()); ++j)
            tmp << Core::Instance ()->data (Core::Instance ()->index (i, j));
        result << tmp;
    }
    return result;
}

IRemoteable::AddJobType TorrentPlugin::GetAddJobType () const
{
    return AJTFile;
}

void TorrentPlugin::AddJob (const QByteArray& data, const QString& where)
{
    QVector<bool> files (true);
    QTemporaryFile file ("lc.remoteadded.XXXXXX");
    if (!file.open ())
    {
        showError ("Could not open temporary file to add the interface-added job");
    }
    file.write (data);
    Core::Instance ()->AddFile (file.fileName (), where, QStringList (tr ("untagged")), files);
    setActionsEnabled ();
}

void TorrentPlugin::StartAt (int pos)
{
    Core::Instance ()->ResumeTorrent (pos);
    setActionsEnabled ();
}

void TorrentPlugin::StopAt (int pos)
{
    Core::Instance ()->PauseTorrent (pos);
    setActionsEnabled ();
}

void TorrentPlugin::DeleteAt (int pos)
{
    Core::Instance ()->RemoveTorrent (pos);
    setActionsEnabled ();
}

QAbstractItemModel* TorrentPlugin::GetRepresentation () const
{
    return Core::Instance ();
}

QAbstractItemDelegate* TorrentPlugin::GetDelegate () const
{
    return 0;
}

bool TorrentPlugin::CouldDownload (const QString& string, LeechCraft::TaskParameters) const
{
    QFile file (string);
    if (!file.exists () || !file.open (QIODevice::ReadOnly))
        return false;

    return Core::Instance ()->IsValidTorrent (file.readAll ());
}

int TorrentPlugin::AddJob (const QString& name, LeechCraft::TaskParameters parameters)
{
    AddTorrentDialog_->Reinit ();
    AddTorrentDialog_->SetFilename (name);

	QString path;
	QStringList tags;
	QVector<bool> files;
	QString fname;
	if (parameters & LeechCraft::FromAutomatic)
	{
		fname = name;
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
	std::auto_ptr<NewTorrentWizard> wizard (new NewTorrentWizard (this));
    if (wizard->exec () == QDialog::Accepted)
        Core::Instance ()->MakeTorrent (wizard->GetParams ());
    setActionsEnabled ();
}

void TorrentPlugin::on_RemoveTorrent__triggered ()
{
    if (QMessageBox::question (this, tr ("Question"), tr ("Do you really want to delete the torrent?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        return;

    int row = TorrentView_->currentIndex ().row ();
    if (row == -1)
        return;

    Core::Instance ()->RemoveTorrent (row);
    TorrentSelectionChanged_ = true;
    updateTorrentStats ();
    setActionsEnabled ();
}

void TorrentPlugin::on_Resume__triggered ()
{
    Core::Instance ()->ResumeTorrent (TorrentView_->currentIndex ().row ());
    setActionsEnabled ();
}

void TorrentPlugin::on_Stop__triggered ()
{
    Core::Instance ()->PauseTorrent (TorrentView_->currentIndex ().row ());
    setActionsEnabled ();
}

void TorrentPlugin::on_ForceReannounce__triggered ()
{
    Core::Instance ()->ForceReannounce (TorrentView_->currentIndex ().row ());
}

void TorrentPlugin::on_ChangeTrackers__triggered ()
{
    QStringList trackers = Core::Instance ()->GetTrackers (TorrentView_->currentIndex ().row ());
    TrackersChanger changer;
    changer.SetTrackers (trackers);
    if (changer.exec () == QDialog::Accepted)
    {
        QStringList newTrackers = changer.GetTrackers ();
        Core::Instance ()->SetTrackers (TorrentView_->currentIndex ().row (), newTrackers);
    }
}

void TorrentPlugin::on_Preferences__triggered ()
{
    XmlSettingsDialog_->show ();
    XmlSettingsDialog_->setWindowTitle (windowTitle () + tr (": Preferences"));
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
    QModelIndex index = TorrentView_->currentIndex ();
    if (!index.isValid ())
        return;

    Core::Instance ()->SetTorrentDownloadRate (val, index.row ());
}

void TorrentPlugin::on_TorrentUploadRateController__valueChanged (int val)
{
    QModelIndex index = TorrentView_->currentIndex ();
    if (!index.isValid ())
        return;

    Core::Instance ()->SetTorrentUploadRate (val, index.row ());
}

void TorrentPlugin::on_TorrentDesiredRating__valueChanged (double val)
{
    QModelIndex index = TorrentView_->currentIndex ();
    if (!index.isValid ())
        return;

    Core::Instance ()->SetTorrentDesiredRating (val, index.row ());
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
    QModelIndex i = FilterModel_->mapToSource (TorrentView_->selectionModel ()->currentIndex ());
    if (!i.isValid ())
        return;

    Core::Instance ()->UpdateTags (i.row (), TorrentTags_->text ().split (' ', QString::SkipEmptyParts));
}

void TorrentPlugin::on_MoveFiles__triggered ()
{
    QModelIndex i = FilterModel_->mapToSource (TorrentView_->selectionModel ()->currentIndex ());
    if (!i.isValid ())
        return;

    QString oldDir = Core::Instance ()->GetTorrentDirectory (i.row ());
    MoveTorrentFiles mtf (oldDir, this);
    if (mtf.exec () == QDialog::Rejected)
        return;
    QString newDir = mtf.GetNewLocation ();
    if (oldDir == newDir)
        return;

    if (Core::Instance ()->MoveTorrentFiles (i.row (), newDir))
        QMessageBox::information (this, tr ("Information"),
                tr ("Started moving torrent's files from %1 to %2").
                arg (oldDir).
                arg (newDir));
    else
        QMessageBox::warning (this, tr ("Warning"),
                tr ("Failed to move torrent's files from %1 to %2").
                arg (oldDir).
                arg (newDir));
}

void TorrentPlugin::itemSelectionChanged (const QModelIndex& index)
{
    Core::Instance ()->SetCurrentTorrent (FilterModel_->mapToSource (index).row ());
    setActionsEnabled ();
    TorrentTags_->setText (Core::Instance ()->GetTagsForIndex (FilterModel_->mapToSource (index).row ()).join (" "));
    TorrentSelectionChanged_ = true;
    restartTimers ();
    updateTorrentStats ();
}

void TorrentPlugin::tabChanged ()
{
	updateTorrentStats ();
}

void TorrentPlugin::setActionsEnabled ()
{
    RemoveTorrent_->setEnabled (TorrentView_->selectionModel ()->currentIndex ().isValid ());
    Stop_->setEnabled (TorrentView_->selectionModel ()->currentIndex ().isValid ());
    Resume_->setEnabled (TorrentView_->selectionModel ()->currentIndex ().isValid ());
    ForceReannounce_->setEnabled (TorrentView_->selectionModel ()->currentIndex ().isValid ());
    ChangeTrackers_->setEnabled (TorrentView_->selectionModel ()->currentIndex ().isValid ());
}

void TorrentPlugin::showError (QString e)
{
    qWarning () << e;
    QMessageBox::warning (this, tr ("Error!"), e);
}

void TorrentPlugin::restartTimers ()
{
    IgnoreTimer_ = true;
}

void TorrentPlugin::updateTorrentStats ()
{
	switch (Stats_->currentIndex ())
	{
		case 0:
			break;
		case 1:
			UpdateDashboard ();
			break;
		case 2:
			updateOverallStats ();
			break;
		case 3:
			UpdateTorrentPage ();
			break;
		case 4:
			UpdateFilesPage ();
			break;
		case 5:
			UpdatePeersPage ();
			break;
		case 6:
			UpdatePiecesPage ();
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

void TorrentPlugin::doLogMessage (const QString& msg)
{
    LogShower_->append (msg.trimmed ());
}

void TorrentPlugin::addToHistory (const QString& name, const QString& where, quint64 size, QDateTime when)
{
    QList<QTreeWidgetItem*> items = HistoryTree_->findItems (name, Qt::MatchExactly);
    for (int i = 0; i < items.size (); ++i)
        if (items.at (i)->text (1) == where)
            return;

    QTreeWidgetItem *item = new QTreeWidgetItem (HistoryTree_);
    item->setText (0, name);
    item->setText (1, where);
    item->setText (2, Proxy::Instance ()->MakePrettySize (size));
    item->setText (3, when.toString ());
}

void TorrentPlugin::UpdateDashboard ()
{
    QModelIndex index = TorrentView_->currentIndex ();
    if (index.isValid ())
    {
        int row = FilterModel_->mapToSource (index).row ();
        TorrentDownloadRateController_->setValue (Core::Instance ()->GetTorrentDownloadRate (row));
        TorrentUploadRateController_->setValue (Core::Instance ()->GetTorrentUploadRate (row));
        TorrentDesiredRating_->setValue (Core::Instance ()->GetTorrentDesiredRating (row));
    }
}

void TorrentPlugin::UpdateTorrentPage ()
{
    QModelIndex index = TorrentView_->currentIndex ();
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
        LabelDistributedCopies_->setText ("<>");
        PiecesWidget_->setPieceMap (std::vector<bool> ());
    }
    else
    {
        TorrentInfo i = Core::Instance ()->GetTorrentStats (FilterModel_->mapToSource (index).row ());
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
        LabelDistributedCopies_->setText (QString::number (i.DistributedCopies_));
        PiecesWidget_->setPieceMap (*(i.Pieces_));
    }
}

void TorrentPlugin::UpdateFilesPage ()
{
    QModelIndex index = FilterModel_->mapToSource (TorrentView_->currentIndex ());
    if (!index.isValid ())
    {
        Core::Instance ()->ClearFiles ();
        return;
    }

    Core::Instance ()->SetCurrentTorrent (index.row ());

    if (TorrentSelectionChanged_)
    {
        Core::Instance ()->ResetFiles (index.row ());
        FilesView_->expandAll ();
    }
    else
        Core::Instance ()->UpdateFiles (index.row ());
}

void TorrentPlugin::UpdatePeersPage ()
{
    QModelIndex index = FilterModel_->mapToSource (TorrentView_->currentIndex ());
    if (!index.isValid ())
        Core::Instance ()->ClearPeers ();
    else
        Core::Instance ()->UpdatePeers (index.row ());
}

void TorrentPlugin::UpdatePiecesPage ()
{
    QModelIndex index = FilterModel_->mapToSource (TorrentView_->currentIndex ());
    if (!index.isValid ())
        Core::Instance ()->ClearPieces ();
    else
        Core::Instance ()->UpdatePieces (index.row ());
}
Q_EXPORT_PLUGIN2 (leechcraft_torrent, TorrentPlugin);


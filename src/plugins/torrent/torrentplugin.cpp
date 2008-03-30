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
    TorrentSelectionChanged_ = true;
    LastPeersUpdate_ = new QTime;
    LastPeersUpdate_->start ();
    IsShown_ = false;
    XmlSettingsDialog_ = new XmlSettingsDialog (this);
    XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (), ":/torrentsettings.xml");
    AddTorrentDialog_ = new AddTorrent (this);
    connect (Core::Instance (), SIGNAL (error (QString)), this, SLOT (showError (QString)));
    connect (Core::Instance (), SIGNAL (logMessage (const QString&)), this, SLOT (doLogMessage (const QString&)));
    connect (Core::Instance (), SIGNAL (torrentFinished (const QString&)), this, SIGNAL (downloadFinished (const QString&)));
    connect (Core::Instance (), SIGNAL (fileFinished (const QString&)), this, SIGNAL (fileDownloaded (const QString&)));
    connect (Core::Instance (), SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)), this, SLOT (updateTorrentStats ()));
    connect (Stats_, SIGNAL (currentChanged (int)), this, SLOT (updateTorrentStats ()));

    Core::Instance ()->DoDelayedInit ();
    FilterModel_ = new QSortFilterProxyModel;
    FilterModel_->setSourceModel (Core::Instance ());
    FilterModel_->setFilterKeyColumn (0);
    TorrentView_->setModel (FilterModel_);
    connect (Core::Instance (), SIGNAL (dataChanged (const QModelIndex&, const QModelIndex&)), FilterModel_, SLOT (invalidate ()));
    connect (FixedStringSearch_, SIGNAL (textChanged (const QString&)), FilterModel_, SLOT (setFilterFixedString (const QString&)));
    connect (WildcardSearch_, SIGNAL (textChanged (const QString&)), FilterModel_, SLOT (setFilterWildcard (const QString&)));
    connect (RegexpSearch_, SIGNAL (textChanged (const QString&)), FilterModel_, SLOT (setFilterRegExp (const QString&)));

    IgnoreTimer_ = true;
    OverallDownloadRateController_->setValue (Core::Instance ()->GetOverallDownloadRate ());
    OverallUploadRateController_->setValue (Core::Instance ()->GetOverallUploadRate ());
    DesiredRating_->setValue (Core::Instance ()->GetDesiredRating ());
    
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

    LogShower_->setPlainText ("BitTorrent initialized");
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
}

void TorrentPlugin::StopAll ()
{
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
    Core::Instance ()->AddFile (file.fileName (), where, files);
}

void TorrentPlugin::StartAt (int pos)
{
    Core::Instance ()->ResumeTorrent (pos);
}

void TorrentPlugin::StopAt (int pos)
{
    Core::Instance ()->PauseTorrent (pos);
}

void TorrentPlugin::DeleteAt (int pos)
{
    Core::Instance ()->RemoveTorrent (pos);
}

QAbstractItemModel* TorrentPlugin::GetRepresentation () const
{
    return Core::Instance ();
}

QAbstractItemDelegate* TorrentPlugin::GetDelegate () const
{
    return 0;
}

bool TorrentPlugin::CouldDownload (const QString& string, bool buffer) const
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
    PrioritySpinbox_->setValue (1);
    PrioritySpinbox_->setEnabled (false);
    TorrentSelectionChanged_ = true;
    restartTimers ();
    updateTorrentStats ();
}

void TorrentPlugin::on_TorrentView__pressed (const QModelIndex&)
{
    PrioritySpinbox_->setValue (1);
    PrioritySpinbox_->setEnabled (false);
    TorrentSelectionChanged_ = true;
    restartTimers ();
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

void TorrentPlugin::on_FilesWidget__currentItemChanged (QTreeWidgetItem *current)
{
    int torrent = TorrentView_->currentIndex ().row ();
    int prio = Core::Instance ()->GetFilePriority (torrent, FilesWidget_->indexOfTopLevelItem (current));
    PrioritySpinbox_->setValue (prio);
    PrioritySpinbox_->setEnabled (true);
}

void TorrentPlugin::on_PrioritySpinbox__valueChanged (int val)
{
    int torrent = TorrentView_->currentIndex ().row ();
    Core::Instance ()->SetFilePriority (torrent, FilesWidget_->indexOfTopLevelItem (FilesWidget_->currentItem ()), val);
}

void TorrentPlugin::setActionsEnabled ()
{
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
    QModelIndex index = TorrentView_->currentIndex ();
    switch (Stats_->currentIndex ())
    {
        case 0:
            if (index.isValid ())
            {
                int row = index.row ();
                TorrentDownloadRateController_->setValue (Core::Instance ()->GetTorrentDownloadRate (row));
                TorrentUploadRateController_->setValue (Core::Instance ()->GetTorrentUploadRate (row));
                TorrentDesiredRating_->setValue (Core::Instance ()->GetTorrentDesiredRating (row));
            }
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
                PiecesWidget_->setPieceMap (std::vector<bool> ());
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
                LabelDistributedCopies_->setText (QString::number (i.DistributedCopies_));
                PiecesWidget_->setPieceMap (*(i.Pieces_));
            }
            break;
        case 3:
            if (TorrentSelectionChanged_)
            {
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
            }
            else
            {
                if (index.isValid ())
                {
                    QList<FileInfo> files = Core::Instance ()->GetTorrentFiles (index.row ());
                    for (int i = 0; i < files.size (); ++i)
                        if (FilesWidget_->topLevelItem (i))
                        {
                            FilesWidget_->topLevelItem (i)->setText (2, QString::number (files.at (i).Priority_));
                            FilesWidget_->topLevelItem (i)->setText (3, QString::number (files.at (i).Progress_ * 100) + "%");
                        }
                }
            }
            break;
        case 4:
            if (!IgnoreTimer_ && LastPeersUpdate_->elapsed () < 5000)
                break;
            IgnoreTimer_ = false;

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
                    PiecesWidget *pieces = new PiecesWidget ();
                    PeersWidget_->setItemWidget (item, 7, pieces);
                    pieces->setPieceMap (peers.at (i).Pieces_);
                }
            }
            LastPeersUpdate_->restart ();
            break;
    }
    TorrentSelectionChanged_ = false;
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

Q_EXPORT_PLUGIN2 (leechcraft_torrent, TorrentPlugin);


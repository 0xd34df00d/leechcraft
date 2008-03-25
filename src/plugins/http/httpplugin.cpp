#include <QtGui/QtGui>
#include <QtNetwork/QtNetwork>
#include <QtDebug>
#include <plugininterface/proxy.h>
#include <xmlsettingsdialog/xmlsettingsdialog.h>
#include "columnselector.h"
#include "httpplugin.h"
#include "xmlsettingsmanager.h"
#include "jobadderdialog.h"
#include "jobparams.h"
#include "job.h"
#include "jobmanager.h"
#include "jobrepresentation.h"
#include "joblistitem.h"
#include "finishedjob.h"
#include "mainviewdelegate.h"
#include "contextablelist.h"
#include "jobpropertiesdialog.h"

void HttpPlugin::Init ()
{
    qRegisterMetaType<ImpBase::length_t> ("ImpBase::length_t");
    Q_INIT_RESOURCE (resources);
    QTranslator *transl = new QTranslator (this);
    QString localeName = QLocale::system ().name ();
    transl->load (QString (":/leechcraft_http_") + localeName);
    qApp->installTranslator (transl);

    Ui_.setupUi (this);

    SaveChangesScheduled_ = false;
    ProvidesList_ << "http" << "ftp" << "resume" << "remoteable";
    UsesList_ << "cron";

    connect (&JobManager::Instance (), SIGNAL (showError (QString, QString)), this, SLOT (showJobErrorMessage (QString, QString)));
    connect (&JobManager::Instance (), SIGNAL (cronEnabled ()), this, SLOT (handleCronEnabled ()));
    connect (&JobManager::Instance (), SIGNAL (addToFinishedList (const FinishedJob*, int)), this, SLOT (addToFinishedList (const FinishedJob*, int)));
    connect (&JobManager::Instance (), SIGNAL (jobFinished (int)), this, SIGNAL (jobFinished (int)));
    JobManager::Instance ().SetTheMain (this);
    SetupStatusBarStuff ();

    ReadSettings ();
    JobManager::Instance ().DoDelayedInit ();
    Ui_.TasksList_->setModel (&JobManager::Instance ());

    IsShown_ = false;

    XmlSettingsDialog_ = new XmlSettingsDialog (this);
    XmlSettingsDialog_->RegisterObject (XmlSettingsManager::Instance (), ":/httpsettings.xml");

    QTimer *speedUpdTimer = new QTimer (this);
    speedUpdTimer->setInterval (1000);
    connect (speedUpdTimer, SIGNAL (timeout ()), this, SLOT (handleTotalSpeedUpdate ()));
    speedUpdTimer->start ();
    connect (Ui_.FinishedList_, SIGNAL (itemSelectionChanged ()), this, SLOT (setActionsEnabled ()));
    connect (Ui_.TasksList_->selectionModel (), SIGNAL (selectionChanged (const QItemSelection&, const QItemSelection&)), this, SLOT (setActionsEnabled ()));
    setActionsEnabled ();

    Plugins_->addAction (Ui_.ActionAddJob_);
    Plugins_->addAction (Ui_.ActionStartAll_);
    Plugins_->addAction (Ui_.ActionStopAll_);
    Plugins_->addSeparator ();
    Plugins_->addAction (Ui_.ActionPreferences_);

    Ui_.TasksList_->AddAction (Ui_.ActionRemoveJob_);
    Ui_.TasksList_->AddAction (Ui_.ActionStart_);
    Ui_.TasksList_->AddAction (Ui_.ActionStop_);
    Ui_.TasksList_->AddAction (Ui_.ActionGetFileSize_);
    Ui_.TasksList_->AddAction (Ui_.ActionJobProperties_);

    Ui_.FinishedList_->AddAction (Ui_.ActionRemoveFinished_);
    Ui_.FinishedList_->AddAction (Ui_.ActionCopyFinishedURL_);

    Ui_.TasksList_->setItemDelegate (new MainViewDelegate (this));
    Ui_.TasksList_->header ()->setClickable (false);
    Ui_.TasksList_->header ()->setStretchLastSection (true);
    Ui_.TasksList_->header ()->setHighlightSections (false);
    Ui_.TasksList_->header ()->setDefaultAlignment (Qt::AlignLeft);

    Ui_.FinishedList_->header ()->setStretchLastSection (true);
    Ui_.FinishedList_->header ()->setHighlightSections (false);
    Ui_.FinishedList_->header ()->setDefaultAlignment (Qt::AlignLeft);
}

HttpPlugin::~HttpPlugin ()
{
}

void HttpPlugin::SetupStatusBarStuff ()
{
    SpeedIndicator_ = new QLabel ("0");
    SpeedIndicator_->setMinimumWidth (70);
    SpeedIndicator_->setAlignment (Qt::AlignRight);
    statusBar ()->addPermanentWidget (SpeedIndicator_);
}

QString HttpPlugin::GetName () const
{
    return "HTTP/FTP";
}

QString HttpPlugin::GetInfo () const
{
    return tr ("Simple HTTP and FTP plugin, providing basic functionality.");
}

QString HttpPlugin::GetStatusbarMessage () const
{
    return "Yeah, that works!";
}

IInfo& HttpPlugin::SetID (IInfo::ID_t id)
{
    ID_ = id;
    return *this;
}

IInfo::ID_t HttpPlugin::GetID () const
{
    return ID_;
}

QStringList HttpPlugin::Provides () const
{
    return ProvidesList_;
}

QStringList HttpPlugin::Needs () const
{
    return NeedsList_;
}

QStringList HttpPlugin::Uses () const
{
    return UsesList_;
}

void HttpPlugin::SetProvider (QObject *object, const QString& feature)
{
    JobManager::Instance ().SetProvider (object, feature);
}

void HttpPlugin::PushMainWindowExternals (const MainWindowExternals& ex)
{
    Plugins_ = ex.RootMenu_->addMenu (tr ("&HTTP/FTP"));
}

void HttpPlugin::Release ()
{
    JobManager::Instance ().Release ();
    writeSettings ();
    XmlSettingsManager::Instance ()->Release ();
}

QIcon HttpPlugin::GetIcon () const
{
    return QIcon (":/resources/images/pluginicon.png");
}

void HttpPlugin::SetParent (QWidget *parent)
{
    setParent (parent);
}

void HttpPlugin::ShowWindow ()
{
    IsShown_ = 1 - IsShown_;
    IsShown_ ? show () : hide ();
}

void HttpPlugin::ShowBalloonTip ()
{
}

void HttpPlugin::StartAll ()
{
    JobManager::Instance ().StartAll ();
}

void HttpPlugin::StopAll ()
{
    JobManager::Instance ().StopAll ();
}

bool HttpPlugin::CouldDownload (const QString& string, bool buffer) const
{
    if (buffer)
    {
        QUrl url (string);
        if (url.isValid () && url.scheme () == "ftp" || url.scheme () == "http")
        {
            QStringList extensions = XmlSettingsManager::Instance ()->property ("FiletypesFromBuffer").toString ().split (' ');
            return extensions.contains (QFileInfo (string).suffix ());
        }
        else
            return false;
    }
    else
    {
        QUrl url (string);
        return url.isValid () && url.scheme () == "ftp" || url.scheme () == "http";
    }
}

void HttpPlugin::AddJob (const QString& name)
{
    if (!CouldDownload (name, false))
        return;

    JobAdderDialog *dia = new JobAdderDialog (this);
    dia->SetURL (name);
    connect (dia, SIGNAL (gotParams (JobParams*)), this, SLOT (handleParams (JobParams*)));
    dia->exec ();
    delete dia;
}

QAbstractItemModel* HttpPlugin::GetRepresentation () const
{
    return &JobManager::Instance ();
}

QAbstractItemDelegate* HttpPlugin::GetDelegate () const
{
    return Ui_.TasksList_->itemDelegate ();
}

qint64 HttpPlugin::GetDownloadSpeed () const
{
    return JobManager::Instance ().GetDownloadSpeed ();
}

qint64 HttpPlugin::GetUploadSpeed () const
{
    return 0;
}

int HttpPlugin::AddJob (const DirectDownloadParams& params)
{
    JobParams *jp               = new JobParams;
    jp->URL_                    = params.Resource_;
    jp->LocalName_              = params.Location_;
    jp->Autostart_              = params.Autostart_;
    jp->ShouldBeSavedInHistory_ = params.ShouldBeSavedInHistory_;
    return JobManager::Instance ().addJob (jp);
}

void HttpPlugin::handleHidePlugins ()
{
    IsShown_ = false;
    hide ();
}

void HttpPlugin::on_ActionAddJob__triggered ()
{
    JobAdderDialog *dia = new JobAdderDialog (this);
    connect (dia, SIGNAL (gotParams (JobParams*)), this, SLOT (handleParams (JobParams*)));
    dia->exec ();
    delete dia;
}

void HttpPlugin::handleParams (JobParams *params)
{
    JobManager::Instance ().addJob (params);
}

void HttpPlugin::on_ActionStart__triggered ()
{
    HandleSelected (JAStart);
}

void HttpPlugin::on_ActionStop__triggered ()
{
    HandleSelected (JAStop);
}

void HttpPlugin::on_ActionRemoveJob__triggered ()
{
    HandleSelected (JADelete);
}

void HttpPlugin::on_ActionRemoveFinished__triggered ()
{
    QList<QTreeWidgetItem*> items = Ui_.FinishedList_->selectedItems ();

    for (int i = 0; i < items.size (); ++i)
        delete items [i];

    setActionsEnabled ();
}

void HttpPlugin::on_ActionGetFileSize__triggered ()
{
    HandleSelected (JAGFS);
}

void HttpPlugin::on_ActionSchedule__triggered ()
{
    HandleSelected (JASchedule);
}

void HttpPlugin::on_ActionStartAll__triggered ()
{
    StartAll ();
}

void HttpPlugin::on_ActionStopAll__triggered ()
{
    StopAll ();
}

void HttpPlugin::on_ActionJobProperties__triggered ()
{
    /*
    QList<QTreeWidgetItem*> items = Ui_.TasksList_->selectedItems ();
    if (!items.size ())
        return;

    int id = dynamic_cast<JobListItem*> (items.at (0))->GetID ();

    JobRepresentation *jr = JobManager::Instance ().GetJobRepresentation (id);
    JobPropertiesDialog dia (jr);
    delete jr;

    if (dia.exec () == QDialog::Rejected)
        return;
    else
    {
        JobParams *p = dia.GetParams ();
        JobManager::Instance ().UpdateParams (id, p);
        delete p;
    }
    */
}

void HttpPlugin::on_ActionPreferences__triggered ()
{
    XmlSettingsDialog_->show ();
    XmlSettingsDialog_->setWindowTitle (windowTitle () + tr (": Preferences"));
}

void HttpPlugin::showJobErrorMessage (QString url, QString message)
{
    QMessageBox::warning (this, tr ("Job error"), tr ("Job with URL<br />%1<br />signals about following error:<br /><br /><em>%2</em>").arg (url).arg (message));
}

void HttpPlugin::handleTotalSpeedUpdate ()
{
    SpeedIndicator_->setText (Proxy::Instance ()->MakePrettySize (GetDownloadSpeed ()) + tr ("/s"));
}

void HttpPlugin::on_ActionAutoadjust__triggered ()
{
    if (JobManager::Instance ().rowCount (QModelIndex ()))
        for (int i = 0; i < JobManager::Instance ().columnCount (QModelIndex ()); ++i)
            Ui_.TasksList_->resizeColumnToContents (i);
    if (Ui_.FinishedList_->topLevelItemCount ())
        for (int i = 0; i < Ui_.FinishedList_->columnCount (); ++i)
            Ui_.FinishedList_->resizeColumnToContents (i);
}

void HttpPlugin::writeSettings ()
{
    SaveChangesScheduled_ = false;

    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName () + "_HTTP");
    settings.beginGroup ("geometry");
    settings.setValue ("size", size ());
    settings.setValue ("pos", pos ());
    settings.setValue ("jobListHeadersState", Ui_.TasksList_->header ()->saveState ());
    settings.setValue ("finishedListHeadersState", Ui_.FinishedList_->header ()->saveState ());
    settings.endGroup ();

    settings.beginWriteArray ("finished");
    settings.remove ("");
    for (int i = 0; i < Ui_.FinishedList_->topLevelItemCount (); ++i)
    {
        settings.setArrayIndex (i);
        QByteArray arr;
        QDataStream str (&arr, QIODevice::WriteOnly);
        Ui_.FinishedList_->topLevelItem (i)->write (str);
        settings.setValue ("representation", arr);
    }
    settings.endArray ();
}

void HttpPlugin::copyFinishedURL ()
{
    QList<QTreeWidgetItem*> item = Ui_.FinishedList_->selectedItems ();
    QApplication::clipboard ()->setText (item.at (0)->text (FListURL));
}

void HttpPlugin::setActionsEnabled ()
{
    QModelIndexList tasksItems = JobManager::Instance ().rowCount () ? Ui_.TasksList_->selectionModel ()->selectedIndexes () : QModelIndexList ();
    QList<QTreeWidgetItem*> finishedItems = Ui_.FinishedList_->selectedItems ();
    Ui_.ActionRemoveFinished_->setEnabled (finishedItems.size ());
    Ui_.ActionCopyFinishedURL_->setEnabled (finishedItems.size ());

    Ui_.ActionRemoveJob_->setEnabled (tasksItems.size ());
    Ui_.ActionStart_->setEnabled (tasksItems.size ());
    Ui_.ActionStop_->setEnabled (tasksItems.size ());
    Ui_.ActionGetFileSize_->setEnabled (tasksItems.size ());
    Ui_.ActionSchedule_->setEnabled (tasksItems.size () && CronEnabled_);
    Ui_.ActionJobProperties_->setEnabled (tasksItems.size ());

    Ui_.ActionStartAll_->setEnabled (JobManager::Instance ().rowCount (QModelIndex ()));
    Ui_.ActionStopAll_->setEnabled (JobManager::Instance ().rowCount ());
}

void HttpPlugin::handleCronEnabled ()
{
    CronEnabled_ = true;
    setActionsEnabled ();
}

void HttpPlugin::on_ActionActiveColumns__triggered ()
{
    QList<QPair<QString, bool> > columns;
    for (int i = 0; i < TaskHeaderLabels_.size (); ++i)
        columns.append (qMakePair (TaskHeaderLabels_.at (i), !Ui_.TasksList_->isColumnHidden (i)));
    ColumnSelector selector (this);
    selector.SetColumnsStates (columns);
    if (selector.exec () == QDialog::Rejected)
        return;
    else
    {
        QList<bool> states = selector.GetColumnsStates ();
        for (int i = 0; i < states.size (); ++i)
            Ui_.TasksList_->setColumnHidden (i, !states.at (i));
    }
}

void HttpPlugin::on_ActionFinishedColumns__triggered ()
{
    QList<QPair<QString, bool> > columns;
    for (int i = 0; i < FinishedHeaderLabels_.size (); ++i)
        columns.append (qMakePair (FinishedHeaderLabels_.at (i), !Ui_.FinishedList_->isColumnHidden (i)));
    ColumnSelector selector (this);
    selector.SetColumnsStates (columns);
    if (selector.exec () == QDialog::Rejected)
        return;
    else
    {
        QList<bool> states = selector.GetColumnsStates ();
        for (int i = 0; i < states.size (); ++i)
            Ui_.FinishedList_->setColumnHidden (i, !states.at (i));
    }
}

void HttpPlugin::addToFinishedList (const FinishedJob *fj, int id)
{
    QTreeWidgetItem *item = new QTreeWidgetItem;
    item->setText (FListLocalName, QFileInfo (fj->GetLocal ()).fileName ());
    item->setText (FListURL, fj->GetURL ());
    item->setText (FListSize, Proxy::Instance ()->MakePrettySize (fj->GetSize ()));
    item->setText (FListSpeed, fj->GetSpeed ());
    item->setText (FListTimeToComplete, fj->GetTimeToComplete ());

    Ui_.FinishedList_->addTopLevelItem (item);

    setActionsEnabled ();

    QString string = tr ("Name: %1, size %2").arg (QFileInfo (fj->GetLocal ()).fileName ()).arg (Proxy::Instance ()->MakePrettySize (fj->GetSize ()));
    emit downloadFinished (string);
    emit fileDownloaded (fj->GetLocal ());

    if (!SaveChangesScheduled_)
    {
        SaveChangesScheduled_ = true;
        QTimer::singleShot (100, this, SLOT (writeSettings ()));
    }
}

void HttpPlugin::ReadSettings ()
{
    QSettings settings (Proxy::Instance ()->GetOrganizationName (), Proxy::Instance ()->GetApplicationName () + "_HTTP");
    settings.beginGroup ("geometry");
    resize (settings.value ("size", QSize (640, 480)).toSize ());
    move   (settings.value ("pos",  QPoint (10, 10)).toPoint ());

    QByteArray jobListArr = settings.value ("jobListHeadersState").toByteArray ();
    if (!jobListArr.isEmpty () && !jobListArr.isNull ())
        Ui_.TasksList_->header ()->restoreState (jobListArr);
    else if (JobManager::Instance ().rowCount (QModelIndex ()))
    {
        for (int i = 0; i < JobManager::Instance ().columnCount (QModelIndex ()); ++i)
            Ui_.TasksList_->resizeColumnToContents (i);
    }

    QByteArray finishedListArr = settings.value ("finishedListHeadersState").toByteArray ();
    if (!finishedListArr.isEmpty () && !finishedListArr.isNull ())
        Ui_.FinishedList_->header ()->restoreState (finishedListArr);
    else if (Ui_.FinishedList_->topLevelItemCount ())
    {
        for (int i = 0; i < Ui_.FinishedList_->columnCount (); ++i)
            Ui_.FinishedList_->resizeColumnToContents (i);
    }

    settings.endGroup ();

    int size = settings.beginReadArray ("finished");
    for (int i = 0; i < size; ++i)
    {
        settings.setArrayIndex (i);
        QTreeWidgetItem item;
        QDataStream str (settings.value ("representation").toByteArray ());
        item.read (str);
        Ui_.FinishedList_->addTopLevelItem (new QTreeWidgetItem (item));
    }
    setActionsEnabled ();
    settings.endArray ();
}

void HttpPlugin::HandleSelected (JobAction ja)
{
    if (!JobManager::Instance ().rowCount (QModelIndex ()))
        return;
    QModelIndexList items = Ui_.TasksList_->selectionModel ()->selectedRows ();

    if (ja == JADelete && QMessageBox::question (this, tr ("Question"), tr ("Do you really want to delete selected jobs?"), QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
        return;

    for (int i = 0; i < items.size (); ++i)
    {
        switch (ja)
        {
            case JAStart:
                JobManager::Instance ().Start (items.at (i).row ());
                break;
            case JAStop:
                JobManager::Instance ().Stop (items.at (i).row ());
                break;
            case JADelete:
                JobManager::Instance ().Delete (items.at (i).row ());
                break;
            case JAGFS:
                JobManager::Instance ().GetFileSize (items.at (i).row ());
                break;
            case JASchedule:
                JobManager::Instance ().Schedule (items.at (i).row ());
                break;
        }
    }
}

void HttpPlugin::closeEvent (QCloseEvent*)
{
    IsShown_ = false;
}

Q_EXPORT_PLUGIN2 (leechcraft_http, HttpPlugin);


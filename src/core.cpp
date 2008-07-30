#include <QMainWindow>
#include <QTreeWidgetItem>
#include <QMessageBox>
#include <QtDebug>
#include <QTimer>
#include <limits>
#include <QApplication>
#include <QClipboard>
#include <QDomDocument>
#include <QDomElement>
#include "mainwindow.h"
#include "pluginmanager.h"
#include "plugininfo.h"
#include "core.h"
#include "exceptions/logic.h"
#include "exceptions/outofbounds.h"
#include "xmlsettingsmanager.h"

Main::Core::Core (QObject *parent)
: QObject (parent)
{
    PreparePools ();
    PluginManager_ = new Main::PluginManager (this);
    connect (this, SIGNAL (error (QString)), parent, SLOT (catchError (QString)));
    connect (PluginManager_, SIGNAL (gotPlugin (const PluginInfo*)), this, SIGNAL (gotPlugin (const PluginInfo*)));

    ClipboardWatchdog_ = new QTimer (this);
    connect (ClipboardWatchdog_, SIGNAL (timeout ()), this, SLOT (handleClipboardTimer ()));
    ClipboardWatchdog_->start (2000);
}

Main::Core::~Core ()
{
}

void Main::Core::Release ()
{
    PluginManager_->Release ();
    delete PluginManager_;
    ClipboardWatchdog_->stop ();
    delete ClipboardWatchdog_;
}

void Main::Core::SetReallyMainWindow (Main::MainWindow *win)
{
    ReallyMainWindow_ = win;
}

Main::MainWindow* Main::Core::GetReallyMainWindow ()
{
    return ReallyMainWindow_;
}

void Main::Core::DelayedInit ()
{
    PluginManager_->InitializePlugins (ReallyMainWindow_);
    PluginManager_->CalculateDependencies ();
    PluginManager_->ThrowPlugins ();
    QObjectList plugins = PluginManager_->GetAllPlugins ();
    foreach (QObject *plugin, plugins)
    {
        connect (this, SIGNAL (hidePlugins ()), plugin, SLOT (handleHidePlugins ()));
        IDownload *download = dynamic_cast<IDownload*> (plugin);

		const QMetaObject *qmo = plugin->metaObject ();

        if (download)
        {
            connect (plugin, SIGNAL (fileDownloaded (const QString&)), this, SLOT (handleFileDownload (const QString&)));
        }
		if (qmo->indexOfSignal (QMetaObject::
					normalizedSignature ("downloadFinished (const "
						"QString&)").constData ()) != -1)
			connect (plugin,
					SIGNAL (downloadFinished (const QString&)),
					this,
					SIGNAL (downloadFinished (const QString&)));
    }
}

bool Main::Core::ShowPlugin (IInfo::ID_t id)
{
    QObject *plugin = PluginManager_->FindByID (id);
    IWindow *w = qobject_cast<IWindow*> (plugin);
    if (w)
    {
        w->ShowWindow ();
        return true;
    }
    else
        return false;
}

void Main::Core::HideAll ()
{
    emit hidePlugins ();
}

void Main::Core::TryToAddJob (const QString& name)
{
    QObjectList plugins = PluginManager_->GetAllPlugins ();
    qDebug () << plugins;
    foreach (QObject *plugin, plugins)
    {
        IDownload *di = dynamic_cast<IDownload*> (plugin);
        IInfo *ii = qobject_cast<IInfo*> (plugin);
        if (di && di->CouldDownload (name, false))
        {
            di->AddJob (name);
            return;
        }
    }
    emit error (tr ("No plugins are able to download \"%1\"").arg (name));
}

QPair<qint64, qint64> Main::Core::GetSpeeds () const
{
    qint64 download = 0;
    qint64 upload = 0;
    QObjectList plugins = PluginManager_->GetAllPlugins ();
    foreach (QObject *plugin, plugins)
    {
        IDownload *di = dynamic_cast<IDownload*> (plugin);
        if (di)
        {
            download += di->GetDownloadSpeed ();
            upload += di->GetUploadSpeed ();
        }
    }

    return QPair<qint64, qint64> (download, upload);
}

QList<JobHolder> Main::Core::GetJobHolders () const
{
    QList<JobHolder> result;
    QObjectList plugins = PluginManager_->GetAllCastableTo<IJobHolder*> ();
    for (int i = 0; i < plugins.size (); ++i)
    {
        IJobHolder *ijh = qobject_cast<IJobHolder*> (plugins.at (i));
        JobHolder jh = { qobject_cast<IInfo*> (plugins.at (i)),
            ijh->GetRepresentation (),
            ijh->GetDelegate () };
        result << jh;
    }
    return result;
}

void Main::Core::handleFileDownload (const QString& file, bool fromBuffer)
{
    if (!fromBuffer && !XmlSettingsManager::Instance ()->property ("QueryPluginsToHandleFinished").toBool ())
        return;

    QObjectList plugins = PluginManager_->GetAllCastableTo<IDownload*> ();
    for (int i = 0; i < plugins.size (); ++i)
    {
        IDownload *id = dynamic_cast<IDownload*> (plugins.at (i));
        IInfo *ii = dynamic_cast<IInfo*> (plugins.at (i));
        if (id->CouldDownload (file, fromBuffer))
        {
            if (QMessageBox::question (qobject_cast<QWidget*> (qobject_cast<QObject*> (this)->parent ()),
                tr ("Question"),
                tr ("File %1 could be handled by plugin %2, would you like to?").arg (file).arg (ii->GetName ()),
                QMessageBox::Yes | QMessageBox::No) == QMessageBox::No)
                continue;

            id->AddJob (file);
        }
    }
}

void Main::Core::handleClipboardTimer ()
{
    QString text = QApplication::clipboard ()->text ();
    if (text.isEmpty () || text == PreviousClipboardContents_)
        return;

    PreviousClipboardContents_ = text;

    if (XmlSettingsManager::Instance ()->property ("WatchClipboard").toBool ())
        handleFileDownload (text, true);
}

void Main::Core::PreparePools ()
{
    for (int i = 0; i < 255; ++i)
        TasksIDPool_.push_back (i);
}

QVariant Main::Core::GetTaskData (int row, int column) const
{
    if (row == -1)
    {
        switch (column)
        {
            case 0:
                return tr ("Name");
            case 1:
                return tr ("Name");
        }
    }
    switch (column)
    {
        case 0:
            return PluginManager_->Name (row);
        case 1:
            return PluginManager_->Info (row);
        default:
            return QVariant ();
    }
}

